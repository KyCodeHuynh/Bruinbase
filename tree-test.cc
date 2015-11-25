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
    // int rc2 = insertTest(filename);
    int rc2 = 0;
    if (rc2 < 0) {
        printf("insertTest FAILED with error: %d\n", rc2);
    }

    // Now that nodes are present, search among them
    int rc3 = locateTest(filename);
    if (rc3 < 0) {
        printf("locateTest FAILED with error: %d\n", rc3);
    }

    // Now that locate() is verified as working, 
    // we can use it to find contents. We then test
    // reading them out with readForward().
    int rc4 = readForwardTest(filename);
    if (rc4 < 0) {
        printf("readForwardTest FAILED with error: %d\n", rc4);
    }

    // Write this only once and break only once: after all tests have run
    if (rc1 < 0 || rc2 < 0 || rc3 < 0 || rc4 < 0) {
        // See: https://stackoverflow.com/questions/18840422/do-negative-numbers-return-false-in-c-c
        // "A zero value, null pointer value, or null member pointer value is
        // converted to false; any other value is converted to true."
        assert(0);
    }

    printf("All BTreeIndex tests passed!\n");
    
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
        return rc;
    }

    rc = indexTree.close();
    if (rc < 0) {
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
        return rc;
    }

    // Case 1: Only root node exists
    rc = indexTree.insert(10, manyRID);
    if (rc < 0) {
        return rc;
    }

    // Case 2: Cause root node to overflow
    for (int i = 15; i < 250; i++) {
        rc = indexTree.insert(i, manyRID);
        if (rc < 0) {
            return rc;
        }
    }

    return 0;
}


int locateTest(const std::string& filename)
{
    BTreeIndex indexTree;
    // locate() is read-only, 
    // and "tree-test.txt" should have been 
    // created by our previous tests
    int rc = indexTree.open(filename, 'w');
    if (rc < 0) {
        return rc;
    }

    RecordId manyRID; 
    manyRID.pid = 1; 
    manyRID.sid = 7;

    // Case 0: Root node does not yet exist
    rc = indexTree.insert(4, manyRID);
    if (rc < 0) {
        return rc;
    }

    printf("PART ONE IS DONE\n");

    manyRID.pid = 1; 
    manyRID.sid = 8;


    // Case 1: Only root node exists
    rc = indexTree.insert(10, manyRID);
    if (rc < 0) {
        return rc;
    }

    rc = indexTree.close();
    if (rc < 0) {
        return rc;
    }

    printf("PART TWO IS DONE\n");

    rc = indexTree.open(filename, 'r');
    if (rc < 0) {
        return rc;
    }


    IndexCursor cursor; 
    // cursor.pid = 1; 
    // cursor.eid = 2;

    printf("READY TO START LOCATING!\n");

    // Locate root node
    rc = indexTree.locate(4, cursor);

    if (rc < 0) {
        return rc;
    }

    printf("1st Pid: %d\n", cursor.pid);
    printf("1st Eid: %d\n", cursor.eid);

    // Locate deep node
    rc = indexTree.locate(10, cursor);
    if (rc < 0) {
        return rc;
    }    

    printf("2nd Pid: %d\n", cursor.pid);
    printf("2nd Eid: %d\n", cursor.eid);

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
        return rc;
    }

    // TODO: Not testable until insert() is working
    // IndexCursor cursor; 
    // cursor.pid = 1; 
    // cursor.eid = 2;
    // RecordId result;
    // indexTree.readForward(cursor, key, result);
    // assert(cursor.eid == 3);

    return 0;
}