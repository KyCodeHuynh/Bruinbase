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

    // TODO: We need to keep the entries sorted by their keys
    // Implementing insertion sort initially, which is O(n^2)
    // TODO: Replace with in-place quicksort? 


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
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 
    return 0; 
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
    // TODO: Linear search of entries for matching key
    // Return its index within the node in the 'eid' parameter

    // TODO: Replace with binary search
    return 0; 
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
    // TODO: Given an entry index number, read out its (key, RecordID) pair
    // from this node
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
    memcpy(&retVal, (buffer + sizeof(int)), sizeof(PageId));
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
    memcpy((buffer + sizeof(int)), &pid, sizeof(PageId));
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
{ return 0; }
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ return 0; }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ return 0; }


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ return 0; }

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
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ return 0; }

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ return 0; }
