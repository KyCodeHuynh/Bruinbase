#include <cassert> 
#include <cstdio>
#include <cstring>

#include "Bruinbase.h"
#include "RecordFile.h"
#include "PageFile.h"
#include "BTreeNode.h"

// print the current keys in a node - in order
void printNode(BTNonLeafNode* node, PageFile* pagefile) {
    typedef struct nonLeafEntry {
        int key;
        PageId pid;
    } nonLeafEntry;    

    int x = 0;

   char buffer[1024];
    // Need to read in PageFile's contents from our specific page
    // AFTER we've updated it with the latest, or else we won't 
    // be able to see the contents of the updated node
    node->write(0, *pagefile);
    pagefile->read(0, buffer);

    PageId first_pageid;
    memcpy(&first_pageid, &buffer[sizeof(int)], sizeof(PageId));
    printf("first page id: %d\n", first_pageid);

    nonLeafEntry inserted; 
    int offset = sizeof(int) + sizeof(PageId);

    while(x < node->getKeyCount()) {
        memcpy(&inserted, &buffer[offset], sizeof(nonLeafEntry));
        printf("key: %d\n", inserted.key);
        printf("pid: %d\n", inserted.pid);
        offset = offset + sizeof(nonLeafEntry);
        x++;
    }
}

void nonLeafNodeTest() {
    /// TESTING FOR NON-LEAF FILE
    PageFile nf("nonleaf-node-test.txt", 'w');
    // for some reason, these get errors  ---------------------------------------------------------------------------------------------------------------------TODO
    // assert(nf.getPageReadCount() == 0);
    // assert(nf.getPageWriteCount() == 0);

    // The leaf node itself, which has just zeroed-out
    // internal buffer initially. 
    BTNonLeafNode nonleafNode;

    // Key count is initially zero, as is next sibling PageId
    assert(nonleafNode.getKeyCount() == 0); 

    // Debugging only
    printf("End PageId: %d\n", nf.endPid());
    printf("Return code: %d\n", nonleafNode.read(0, nf));

    // Now we load the actual disk page, which requires
    // us to write to the PageFile first, to set-up a page
    char temp[1024];
    memset(temp, 0, 1024);
    nf.write(0, temp);

    // Read should now succeed
    assert(nonleafNode.read(0, nf) == 0);

    // These should not have changed, as we wrote in all zeros
    assert(nonleafNode.getKeyCount() == 0); 

    // But this read with an invalid PageId should fail
    assert(nonleafNode.read(-1, nf) == RC_INVALID_PID);

    /// TESTING: initialize root
    // idk if we are allowed to have other keys in there at that point, so ill check that. if we do-- we may need to shift everything right once

    // Empty node - initialize root
    int key1 = 10; 
    PageId pid1 = 2;
    PageId pid2 = 1;
    assert(nonleafNode.initializeRoot(pid1, key1, pid2) == 0);

    // This is the first inserted key, so it should 
    // have been inserted right after the reserved buffer space
    char buffer2[1024];
    // Need to read in PageFile's contents from our specific page
    // AFTER we've updated it with the latest, or else we won't 
    // be able to see the contents of the updated node
    nonleafNode.write(0, nf);
    nf.read(0, buffer2);

    // LeafEntry is private to BTLeafNode
    typedef struct nonLeafEntry {
        int key;
        PageId pid;
    } nonLeafEntry;

    nonLeafEntry inserted2; 
    int offset2 = sizeof(int) + sizeof(PageId);
    PageId first_pageid;
    memcpy(&first_pageid, &buffer2[sizeof(int)], sizeof(PageId));
    memcpy(&inserted2, &buffer2[offset2], sizeof(nonLeafEntry));
    assert(first_pageid == 2);
    assert(inserted2.key == 10);
    assert(inserted2.pid == 1); 

    /// TESTING: insert

    // This key should be inserted right after the previous one.
    key1 = 15; 
    pid1 = 3; 

    // Compute before insertion, as this key is being added in
    int insertPoint = offset2 + (nonleafNode.getKeyCount() * sizeof(nonLeafEntry));
    assert(nonleafNode.insert(key1, pid1) == 0);
    nonleafNode.write(0, nf);
    nf.read(0, buffer2);
    memcpy(&inserted2, &buffer2[insertPoint], sizeof(nonLeafEntry));
    assert(inserted2.key == 15);
    assert(inserted2.pid == 3);

    // // This key should go between the previous two: 10, 12, 15
    key1 = 12; 
    pid1 = 4; 
    insertPoint = offset2 + (sizeof(nonLeafEntry));
    assert(nonleafNode.insert(key1, pid1) == 0);
    nonleafNode.write(0, nf);
    nf.read(0, buffer2);
    memcpy(&inserted2, &buffer2[insertPoint], sizeof(nonLeafEntry));
    // printf("Key at second entry: %d\n", inserted.key);
    assert(inserted2.key == 12);
    assert(inserted2.pid == 4);

    // sanity check 
    printNode(&nonleafNode, &nf);

    printf("I'm going to re-initialize the node and see what happens\n");
    printf("NEW ANSWER:\n");

    pid1 = 55;
    pid2 = 12;
    key1 = 5;

    assert(nonleafNode.initializeRoot(pid1, key1, pid2) == 0);

    printNode(&nonleafNode, &nf);
    printf("Adding -1 to the combo:\n\n");

    // // This negative key should go first: -1, 10, 12, 15
    key1 = -1; 
    pid1 = 5; 
    insertPoint = offset2;
    assert(nonleafNode.insert(key1, pid1) == 0);
    nonleafNode.write(0, nf);
    nf.read(0, buffer2);
    memcpy(&inserted2, &buffer2[insertPoint], sizeof(nonLeafEntry));
    // printf("Key at second entry: %d\n", inserted.key);
    assert(inserted2.key == -1);
    assert(inserted2.pid == 5);

    printNode(&nonleafNode, &nf);
    printf("key count: %d\n", nonleafNode.getKeyCount());

    // Need to be able to insert at least 70 keys
    // We've inserted 5 so far, so let's insert 66 more for 71 total
    assert(nonleafNode.getKeyCount() == 5);
    int manyKey = 42;
    PageId manyPID; 
    manyPID = 20; 
    int manyInsertPoint = 0;
    nonLeafEntry manyInserted;
    for (int i = 0; i < 66; i++) {
        manyInsertPoint = offset2 + (nonleafNode.getKeyCount() * sizeof(nonLeafEntry));
        assert(nonleafNode.insert(manyKey, manyPID) == 0);
        // Update PageFile with latest node contents
        nonleafNode.write(0, nf);
        nf.read(0, buffer2);

        // Check inserted key
        memcpy(&manyInserted, &buffer2[manyInsertPoint], sizeof(nonLeafEntry));
        assert(manyInserted.key == manyKey);
        manyKey++;
        manyPID++;
    }
    // assert(nonleafNode.getKeyCount() == 71);
    printf("All 71 nodes have been inserted! \n");
    // printNode(&nonleafNode, &nf);
    printf("key count: %d\n", nonleafNode.getKeyCount());



    /// TESTING: locate()

    // We should now have: -1, 10, 12, 15, 42, 43, .., 107
    int pid = -1; 
    int searchKey = -1;
    assert(nonleafNode.locateChildPtr(searchKey, pid) == 0);
    assert(pid == 5);

    pid = -1; 
    searchKey = 5;
    assert(nonleafNode.locateChildPtr(searchKey, pid) == 0);
    assert(pid == 12);

    // Does not exist
    pid = -1; 
    searchKey = 109;
    assert(nonleafNode.locateChildPtr(searchKey, pid) == RC_NO_SUCH_RECORD);

    pid = -1; 
    searchKey = 106;
    int count = nonleafNode.getKeyCount();

    assert(nonleafNode.locateChildPtr(107, pid) == 0);
    assert(pid == 85);

    printf("the pid is: %d\n", pid);
    printf("the key count is: %d\n", count);

    // need to test insertAndSplit !! 

}

int main() 
{
    /// TESTING: Initial BTLeafNode state and getter/setter functions
    // Create PageFile that will store a leaf node
    PageFile pf("node-test.txt", 'w');
    assert(pf.getPageReadCount() == 0);
    assert(pf.getPageWriteCount() == 0);
    // assert(pf.endPid() == 0); 

    // The leaf node itself, which has just zeroed-out
    // internal buffer initially. 
    BTLeafNode leafNode;

    // Key count is initially zero, as is next sibling PageId
    assert(leafNode.getKeyCount() == 0); 
    assert((int)leafNode.getNextNodePtr() == 0); 

    // Debugging only
    // printf("End PageId: %d\n", pf.endPid());
    // printf("Return code: %d\n", leafNode.read(0, pf));

    // Now we load the actual disk page, which requires
    // us to write to the PageFile first, to set-up a page
    char tempBuffer[1024];
    memset(tempBuffer, 0, 1024);
    pf.write(0, tempBuffer);

    // Read should now succeed
    assert(leafNode.read(0, pf) == 0);

    // These should not have changed, as we wrote in all zeros
    assert(leafNode.getKeyCount() == 0); 
    assert((int)leafNode.getNextNodePtr() == 0); 

    // But this read with an invalid PageId should fail
    assert(leafNode.read(-1, pf) == RC_INVALID_PID);

    // Now try setting the sibling PageId/pointer 
    // to an arbitrary PageId and getting it back
    assert(leafNode.setNextNodePtr(10) == 0);
    assert(leafNode.getNextNodePtr() == 10);

    // An invalid PageId should fail
    assert(leafNode.setNextNodePtr(-1) == RC_INVALID_PID);


    /// TESTING: insert()

    // Empty node initially, so insertion should succeed
    int key = 10; 
    // PageId 1, SlotId 0
    RecordId rid;
    rid.pid = 1; 
    rid.sid = 0;
    assert(leafNode.insert(key, rid) == 0);

    // This is the first inserted key, so it should 
    // have been inserted right after the reserved buffer space
    char buffer[1024];
    // Need to read in PageFile's contents from our specific page
    // AFTER we've updated it with the latest, or else we won't 
    // be able to see the contents of the updated node
    leafNode.write(0, pf);
    pf.read(0, buffer);

    // LeafEntry is private to BTLeafNode
    typedef struct LeafEntry {
        int key;
        RecordId rid;
    } LeafEntry;

    LeafEntry inserted; 
    int offset = sizeof(int) + sizeof(PageId);
    memcpy(&inserted, &buffer[offset], sizeof(LeafEntry));
    assert(inserted.key == 10);
    assert(inserted.rid.pid == 1); 
    assert(inserted.rid.sid == 0);

    // This key should be inserted right after the previous one.
    key = 15; 
    rid.pid = 2; 
    rid.sid = 3;
    // Compute before insertion, as this key is being added in
    int insertPoint = offset + (leafNode.getKeyCount() * sizeof(LeafEntry));
    assert(leafNode.insert(key, rid) == 0);
    leafNode.write(0, pf);
    pf.read(0, buffer);
    memcpy(&inserted, &buffer[insertPoint], sizeof(LeafEntry));
    assert(inserted.key == 15);
    assert(inserted.rid.pid == 2);
    assert(inserted.rid.sid == 3);

    // This key should go between the previous two: 10, 12, 15
    key = 12; 
    rid.pid = 4; 
    rid.sid = 5;
    insertPoint = offset + (1 * sizeof(LeafEntry));
    assert(leafNode.insert(key, rid) == 0);
    leafNode.write(0, pf);
    pf.read(0, buffer);
    memcpy(&inserted, &buffer[insertPoint], sizeof(LeafEntry));
    // printf("Key at second entry: %d\n", inserted.key);
    assert(inserted.key == 12);
    assert(inserted.rid.pid == 4);
    assert(inserted.rid.sid == 5);

    // This negative key should go first: -1, 10, 12, 15
    key = -1; 
    rid.pid = 5; 
    rid.sid = 6;
    insertPoint = offset;
    assert(leafNode.insert(key, rid) == 0);
    leafNode.write(0, pf);
    pf.read(0, buffer);
    memcpy(&inserted, &buffer[insertPoint], sizeof(LeafEntry));
    // printf("Key at second entry: %d\n", inserted.key);
    assert(inserted.key == -1);
    assert(inserted.rid.pid == 5);
    assert(inserted.rid.sid == 6);

    // Need to be able to insert at least 70 keys
    // We've inserted 4 so far, so let's insert 67 more for 71 total
    assert(leafNode.getKeyCount() == 4);
    int manyKey = 42;
    RecordId manyRID; 
    manyRID.pid = 6; 
    manyRID.sid = 7;
    int manyInsertPoint = 0;
    LeafEntry manyInserted;
    for (int i = 0; i < 67; i++) {
        manyInsertPoint = offset + (leafNode.getKeyCount() * sizeof(LeafEntry));
        assert(leafNode.insert(manyKey, manyRID) == 0);
        // Update PageFile with latest node contents
        leafNode.write(0, pf);
        pf.read(0, buffer);

        // Check inserted key
        memcpy(&manyInserted, &buffer[manyInsertPoint], sizeof(LeafEntry));
        assert(manyInserted.key == manyKey);
        manyKey++;
        manyRID.pid++;
        manyRID.sid++;
    }
    assert(leafNode.getKeyCount() == 71);


    /// TESTING: locate()

    // We should now have: -1, 10, 12, 15, 42, 43, .., 108
    int eid = -1; 
    int searchKey = -1;
    assert(leafNode.locate(searchKey, eid) == 0);
    assert(eid == 0);

    eid = -1; 
    searchKey = 10; 
    assert(leafNode.locate(searchKey, eid) == 0);
    assert(eid == 1); 

    // Does not exist
    eid = -1; 
    searchKey = 13; 
    assert(leafNode.locate(searchKey, eid) == RC_NO_SUCH_RECORD);
    assert(eid == 3);

    eid = 1; 
    searchKey = 42; 
    assert(leafNode.locate(searchKey, eid) == 0); 
    assert(eid == 4);

    // Index of last entry should be getKeyCount() - 1
    eid = -1; 
    searchKey = 108; 
    leafNode.locate(searchKey, eid);
    // printf("eid: %d\n", eid);
    assert(leafNode.locate(searchKey, eid) == 0);
    assert(eid == leafNode.getKeyCount() - 1);
    assert(eid == 70);

    // And finally, the second-to-last entry
    eid = -1; 
    searchKey = 107; 
    leafNode.locate(searchKey, eid);
    assert(leafNode.locate(searchKey, eid) == 0);
    assert(eid == 69);


    /// TESTING: readEntry()

    // Reset to test what readEntry() gives back to us
    RecordId result;
    result.pid = -1; 
    result.sid = -1;
    searchKey = -1; 
    // Unchanged from above
    eid = 69; 
    assert(leafNode.readEntry(eid, searchKey, result) == 0);
    assert(searchKey == 107);
    assert(result.pid == 71);
    assert(result.sid == 72);

    result.pid = -1; 
    result.sid = -1; 
    searchKey = -1;
    eid = 0;
    assert(leafNode.readEntry(eid, searchKey, result) == 0);
    assert(searchKey == -1); 
    assert(result.pid == 5); 
    assert(result.sid == 6); 

    // Invalid index should give back RC_NO_SUCH_RECORD
    eid = -1; 
    assert(leafNode.readEntry(eid, searchKey, result) == RC_NO_SUCH_RECORD);

    eid = leafNode.getKeyCount();
    assert(leafNode.readEntry(eid, searchKey, result) == RC_NO_SUCH_RECORD);


    /// TESTING: insertAndSplit()
    BTLeafNode sibling;
    RecordId insert;
    insert.pid = 1;
    insert.sid = 2;
    int siblingKey = -1; 

    // Sibling should get half of the keys, with 109 being its last
    leafNode.insertAndSplit(11, insert, sibling, siblingKey);
    printf("Sibling key count: %d\n", sibling.getKeyCount());
    printf("Sibling's first key: %d\n", siblingKey);
    


/////////////////////////////////// TESTING NON LEAF NODE ////////////////////////////////////////////////////////////////////////////////////////
    // TESTING: Initial BTNonLeafNode state and getter/setter functions
    // Create PageFile that will store a non leaf node

    nonLeafNodeTest();
    pf.close();
    return 0;
}
