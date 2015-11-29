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
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
// CRYSTAL - I feel like we need to include this header file
// KY-CUONG - I'm fairly sure we need to do so
#include "BTreeIndex.h"

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

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
    RecordFile rf;   // RecordFile containing the table
    RecordId   rid;  // record cursor for table scanning

    RC     rc;
    int    key;     
    string value;
    int    count;
    int    diff;

    // open the table file
    if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
        fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
        return rc;
    }

    // scan the table file from the beginning
    rid.pid = rid.sid = 0;
    count = 0;
    while (rid < rf.endRid()) {
        // read the tuple
        if ((rc = rf.read(rid, key, value)) < 0) {
            fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
            goto exit_select;
        }

        // check the conditions on the tuple
        for (unsigned i = 0; i < cond.size(); i++) {
            // compute the difference between the tuple value and the condition value
            switch (cond[i].attr) {
                case 1:
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
    fprintf(stderr, "DEBUG: We're past opening the loadfile\n");

    // CRYSTAL
    // If index is true, make a BTreeIndex in addition to the RecordFile
    // I was going to try and integrate both methods, but that would add extra logic in the while() statement
    if (index == true) {
        // DEBUG
        fprintf(stderr, "DEBUG: Desire to make index is TRUE!\n");
        if ((retIndexCode = indexFile.open(table + ".idx", 'w')) < 0) {    
            fprintf(stderr, "Could not open/create file %s.idx for writing\n", table.c_str());
            return retIndexCode;        
        }

        if (load.is_open()) {
            fprintf(stderr, "DEBUG: Loadfile stream is open!\n");
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
                fprintf(stderr, "DEBUG: \n rid.PID: %d\n rid.SID: %d\n", rid.pid, rid.sid);
                fprintf(stderr, "DEBUG: Tree Height: %d\n", indexFile.getTreeHeight());
                fprintf(stderr, "DEBUG: Root Pid: %d\n", indexFile.getRootPid());
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
        fprintf(stderr, "DEBUG: Desire to make index is FALSE!\n");
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
                fprintf(stderr, "DEBUG: rid.PID: %d\n rid.SID: %d\n", rid.pid, rid.sid);
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
