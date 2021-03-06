#include <cassert> 
#include <cstdio>
#include <cstring>

#include <cmath>
#include "BTreeNode.h"

using namespace std;

/*
 * Read the content of the node from the page pid in the PageFile pf
 * into our internal buffer. The PageFile pf is part of
 * the BTreeIndex class; each node occupies a page.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from (internal PageFile for B+ tree)
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
    if (pid < 0) {
        return RC_INVALID_PID;
    }
    // PageFile can read contents of a PageID into a buffer.
    // It will also give the appropriate return code.
    // PageID is something managed in SqlEngine.
    return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to (internal one for our B+ tree)
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
    // Write the contents of buffer to the page specified by PageID
    // Our internal buffer is where we modify the node contents.
    return pf.write(pid, buffer); 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
    // Key count is the first sizeof(int) bytes of the buffer
    int numKeys = 0; 
    memcpy(&numKeys, buffer, sizeof(int));

    return numKeys;
}

RC BTLeafNode::setKeyCount(int numKeys)
{
    int numKeysCopy = numKeys;
    memcpy(buffer, &numKeysCopy, sizeof(int));

    // TODO: Error cases? 
    return 0;
}

/*
 * Insert a (key, rid) pair to the node. Assumes no duplicate keys inserted.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
    // Leaf nodes hold (key, RecordID) entries, held in LeafEntry instances
    // Alas, C++ does not support tagged initialization like C99
    LeafEntry newEntry = { key, rid };
    
    // NOTE: The first sizeof(int) + sizeof(PageId) bytes are reserved
    // for the key count and next sibling PageId
    int offset = sizeof(int) + sizeof(PageId);
    int numKeys = getKeyCount();
    int bytesUsed = offset + (numKeys * sizeof(LeafEntry));

    // Check if node full, i.e., we don't have space
    // for another LeafEntry. 
    if ((PageFile::PAGE_SIZE - bytesUsed) < 0) {
        return RC_NODE_FULL;
    }

    // We need to keep the entries sorted by their keys
    // This is basically the problem of insertion into 
    // a sorted array (and keeping it sorted). 
    // Conventionally, there are two approaches, with the
    // second being a natural improvement on the first: 
    //
    // 1. Brute force. We scan linearly from the beginning 
    //    to find the appropriate spot for the new element, 
    //    then shift over all the succeeding elements to make
    //    space for the new one. 
    //
    // 2. Smarter search. We use binary search to find the spot 
    //    and then shift over for insertion. 
    //
    // However, there is actually a third approach: 
    //
    // 3. Shift as we go. Starting from the empty space just after
    //    the last sorted element, we check if the new element can 
    //    be inserted there. If not, we shift over the last element, 
    //    and check if the new element can be inserted into the spot freed up.
    //    We do so until until find a spot or hit the beginning of the array, 
    //    which is the special case of pre-pending. This approach is very nice
    //    for when we're making a fully-sorted list out of a nearly-sorted one. 
    //    Since we'd begin with an initially empty array, most of the initial 
    //    insertions would be instant, as we'd be appending the biggest number. 
    //
    // Of course, with any approach, we should first check if
    // the array/buffer is full, if the new element is invalid 
    // or a duplicate, etc. Here, we assume no duplicates, 
    // and keys may be 0 or negative. 

    // We just need numbers, rather than pointers, 
    // as we have an array of chars, so we can move around
    // in terms of buffer +/- number of bytes/chars. 
    int indexFirst = sizeof(int) + sizeof(PageId);
    int indexLast = indexFirst + (getKeyCount() * sizeof(LeafEntry));
    int indexCur = indexLast; 

    // The preceding value, initially the last sorted value
    LeafEntry valPrev;

    // Keep in mind that a[i] == *(a + i)
    // so we need to take address-of to get 
    // a pointer to a char rather than an actual char
    memcpy(&valPrev, &buffer[indexLast - sizeof(LeafEntry)], sizeof(LeafEntry));

    // Keep going while our new key is not in position
    // or we haven't hit the beginning
    while ( (key < valPrev.key) && (indexFirst < indexCur) ) {
        memmove(&buffer[indexCur], &buffer[indexCur - sizeof(LeafEntry)], sizeof(LeafEntry));

        // Update index and value to compare against, which moves us leftward
        // Without this, only the first two insertions work, as the remaining
        // ones always compare against the last element (as we would have forgotten
        // to update valPrev), leading to inaccurate placement.
        indexCur = indexCur - sizeof(LeafEntry);
        memcpy(&valPrev, &buffer[indexCur - sizeof(LeafEntry)], sizeof(LeafEntry));   
    }

    // If we're here, we've found the insertion spot for our arguments.
    // Note that LeafEntry is composed of two ints, so struct alignment
    // by compiler is not an issue. 
    LeafEntry newItem = { key, rid };
    memcpy(&buffer[indexCur], &newItem, sizeof(LeafEntry));

    // Don't forget to update the key count!
    setKeyCount(getKeyCount() + 1);

    return 0; 
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
 // TODO: Unit-test!
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 
    int offset = sizeof(int) + sizeof(PageId); 
    LeafEntry entry; 
    entry.key = key;
    entry.rid = rid;

    int searchIndex = getKeyCount() - 1;
    int midpoint = floor(getKeyCount() / 2);
    bool pastMid = false; 

    int indexFirst = sizeof(int) + sizeof(PageId);
    int indexLast = indexFirst + (getKeyCount() * sizeof(LeafEntry));
    int indexCur = indexLast;

    // If we go past the midpoint while looking for our
    // insertion spot, then we'll insert into the sibling node, 
    // which will always receive the latter half of the current node. 
    // Otherwise, we insert into the current node.
    LeafEntry valPrev;
    memcpy(&valPrev, &buffer[indexLast - sizeof(LeafEntry)], sizeof(LeafEntry));

    // Same spot-finding algorithm as insert(), but without actually moving items
    while ( (key < valPrev.key) && (indexFirst < indexCur) ) {
        if (searchIndex <= midpoint) {
            pastMid = true;
        }

        // Update index and value to compare against, which moves us leftward
        searchIndex -= 1;
        indexCur = indexCur - sizeof(LeafEntry);
        memcpy(&valPrev, &buffer[indexCur - sizeof(LeafEntry)], sizeof(LeafEntry));   
    }

    // Copy contents over to sibling node, which requires inserting
    // everything from midpoint onward. 
    LeafEntry copy;
    int count = 0;

    for (int copyIndex = offset + midpoint*sizeof(LeafEntry); copyIndex < indexLast; copyIndex += sizeof(LeafEntry)) {
        memcpy(&copy, &buffer[copyIndex], sizeof(LeafEntry));
        sibling.insert(copy.key, copy.rid);
        count++;
    }

    // Memset current node's latter half to 0, as those keys were moved
    // memset(buffer + midpoint, 0, PageFile::PAGE_SIZE - midpoint);
    
    //set the key count of the original node
    setKeyCount(getKeyCount()-count);

    //set the key count of the new node
    // setKeyCount(count);    


    // Insert our argument (key, RecordId) pair into the appropriate node 
    if (pastMid) {
        insert(entry.key, entry.rid);
    }
    else {
        sibling.insert(entry.key, entry.rid);
    }

    // Set siblingKey to be the first key after everything
    LeafEntry result; 
    sibling.readEntry(0, result.key, result.rid);
    siblingKey = result.key;

    return 0; 
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the first index entry
 * with key >= searchKey (i.e., the closest approximate index number)
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+ tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
    // printf("LOOKING FOR KEY: %d\n", searchKey);
    // printf("KEY COUNT - node: %d\n", getKeyCount());

    // Linear search of entries for matching key
    // Return its index within the node in the 'eid' parameter
    int offset = sizeof(int) + sizeof(PageId); 
    LeafEntry entry; 
    int searchPoint = offset; 
    int searchIndex = 0;

    // DEBUG
    PageId my_pid;
    memcpy(&my_pid, &buffer[sizeof(int)], sizeof(PageId));

    while (searchPoint < (offset + (getKeyCount() * sizeof(LeafEntry)))) {
        memcpy(&entry, &buffer[searchPoint], sizeof(LeafEntry));

        // // DEBUG
        // if (searchKey == 4) {
        //     printf("Inside of BTLeafNode::locate()\n");
        //     printf("entry.key: %d\n", entry.key);
        //     printf("searchKey arg: %d\n", searchKey);
        // }
        // Found the entry
        if (entry.key == searchKey) {
            eid = searchIndex;
            return 0;
        }

        // Since we're proceeding left-to-right in a sorted list, 
        // then a current entry key greater than our searchKey
        // implies that our sought-for key is not in this node.
        // Set eid to the index of the first node with key >= searchKey
        // Example: 1 3 5 7 8
        // locate() with searchKey of 3 = 1
        // locate() with searchKey of 4 = 2
        if (entry.key > searchKey) {
            eid = searchIndex;
            return RC_NO_SUCH_RECORD;
        }

        searchPoint += sizeof(LeafEntry);
        searchIndex += 1;
    }
        

    // Hit end of node without finding searchKey
    // Set eid to index of highest item
    eid = getKeyCount() - 1; 
    return RC_NO_SUCH_RECORD; 
}

/*
 * Read the (key, rid) pair from the eid entry.
 * eid is the index number of the sought-for key in this node
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ 
    // Given an entry index number, read out its 
    // (key, RecordID) pair from this node

    // Negative or overly large entry index? 
    if (eid < 0 || eid > (getKeyCount() - 1)) {
        // fprintf(stderr, "DEBUG: Invalid eid in BTLeafNode::readEntry() [Line: %d]\n", __LINE__);
        return RC_NO_SUCH_RECORD;
    }

    int offset = sizeof(int) + sizeof(PageId);
    LeafEntry entry; 
    // TODO: Is this offset off by 1? 
    memcpy(&entry, &buffer[offset + (eid * sizeof(LeafEntry))], sizeof(LeafEntry));
    key = entry.key;
    rid.pid = entry.rid.pid;
    rid.sid = entry.rid.sid;

    return 0; 
}

/*
 * Return the pid of the next sibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ 
    // Return internally stored pointer to next sibling
    PageId retVal; 
    memcpy(&retVal, &buffer[sizeof(int)], sizeof(PageId));
    return retVal; 
}

/*
 * Set the pid of the next sibling node. Assumes 
 * internal buffer loaded with contents of node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
    if (pid < 0) {
        return RC_INVALID_PID;
    }
    // Only BTreeIndex has the informational context for this
    // We copy this into the next sizeof(PageId) bytes after 
    // the first sizeof(int) bytes in the internal buffer. 
    // Pointer arithmetic moves us forward by the sizeof(dataType) 
    // for each +1 increment. We have chars though, so everything
    // is 1 byte.   
    memcpy(&buffer[sizeof(int)], &pid, sizeof(PageId));
    return 0; 
}


////// Start non-leaf node implementation


/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
    if (pid < 0) {
        return RC_INVALID_PID;
    }
    return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
    return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
    // Key count is the first sizeof(int) bytes of the buffer
    int numKeys = 0; 
    memcpy(&numKeys, buffer, sizeof(int));

    return numKeys;
}

RC BTNonLeafNode::setKeyCount(int numKeys)
{
    int numKeysCopy = numKeys;
    memcpy(buffer, &numKeysCopy, sizeof(int));

    return 0;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
    NonLeafEntry newEntry = { key, pid };
    
    int offset = sizeof(int) + sizeof(PageId);
    int numKeys = getKeyCount();
    int bytesUsed = offset + (numKeys * sizeof(NonLeafEntry));

    // Check if node full, i.e., we don't have space
    if ((PageFile::PAGE_SIZE - bytesUsed) < 0) {
        return RC_NODE_FULL;
    }

    int indexFirst = sizeof(int) + sizeof(PageId);
    int indexLast = indexFirst + (getKeyCount() * sizeof(NonLeafEntry));
    int indexCur = indexLast; 

    NonLeafEntry valPrev;
    memcpy(&valPrev, &buffer[indexLast - sizeof(NonLeafEntry)], sizeof(NonLeafEntry));

    // Keep going while our new key is not in position
    // or we haven't hit the beginning
    while ( (key < valPrev.key) && (indexFirst < indexCur) ) {
        memmove(&buffer[indexCur], &buffer[indexCur - sizeof(NonLeafEntry)], sizeof(NonLeafEntry));

        indexCur = indexCur - sizeof(NonLeafEntry);
        memcpy(&valPrev, &buffer[indexCur - sizeof(NonLeafEntry)], sizeof(NonLeafEntry));   
    }

    NonLeafEntry newItem = { key, pid };
    memcpy(&buffer[indexCur], &newItem, sizeof(NonLeafEntry));

    // Don't forget to update the key count!
    setKeyCount(getKeyCount() + 1);

    return 0; 
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{
    int offset = sizeof(int) + sizeof(PageId); 
    int count = 0;

    insert(key, pid);

    int midIndex = offset + (floor(getKeyCount() / 2))*sizeof(NonLeafEntry);
    int indexLast = offset + (getKeyCount() * sizeof(NonLeafEntry));

    NonLeafEntry midkeyEntry;    
    memcpy(&midkeyEntry, &buffer[midIndex], sizeof(NonLeafEntry));
    midKey = midkeyEntry.key;


    NonLeafEntry copy;
    for (int copyIndex = midIndex; copyIndex < indexLast; copyIndex += sizeof(NonLeafEntry)) {
        memcpy(&copy, &buffer[copyIndex], sizeof(NonLeafEntry));
        sibling.insert(copy.key, copy.pid);           
        count++; 
    }

    setKeyCount(getKeyCount()-count);

    return 0; 
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid. Recall that these PageId pointers lead to other 
 * non-leaf nodes or to a leaf node
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
    // Search entries from beginning of non-leaf node
    // Example node keys: -1, 0, 3, 5, 7, 10
    // searchKey: 4.
    // We'd want to return the PageId associated with 
    // key 5, the first one such that searchKey <= to it
    int offset = sizeof(int) + sizeof(PageId); 
    NonLeafEntry entry; 
    // int searchPoint = offset; 
    // int lastEntry = offset + (getKeyCount() * sizeof(NonLeafEntry)); 
    int firstPoint = offset; 
    int searchPoint = offset + ((getKeyCount()-1) * sizeof(NonLeafEntry)); 

    // We are going to start at the end, and make our way down
    // right to left to find the right key

    while (searchPoint >= firstPoint) {
        memcpy(&entry, &buffer[searchPoint], sizeof(NonLeafEntry));

        // Found an exact key match or one greater than our searchKey
        // Handles the case where searchKey <= all current keys, 
        // in which case we give back the PageId in the leftmost entry, 
        // which would happen upon the first iteration of this loop.
        if (searchKey >= entry.key) {
            pid = entry.pid;
            return 0;
        }

        searchPoint -= sizeof(NonLeafEntry);
    }
        
    // Edge case: hit the leftmost, and all current keys
    // are > searchKey. Return PageId of leftmost entry.
    PageId leftmost;
    memcpy(&leftmost, &buffer[sizeof(int)], sizeof(PageId));
    pid = leftmost;
    return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
    int bytesUsed = sizeof(int) + (getKeyCount() * sizeof(NonLeafEntry));   
    if ((PageFile::PAGE_SIZE - bytesUsed) < 0) {
        return RC_NODE_FULL;
    }

    NonLeafEntry old;

    if (getKeyCount() != 0) {
        memcpy(&old, &buffer[sizeof(int)+sizeof(PageId)], sizeof(NonLeafEntry));
    }

    NonLeafEntry newItem = { key, pid2 };
    PageId first_pid = pid1;
    memcpy(&buffer[sizeof(int)], &first_pid, sizeof(PageId));
    memcpy(&buffer[sizeof(int)+sizeof(PageId)], &newItem, sizeof(NonLeafEntry));

    // Don't forget to update the key count!
    // Until we find out whether the root node is empty before initializing, I'll do a safe route:
    if (getKeyCount() != 0) {
        // insert adjusts the key count
        insert(old.key, old.pid);
    } else {
        setKeyCount(1);
    }
    // if the count is greater than 1, that means there's already keys inside.. should I move them? 
    // if I DO MOVE THEM: i would add 1 to the key count, re-insert that key into the node lol

    return 0;
}
