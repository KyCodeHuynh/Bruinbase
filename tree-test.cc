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

    int rc = treeSetupTest(filename);
    if (rc < 0) {
        printf("treeSetupTest FAILED with error: %d\n", rc);
    }

    // Not else-if or return 1, as we want to see all failures
    rc = insertTest(filename);
    if (rc < 0) {
        printf("insertTest FAILED with error: %d\n", rc);
    }

    // Now that nodes are present, search among them
    rc = locateTest(filename);
    if (rc < 0) {
        printf("locateTest FAILED with error: %d\n", rc);
    }

    // Now that locate() is verified as working, 
    // we can use it to find contents. We then test
    // reading them out with readForward().
    rc = readForwardTest(filename);
    if (rc < 0) {
        printf("readForwardTest FAILED with error: %d\n", rc);
    }

    // Write this only once and break only once: after all tests have run
    if (rc < 0) {
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

    // TODO: tests of treeHeight and rootPid helpers

    // NOTE: treeHeight and rootPid are not available
    // until an index file is loaded, so treeHeight below
    // should be an error code, i.e., negative
    // This is due to them being stored in page 0, but can be interpreted
    // as BTreeIndex being an API atop raw PageFile data. 
    // Without both, it's useless. 
    if (indexTree.getTreeHeight() >= 0) {
        return RC_INVALID_ATTRIBUTE;    
    }

    // Try opening the passed-in filename
    // NOTE: Only insert() modifies a tree's nodes,
    // but this is our first test, so we need to 
    // create that index file if it doesn't already exist
    int rc = indexTree.open(filename, 'w');
    if (rc < 0) {
        return rc;
    }

    // getTreeHeight() and getRootPid() are meant
    // to only be called after insert(), but 
    // we can check consistency between getters/setters first
    int result = indexTree.getTreeHeight();
    if (indexTree.getTreeHeight() != -1) {
        return -1;
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
    RecordId manyRID; 
    manyRID.pid = 6; 
    manyRID.sid = 7;
    indexTree.insert(4, manyRID);
    
    return 0;
}


int locateTest(const std::string& filename)
{
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