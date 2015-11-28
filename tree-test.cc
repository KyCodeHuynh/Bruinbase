// Uncomment this to disable all asserts; 
// main() will then display all failed unit-tests
// #define NDEBUG 
#include <cassert> 
#include <cstdio>
#include <cstring>
#include <string>

#include "Bruinbase.h"
#include "RecordFile.h"
#include "PageFile.h"
#include "BTreeIndex.h"

// Encapsulate everything into test functions
// for ease of abstracting and checking I/O 

// Check initial tree state (open() and close())
int treeSetupTest(const std::string& filename); 

// Check insert()
int insertTest(const std::string& filename);

// Check locate()
int locateTest(const std::string& filename);

// Check insert() and locate() in succession
int insertAndLocateTest(const std::string& filename);

// Check readForward()
int readForwardTest(const std::string& filename);

int main()
{
    const std::string filename = "tree-test.txt";

    int rc1 = treeSetupTest(filename);
    if (rc1 < 0) {
        printf("treeSetupTest FAILED with error: %d\n", rc1);
    }

    // Not else-if or return 1, as we want to see all failures
    int rc2 = insertTest(filename);
    if (rc2 < 0) {
        printf("insertTest FAILED with error: %d\n", rc2);
    }

    // Now that nodes are present, search among them
    int rc3 = locateTest(filename);
    if (rc3 < 0) {
        printf("locateTest FAILED with error: %d\n", rc3);
    }

    // Next, try insert() and locate() in succession
    int rc4 = insertAndLocateTest(filename);
    if (rc4 < 0) {
        printf("insertAndLocateTest FAILED with error: %d\n", rc4);
    }

    // Now that locate() is verified as working, 
    // we can use it to find contents. We then test
    // reading them out with readForward().
    int rc5 = readForwardTest(filename);
    if (rc5 < 0) {
        printf("readForwardTest FAILED with error: %d\n", rc5);
    }

    // Write this only once and break only once: after all tests have run
    if (rc1 < 0 || rc2 < 0 || rc3 < 0 || rc4 < 0 || rc5 < 0) {
        // See: https://stackoverflow.com/questions/18840422/do-negative-numbers-return-false-in-c-c
        // "A zero value, null pointer value, or null member pointer value is
        // converted to false; any other value is converted to true."
        assert(0);
        return -1;
    }

    // Silence is golden
    // printf("All BTreeIndex tests passed!\n");
    
    return 0;
}


int treeSetupTest(const std::string& filename)
{
    BTreeIndex indexTree; 

    // Try opening the passed-in filename
    // NOTE: Only insert() modifies a tree's nodes,
    // but this is our first test, so we need to 
    // create that index file if it doesn't already exist
    int rc = indexTree.open(filename, 'w');
    if (rc < 0) {
        // When NDEBUG is defined, 
        // all assert()'s will be disabled, 
        // but the error codes will be detected by main()
        assert(0);
        return rc;
    }

    rc = indexTree.close();
    if (rc < 0) {
        assert(0);
        return rc;
    }

    return 0;
}


int insertTest(const std::string& filename)
{
    BTreeIndex indexTree;
    int rc = indexTree.open(filename, 'w');
    if (rc < 0) {
        return rc;
    }

    RecordId manyRID; 
    manyRID.pid = 6; 
    manyRID.sid = 7;

    // Case 0: Root node does not yet exist
    rc = indexTree.insert(4, manyRID);
    if (rc < 0) {
        assert(0);
        return rc;
    }

    // DEBUG: try locate() right after insert() in same function
    // IndexCursor cursor;
    // rc = indexTree.locate(4, cursor);
    // if (rc < 0) {
    //     // DEBUG
    //     printf("ERROR on locate(4) inside of insertTest(): %d\n", rc);
    //     assert(0);
    //     return rc;
    // }

    // Avoid duplicate entries! 
    manyRID.pid = 7; 
    manyRID.sid = 8;

    // Case 1: Only root node exists
    // rc = indexTree.insert(10, manyRID);
    // if (rc < 0) {
    //     assert(0);
    //     return rc;
    // }

    // TODO: Re-enable this
    // Case 2: Cause root node to overflow
    // for (int i = 15; i < 250; i++) {
    //     // Lets us check locate() easily later
    //     // For actual data, we'll just map 
    //     // them to different keys
    //     manyRID.pid = i;
    //     manyRID.sid = i + 1;
    //     rc = indexTree.insert(i, manyRID);
    //     if (rc < 0) {
    //         return rc;
    //     }
    // }

    rc = indexTree.close();
    if (rc < 0) {
        assert(0);
        return rc;
    }

    return 0;
}


int locateTest(const std::string& filename)
{
    BTreeIndex indexTree;
    int rc = indexTree.open(filename, 'r');
    if (rc < 0) {
        return rc;
    }

    // TODO: Try locating previous insert()
    // entries, which are i and i + 1

    IndexCursor cursor;
    cursor.pid = -1; 
    cursor.eid = -1;

    rc = indexTree.locate(4, cursor);
    if (rc < 0) {
        // DEBUG
        printf("ERROR on locate(4): %d\n", rc);
        assert(0);
        return rc;
    }

    // rc = indexTree.locate(10, cursor);
    // if (rc < 0) {
    //     assert(0);
    //     return rc;
    // }
    
    rc = indexTree.close();
    if (rc < 0) {
        assert(0);
        return rc;
    }

    return 0;
}

int insertAndLocateTest(const std::string& filename)
{
    BTreeIndex indexTree;
    int rc = indexTree.open(filename, 'w');
    if (rc < 0) {
        assert(0);
        return rc;
    }

    RecordId manyRID; 
    manyRID.pid = 8; 
    manyRID.sid = 9;

    rc = indexTree.insert(0, manyRID);
    if (rc < 0) {
        assert(0);
        return rc;
    }

    manyRID.pid = 9; 
    manyRID.sid = 10;

    rc = indexTree.insert(1, manyRID);
    if (rc < 0) {
        assert(0);
        return rc;
    }

    IndexCursor cursor; 
    cursor.pid = -1; 
    cursor.eid = -1;

    // DEBUG
    // printf("READY TO START LOCATING!\n");

    // Try locate()'ing the previously inserted entries
    rc = indexTree.locate(0, cursor);
    if (rc < 0) {
        assert(0);
        return rc;
    }

    // DEBUG
    // printf("1st Pid (should be 1): %d\n", cursor.pid);
    // printf("1st Eid (should be 0): %d\n", cursor.eid);

    // TODO: The current problem case: output 
    // internal state during insert() to see what's wrong

    rc = indexTree.locate(1, cursor);
    if (rc < 0) {
        assert(0);
        return rc;
    }    

    // DEBUG
    // printf("2nd Pid (should be 1): %d\n", cursor.pid);
    // printf("2nd Eid (should be 1): %d\n", cursor.eid);

    rc = indexTree.close();
    if (rc < 0) {
        assert(0);
        return rc;
    }

    return 0;
}

int readForwardTest(const std::string& filename)
{
    BTreeIndex indexTree;
    // readForward() is read-only, 
    // and "tree-test.txt" should have been 
    // created by our previous tests
    int rc = indexTree.open(filename, 'r');
    if (rc < 0) {
        assert(0);
        return rc;
    }

    // TODO: Need to print out index contents
    // to figure out which entry ID is correct
    
    // int key = 4;
    // IndexCursor cursor; 
    // cursor.pid = 1; 
    // cursor.eid = 2;
    // RecordId result;
    // indexTree.readForward(cursor, key, result);
    // if (cursor.eid != 3) {
    //     assert(0);
    //     return -1;
    // }

    rc = indexTree.close();
    if (rc < 0) {
        assert(0);
        return rc;
    }

    return 0;
}