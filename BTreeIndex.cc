/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    // Cannot yet assume that PageFile is loaded;
    // open() handles that. 

    isInitialized = false;

    // TODO: Do we actually use these class members anywhere? 
    // Remove if not.

	// // Default: rootPid is -1, which mean no node has been created
 //    rootPid = -1;

 //    // Initialize tree height at 0 - there's no nodes
 //    treeHeight = 0;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
	// Open the index file
	// Using the PageFile documentation for open()
	// Will create an index file if it does not exist, and will return proper error codes
    int rc = pf.open(indexname, mode);

    if (rc < 0) {
        return rc;
    }

    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	// Close the index file
	// Using the PageFile documentation for close()
	// Will create an index file if it does not exist, and will return proper error codes
    return pf.close();
}


/**
* Return the height of the tree, stored in page 0 of our internal PageFile
* Assumes that the PageFile has already been loaded.
*/
int BTreeIndex::getTreeHeight() const
{
    // STORAGE in Page 0: [rootPid, treeHeight]
    char buffer[1024]; 
    int rc = pf.read(0, buffer); 
    if (rc < 0) {
        return rc;
    }

    int treeHeight = 0; 
    int offset = sizeof(PageId);

    // C functions are not at all intuitive to read
    memcpy(&treeHeight, &buffer[offset], sizeof(int));

    // Sanity check, in case PageFile was not yet initialized
    if (treeHeight >= 0) {
        return treeHeight;
    }
    else {
        return -1;
    }
}


/**
* Set new height of the tree, stored in page 0
* Assumes that the PageFile has already been loaded.
* @param newHeight[IN] the new height of the tree
* @return error code. 0 if no error.
*/
RC BTreeIndex::setTreeHeight(int newHeight)
{
    // STORAGE in Page 0: [rootPid, treeHeight]
    char buffer[1024]; 
    int rc = pf.read(0, buffer); 
    if (rc < 0) {
        return rc;
    }

    int offset = sizeof(PageId);

    // C functions are not at all intuitive to read
    memcpy(&buffer[offset], &newHeight, sizeof(int));

    // TODO: Error cases and handling? 
    return 0;
}


/**
* Return the rootPid of the tree, stored in page 0 of our internal PageFile
* Assumes that the PageFile has already been loaded.
*/
PageId BTreeIndex::getRootPid() const
{
    // PageFile does not yet have contents
    // or is not yet loaded, so no actual root yet
    if (! isInitialized) {
        return 0;
    }
    // STORAGE in Page 0: [rootPid, treeHeight]
    char buffer[1024]; 
    int rc = pf.read(0, buffer); 
    if (rc < 0) {
        return rc;
    }

    int rootPid = 0; 
    // PageId is stored first
    int offset = 0; 

    // C functions are not at all intuitive to read
    memcpy(&rootPid, &buffer[offset], sizeof(PageId));

    // Sanity check, in case PageFile was not yet initialized
    if (rootPid >= 1) {
        return rootPid;
    }
    else {
        return 0;
    }
 
}


/**
* Set new rootPid of the tree, stored in page 0
* Assumes that the PageFile has already been loaded.
* @param newRootPid[IN] the new rootPid of the tree
* @return error code. 0 if no error.
*/
RC BTreeIndex::setRootPid(int newRootPid)
{
    // STORAGE in Page 0: [rootPid, treeHeight]
    char buffer[1024]; 
    int rc = pf.read(0, buffer); 
    if (rc < 0) {
        return rc;
    }

    int offset = 0;

    // C functions are not at all intuitive to read
    memcpy(&buffer[offset], &newRootPid, sizeof(PageId));

    // TODO: Error cases and handling? 
    return 0;
}


/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    // CASE 0: Root node does not yet exist
    //
    // Reserve page 0 for tree information, 
    // to allow a consistent buffer format for nodes
    // in all remaining pages. The first node of the 
    // tree will be found in page 1. 
    if (isInitialized == false) {
        // Initial rootPid and treeHeight are both 0
        char buffer[1024];

        // We cannot use the setter helpers, as 
        // page 0 is not yet allocated.
        memset(buffer, 0, 1024);

        // endPid will automatically be updated to 1
        pf.write(0, buffer);

        isInitialized = true;

        // TODO: Create leaf node and insert into page 1
        BTLeafNode leaf_root;

        // TODO: Update rootPid and treeHeight
    }

    // CASE 1: Only the root node exists
    //
    // Our root node begins as a leaf node, where we insert our 
    // first few (key, RecordId) pairs. After insertion, we use BTLeafNode::write()
    // to write the node contents to our internal PageFile to save them. 
    else if (getTreeHeight() == 0) {
        // If our root node is full, we have leaf node overflow
        // We split into two leaf nodes, and create a parent non-leaf node
        // using BTNonLeafNode::initializeRoot()

        // TODO: Update the rootPid found in page 0
        // TODO: Update the treeHeight found in page 0
    }

    // CASE 2: Root node + children exist

    // TODO: See Crystal's notes below



	// Check if it's empty
	// if (rootPid == -1) {
	// 	// Create the first leafnode
	// 	BTLeafNode leafnode;
	// 	// Insert the specified key and rid values
	// 	int rc = leafnode.insert(key, rid);
	// 	if (rc < 0 ) {
	// 		return rc;
 //    	}

 //    	int new_pid = pf.endPid();
 //    	// Write the new leafnode into the PageFile pf
	// 	rc = leafnode.write(new_pid, pf);
	// 	if (rc < 0 ) {
	// 		return rc;
 //    	}

 //    	// If everything succeeds,
 //    	// assign rootPid to this node and update treeHeight
 //    	rootPid = new_pid;
 //    	treeHeight++;
 // 		return 0;
	// } else {

 //    }

    
		// Crystal: I think we need a recursive function
		// 			We need to locate where the node is supposed to go
		//			Try to insert stuff... and if it's full, do something else
		//			Recursion will help to keep track of stuff. 

		// GENERAL IDEA:
		//
		// 1. Look for where the node is SUPPOSED to go...
		// 2. Figure out qualities of the node
		//		IF IT'S ROOTPID ... 
		//			(idk if there's any case where it would be root and NOT need a change)
		//			a) insert() returns not full, insert successfully
		//			b) returns full, deal with "new node insert"
		// 		IF IT'S LEAF NODE:
		//			a) insert() returns not full, insert successfully.
		//			b) returns full, deal with "leaf overflow"
		//				(also involves non-leaf inserting)
		//		IF IT'S NON LEAF NODE:
		//			a) insert() returns not full, insert successfully.
		//			b) returns full, deal with "non leaf overflow"  

		// Check if you're in a leaf node
		// If you are, you're going to try to insert your key + rid !
		// But you should first check if it will cause overflow.. cuz then u will need to do overflow+split

			// leaf overflow
			// if the parent of this is ALSO full.. need to do overflow again
	        // TODO: Write node contents to disk with BT(Non)LeafNode::write()
	        // Update treeHeight when?

		// Check if you're in a non-leaf node
		// you got to search where it has to go

			// non-leaf overflow -- involves insertAndSplit
			// if the parent is ALSO full, overflow again.. recursion
	    // TODO: Write node contents to disk with BT(Non)LeafNode::write()
	        // Update treeHeight when?


    return 0;
}

/**
* Recursive function to search through the nodes
* to find the searchKey. Helper to locate().
* @param searchKey[IN] the key that we're looking for
* @param cur_tree_height[IN] current tree height
* @param cur_pid[IN] current page id of the current node
* @param cursor[OUT] the cursor pointing to the index entry
*/
RC BTreeIndex::find(int searchKey, IndexCursor& cursor, int cur_tree_height, PageId cur_pid) {
	// If we are at the leaf node
	if (cur_tree_height == 1) {
		// Try to get the node that the pid is pointing to
		BTLeafNode leafnode;
		int rc = leafnode.read(cur_pid, pf);
		if (rc < 0) {
			return rc;
    	}

    	// look for the searchKey, set cursor.eid
    	rc = leafnode.locate(searchKey, cursor.eid);
    	if (rc < 0) {
    		return rc;
    	}

        // Output PageId of located entry
        // cursor.eid is already set by BTLeafNode::locate() above
    	cursor.pid = cur_pid;
    	return 0;
	}
	// If we are at a non-leaf node
	else if (cur_tree_height > 1) {
		BTNonLeafNode nonleafnode;
		int rc = nonleafnode.read(cur_pid, pf);
		if (rc < 0) {
			return rc;
		}

		PageId new_pid = -1;
        // locateChildPtr() gives the child pointer to follow,
        // given a searchKey. We do this to traverse the tree.
		rc = nonleafnode.locateChildPtr(searchKey, new_pid);
		if (rc < 0) {
			return rc;
		}

		return find(searchKey, cursor, cur_tree_height - 1, new_pid);
	} 
	// Shouldn't get to this point, but if for some reason, the height is 0
	else {
		return RC_NO_SUCH_RECORD;
	}
}	

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	// If it's empty, return RC_NO_SUCH_RECORD
	if (getTreeHeight() == 0) {
		return RC_NO_SUCH_RECORD;
	}
    else {
        // find() is our recursive helper above, 
        // and implements the standard B+ tree search algorithm
        // TODO: Double-check this function's last arg, 
        // which is now getting rootPid via helper
        return find(searchKey, cursor, getTreeHeight(), getRootPid());
    }

    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move forward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    // Assuming this is a leaf node, so we have a RecordId
    // Use readEntry(), then update cursor.eid += 1

    // Spin up a new leaf node with the pointed-to contents
    BTLeafNode leaf;
    int rc = leaf.read(cursor.pid, pf);
    if (rc < 0) {
        return rc;
    }

    // Get the wanted contents. 
    rc = leaf.readEntry(cursor.eid, key, rid);
    if (rc < 0) {
        return rc;
    }

    // Update the IndexCursor
    cursor.eid += 1;
    
    return 0;
}