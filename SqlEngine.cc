/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <iostream>
#include <fstream>

#include "Bruinbase.h"
#include "SqlEngine.h"
// This needs to be included: https://piazza.com/class/ieyj7ojonx58s?cid=338
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

// external functions and variables for load file and sql command parsing
extern FILE* sqlin;
int sqlparse(void);
BTreeIndex indexTree;


RC SqlEngine::run(FILE* commandline)
{
    fprintf(stdout, "Bruinbase> ");

    // set the command line input and start parsing user input
    sqlin = commandline;
    sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
    // SqlParser.y by bison (bison is GNU equivalent of yacc)

    return 0;
}

// attr is a number that notes what 
RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
    // Error: attr is outside of its allowable range
    if (attr < 1 || attr > 4) {
        fprintf(stderr, "Error: SqlEngine::load() received an invalid 'attr' argument");
    }

    RecordFile rf;   // RecordFile containing the table
    RecordId   rid;  // record cursor for table scanning
    BTreeIndex indexTree; // Index file data for B+ tree
    int countResult = 0; // Number of matching result tuples

    // Open the table file
    RC rc;
    if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
        fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
        return rc;
    }

    // Open the index file
    // fprintf(stderr, "DEBUG: Table name passed-in: %s\n", table.c_str());
    if ((rc = indexTree.open(table + ".idx", 'r')) < 0) {
        fprintf(stderr, "Error: IndexFile %s.idx does not exist\n", table.c_str());
        return rc;
    }

    // Use the index only if at least one selector 
    // other than '<>' exists, and it compares key values.
    // This avoids excessive page reads.
    // See: https://piazza.com/class/ieyj7ojonx58s?cid=362
    // See: https://piazza.com/class/ieyj7ojonx58s?cid=341
    bool useIndex = false; 
    std::vector<SelCond>::const_iterator it;
    for (it = cond.begin(); it != cond.end(); it++) {
        if (it->attr == 1 && it->comp != SelCond::NE) {
            useIndex = true;
        }
    }

    // To use the index, we find the range of keys over which to check tuples. 
    // For example: key >= 5 AND key < 11 implies: 5 <= key < 11
    // This lets us handle arbitrarily many conditions on keys, 
    // as they always end up defining some range of allowable keys. 
    // See: https://piazza.com/class/ieyj7ojonx58s?cid=335
    //
    // If there are no WHERE conditions, we can just iterate
    // through the table and get projected attributes
    if (useIndex && cond.size() != 0) {
        // Keys are of at most long int size, i.e., +/- 2 billion or so
        // See: https://piazza.com/class/ieyj7ojonx58s?cid=328
        // See: http://www.cplusplus.com/reference/climits/
        int smallest_key = indexTree.getSmallestKey();
        int largest_key = indexTree.getLargestKey();
        int rangeBottom = smallest_key;
        int rangeTop = largest_key;
        
        fprintf(stderr, "RANGE BOTTOM START: %d\n", rangeBottom);
        fprintf(stderr, "RANGE TOP START: %d\n", rangeTop);

        // We want the smallest <, <= (lower bound)
        // and the largest >, >= (upper bound)
        for (it = cond.begin(); it != cond.end(); it++) {
            // We're figuring out a range for keys, not values, 
            // which are not searchable via the B+ tree index
            int compKey = atoi(it->value);
            if (it->attr == 1) {
                // Update rangeBottom if we find a 'key < A' or 'key <= A', 
                // where A < rangeBottom. 
                if (it->comp == SelCond::LE || it->comp == SelCond::LT) {
                    if (compKey < rangeTop) {
                        rangeTop = compKey;
                    }
                }
                else if (it->comp == SelCond::GE || it->comp == SelCond::GT) {
                    if (compKey > rangeBottom) {
                        rangeBottom = compKey;
                    }
                }
            }
        }

        // Make sure that rangeBottom and rangeTop don't have an impossible range
        if (rangeTop < rangeBottom) {
            fprintf(stderr, "This is an impossible range.\n");
            fprintf(stderr, "rangeTop and rangeBottom: %d %d\n", rangeTop, rangeBottom);    
            return RC_INVALID_ATTRIBUTE;
        }

        // DEBUG - checking what our range is 
        fprintf(stderr, "rangeTop and rangeBottom: %d %d\n", rangeTop, rangeBottom);

        // readForward() from rangeBottom until up to and including rangeTop
        // find the rangeBottom value
        int start_key = rangeBottom;

        // IndexCursor pointing to the first key
        IndexCursor cursor;
        indexTree.locate(start_key, cursor);

        // Holders for each key, rid, value
        int key = -1;
        RecordId rid;
        rid.pid = -1; 
        rid.sid = -1;
        string value = "";
        int diff = 0;

        // Major for-loop to print out all the keys
        while (1) {
            // Reads the current cursor into key and rid, goes to the next one
            indexTree.readForward(cursor, key, rid);

            if ((rc = rf.read(rid, key, value)) < 0) {
                fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
                goto exit_index_select;
            }

            // Check the conditions on the tuple
            // Run through the list of conditions for each tuple
            for (unsigned i = 0; i < cond.size(); i++) {
                // compute the difference between the tuple value and the condition value
                switch (cond[i].attr) {
                    case 1:
                        // 1 indicates we're selecting on a key
                        diff = key - atoi(cond[i].value);
                        break;
                    case 2:
                        diff = strcmp(value.c_str(), cond[i].value);
                        break;
                }

                // skip the tuple if any condition is not met
                switch (cond[i].comp) {
                    case SelCond::EQ:
                        if (diff != 0) goto read_forward;
                        break;
                    case SelCond::NE:
                        if (diff == 0) goto read_forward;
                        break;
                    case SelCond::GT:
                        if (diff <= 0) goto read_forward;
                        break;
                    case SelCond::LT:
                        if (diff >= 0) goto read_forward;
                        break;
                    case SelCond::GE:
                        if (diff < 0) goto read_forward;
                        break;
                    case SelCond::LE:
                        if (diff > 0) goto read_forward;
                        break;
                }
            }

            // the condition is met for the tuple.
            // increase matching tuple counter
            countResult++;

            // print the tuple
            switch (attr) {
                case 1:  // SELECT key
                    fprintf(stdout, "%d\n", key);
                    break;
                case 2:  // SELECT value
                    fprintf(stdout, "%s\n", value.c_str());
                    break;
                case 3:  // SELECT *
                    fprintf(stdout, "%d '%s'\n", key, value.c_str());
                    break;
            }

            // move to the next tuple
            read_forward:
                // At the end of the for loop, if this was the last key, then break out of loop
                if (key == rangeTop || key == largest_key) {
                    break;
                }
        }

        // SELECT COUNT(*) ?
        if (attr == 4) {
            fprintf(stdout, "%d\n", countResult);
        }
        rc = 0;

        // close the table file and return
        exit_index_select:
            rf.close();
            return rc;
    }

    // Scan through the table if we should do that instead
    else {
        int    key;
        string value;
        int    count;
        int    diff;

        // scan the table file from the beginning
        rid.pid = rid.sid = 0;
        count = 0;
        while (rid < rf.endRid()) {
            // read the tuple
            if ((rc = rf.read(rid, key, value)) < 0) {
                fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
                goto exit_select;
            }

            // Check the conditions on the tuple
            // Run through the list of conditions for each tuple
            for (unsigned i = 0; i < cond.size(); i++) {
                // compute the difference between the tuple value and the condition value
                switch (cond[i].attr) {
                    case 1:
                        // 1 indicates we're selecting on a key
                        diff = key - atoi(cond[i].value);
                        break;
                    case 2:
                        diff = strcmp(value.c_str(), cond[i].value);
                        break;
                }

                // skip the tuple if any condition is not met
                switch (cond[i].comp) {
                    case SelCond::EQ:
                        if (diff != 0) goto next_tuple;
                        break;
                    case SelCond::NE:
                        if (diff == 0) goto next_tuple;
                        break;
                    case SelCond::GT:
                        if (diff <= 0) goto next_tuple;
                        break;
                    case SelCond::LT:
                        if (diff >= 0) goto next_tuple;
                        break;
                    case SelCond::GE:
                        if (diff < 0) goto next_tuple;
                        break;
                    case SelCond::LE:
                        if (diff > 0) goto next_tuple;
                        break;
                }
            }

            // the condition is met for the tuple.
            // increase matching tuple counter
            count++;

            // print the tuple
            switch (attr) {
                case 1:  // SELECT key
                    fprintf(stdout, "%d\n", key);
                    break;
                case 2:  // SELECT value
                    fprintf(stdout, "%s\n", value.c_str());
                    break;
                case 3:  // SELECT *
                    fprintf(stdout, "%d '%s'\n", key, value.c_str());
                    break;
            }

            // move to the next tuple
            next_tuple:
                    ++rid;
        }

        // print matching tuple count if "select count(*)"
        if (attr == 4) {
            fprintf(stdout, "%d\n", count);
        }
        rc = 0;

        // close the table file and return
        exit_select:
            rf.close();
            return rc;
    }
    
    // That's a wrap!
    int retCode = 0;
    if ((retCode = rf.close()) < 0) {
        return retCode;
    }

    if ((retCode = indexTree.close()) < 0) {
        return retCode;
    }

    return 0;
}



RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
    // Use I/O libraries to open() loadfile and get a resource handle for it.
    // Open RecordFile "table".tbl if it already exists, or create it if not.
    // Get a line/tuple from loadfile using I/O libraries
    // Parse that line/tuple using SqlEngine::parseLoadLine()
    // Insert the parsed tuple into the RecordFile

    // CRYSTAL
    RecordFile recFile;
    BTreeIndex indexFile;

    RC retRecCode = 0;
    RC retIndexCode = 0;

    // Make sure to create recordFile
    if ((retRecCode = recFile.open(table + ".tbl", 'w')) < 0) {
        fprintf(stderr, "Could not open/create file %s.tbl for writing\n", table.c_str());
        return retRecCode;
    }

    // Open loadfile for reading only; ofstream does output; fstream does both
    // See documentation at: http://www.cplusplus.com/doc/tutorial/files/
    string line;
    ifstream load;
    load.open(loadfile.c_str( ));

    // DEBUG
    // fprintf(stderr, "DEBUG: We're past opening the loadfile\n");

    // CRYSTAL
    // If index is true, make a BTreeIndex in addition to the RecordFile
    // I was going to try and integrate both methods, but that would add extra logic in the while() statement
    if (index == true) {
        // DEBUG
        // fprintf(stderr, "DEBUG: Desire to make index is TRUE!\n");
        // fprintf(stderr, "DEBUG: able name in load(): %s\n", table.c_str());
        if ((retIndexCode = indexFile.open(table + ".idx", 'w')) < 0) {
            fprintf(stderr, "Could not open/create file %s.idx for writing\n", table.c_str());
            return retIndexCode;
        }

        if (load.is_open()) {
            // fprintf(stderr, "DEBUG: Loadfile stream is open!\n");
            int key = -1;
            string value = "";
            RecordId rid;
            rid.pid = -1; // PageID
            rid.sid = -1; // SlotID

            // Load a line into 'line'
            while(getline(load, line)) {
                // Parse 'line', load it into the RecordFile, add to BTreeIndex
                parseLoadLine(line, key, value);
                // fprintf(stderr, "DEBUG: Key: %d \n Value: %s\n\n", key, value.c_str());
                recFile.append(key, value, rid);
                // fprintf(stderr, "DEBUG: rid.PID: %d\n rid.SID: %d\n", rid.pid, rid.sid);
                indexFile.insert(key, rid);
            }
        }
        else {
            fprintf(stderr, "Unable to open file %s for reading\n", loadfile.c_str());
            return RC_FILE_OPEN_FAILED;
        }

        indexFile.close();
    }
    // TODO: We actually always need to load into a RecordFile
    // It's only optional on whether or not we create a BTreeIndex
    else {
        // DEBUG:
        // fprintf(stderr, "DEBUG: Desire to make index is FALSE!\n");
        if (load.is_open()) {
            int key = -1;
            string value = "";
            RecordId rid;
            rid.pid = -1; // PageID
            rid.sid = -1; // SlotID

            // Load a line into 'line'
            while(getline(load, line)) {
                // Parse 'line' and load it into the RecordFile
                parseLoadLine(line, key, value);
                // fprintf(stderr, "DEBUG: Key: %d \n Value: %s\n\n", key, value.c_str());
                recFile.append(key, value, rid);
                // fprintf(stderr, "DEBUG: rid.PID: %d\n rid.SID: %d\n", rid.pid, rid.sid);
            }
        }
        else {
            fprintf(stderr, "Unable to open file %s for reading\n", loadfile.c_str());
            return RC_FILE_OPEN_FAILED;
        }
    }

    load.close();
    recFile.close();
    return 0;
}

// Takes a raw input line from the loadfile,
// and populates its outputs with the key/value pair.
RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;

    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');

    // if there is nothing left, set the value to empty string
    if (c == 0) {
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
