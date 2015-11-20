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

int PAGE_ID_MAX = 5;
int KEY_COUNT_MAX = 4;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
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
	// Check if it's empty
	// If so, put the first key in a leaf node!
	// I'm considering having an initialized root node, with it pointing to 1-key leaf node

	// Check if you're in a leaf node
	// If you are, you're going to try to insert your key + rid !
	// But you should first check if it will cause overflow.. cuz then u will need to do overflow+split

		// leaf overflow
		// if the parent of this is ALSO full.. need to do overflow again


	// Check if you're in a non-leaf node
	// you got to search where it has to go

		// non-leaf overflow -- involves insertAndSplit
		// if the parent is ALSO full, overflow again.. recursion

    return 0;
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


	// If it's NOT empty -- start at the root node
		// if you're at a non-leaf node
		// use locateAtChildPtr() to get the key,pid
		// follow the pid to the next node - do the same thing

		// if you're at a leaf node
		// use locate() to get the key, pid, rid match
			// if you find the right one, set IndexCursor.pid = PageId and IndexCursor.eid = searchKey index entry #
			// if you DON'T find the right one, set IndexCursor = PageId of leaf node and eid = read description..?

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
	// TODO: I'm not quite sure how to set up the buffer for this
	// pf.read(cursor.pid, void *buffer);

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