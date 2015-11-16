#include <cassert> 
#include <cstdio>
#include <cstring>

#include "Bruinbase.h"
#include "RecordFile.h"
#include "PageFile.h"
#include "BTreeNode.h"

int main() 
{
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
        leafNode.write(0, pf);
        pf.read(0, buffer);
        memcpy(&manyInserted, &buffer[manyInsertPoint], sizeof(LeafEntry));
    }
    assert(leafNode.getKeyCount() == 71);

    /// TESTING: locate()

    // We should now have: -1, 10, 12, 15, 42, 43, .., 109
    // int eid = -1; 
    // int searchKey = -1;
    // assert(leafNode.locate(searchKey, eid) == 0);
    // assert(eid == 0);

    // eid = -1; 
    // searchKey = 10; 
    // assert(leafNode.locate(searchKey, eid) == 0):
    // assert(eid == 1); 

    // // Does not exist
    // eid = -1; 
    // searchKey = 13


    /// TESTING: readEntry()



    /// TESTING: insertAndSplit()
    


    pf.close();
    return 0;
}