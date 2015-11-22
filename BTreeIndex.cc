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
	// Default: rootPid is -1, which mean no node has been created
    rootPid = -1;

    // Initialize tree height at 0 - there's no nodes
    treeHeight = 0;
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
    return pf.open(indexname, mode);
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

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{

	// *** FIRST NODE CASE ***
	//
	// When the rootPid is empty, it means there is no initialized nodes yet.
	// This takes the first key, rid value and inserts it into a leaf node
	// It then writes the node to the pagefile, updates values, and returns.

	// Check if it's empty
	if (rootPid == -1) {
		// Create the first leafnode
		BTLeafNode leafnode;
		// Insert the specified key and rid values
		int rc = leafnode.insert(key, rid);
		if (rc < 0 ) {
			return rc;
    	}

    	int new_pid = pf.endPid();
    	// Write the new leafnode into the PageFile pf
		rc = leafnode.write(new_pid, pf);
		if (rc < 0 ) {
			return rc;
    	}

    	// If everything succeeds,
    	// assign rootPid to this node and update treeHeight
    	rootPid = new_pid;
    	treeHeight++;
 		return 0;
	} else {

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
	}

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
	if (treeHeight == 0) {
		return RC_NO_SUCH_RECORD;
	}
    else {
        // find() is our recursive helper above, 
        // and implements the standard B+ tree search algorithm
        return find(searchKey, cursor, treeHeight, rootPid);
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