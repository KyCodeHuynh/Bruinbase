 /*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include <cassert> 
#include <cstdio>
#include <cstring>
#include "BTreeIndex.h"
#include "BTreeNode.h"

// TODO: For debugging purposes only!
// #include <cstdio>

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    // Cannot yet assume that PageFile is loaded;
    // open() handles that. 

    // CRYSTAL 
    // Will be unnecessary after setInit() is fully implemented
    // isInitialized = false;

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
    // printf("DEBUG: Return code from BTreeIndex::pf.open(): %d\n", rc);

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

    // Don't forget to write back to disk!
    pf.write(0, buffer);
    if (rc < 0) {
        return rc;
    }

    return 0;
}

/**
* Return the init status of the tree, stored in page 0 of our internal PageFile
* Assumes that the PageFile has already been loaded.
* -1 means not initialized, 0 means empty, 1 means at least 1 node in the tree
*/
int BTreeIndex::getInit() const
{
    // STORAGE in Page 0: [rootPid, treeHeight, init status]
    char buffer[1024]; 
    int rc = pf.read(0, buffer); 
    if (rc < 0) {
        return rc;
    }

    int status = -1; 
    // PageId and treeHeight is stored first
    int offset = sizeof(PageId) + sizeof(int); 

    // C functions are not at all intuitive to read
    memcpy(&status, &buffer[offset], sizeof(int));
    return status;
}


/**
* Set init value of the tree, stored in page 0
* Assumes that the PageFile has already been loaded.
* This is meant to indicate whether it has been initialized or not - or just empty.
* @param newRootPid[IN] should be -1 if not initialized, 0 if empty, 1 if initialized
* @return error code. 0 if no error.
*/
RC BTreeIndex::setInit(int status)
{
    // STORAGE in Page 0: [rootPid, treeHeight, status]
    char buffer[1024]; 
    int rc = pf.read(0, buffer); 
    if (rc < 0) {
        return rc;
    }

    int offset = sizeof(PageId) + sizeof(int);
    memcpy(&buffer[offset], &status, sizeof(int));

    // Don't forget to write the change back to disk!
    pf.write(0, buffer);
    if (rc < 0) {
        return rc;
    }

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
    if (getInit() <= 0) {
        // DEBUG
        // printf("getRootPid() reports not yet initialized or its empty. Give 0 by default.\n");
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
        // DEBUG
        // printf("Else case of getRootPid(), so 0\n");
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

    // Don't forget to write the change back to disk!
    pf.write(0, buffer);
    if (rc < 0) {
        return rc;
    }

    return 0;
}

 /**
  * Recursive function to insert (key, RecordId) pairs
  * into the B+ tree index
  * @param curDepth[IN] the depth this recursive call is at
  * @param key[IN] the key we're inserting
  * @param rid[IN] the RecordId we're inserting
  * @param insertPid[IN] the PageId we're inserting (>= 1 when recursing on non-leaf; -1 otherwise)
  * @param visited[OUT] the stack of PageIds visited nodes, most recent on top
  * @return error code if error. 0 if successful.
  */
RC BTreeIndex::helperInsert(int curDepth, int key, const RecordId& rid, PageId insertPid, std::stack<PageId>& visited)
{
    // Idea: find() gives back a stack of visited nodes
    // (if the searchKey doesn't exist, find() gives back 
    // the PageId of the leaf node where it should exist)
    //
    // We try insertion, beginning at the leaf found by find().
    // Whenever we have overflow, we back to the parent node via 
    // the visited stack, where the parent is at the top() after
    // initial pop() for current node. This lets us recurse upward,
    // from handling overflow that "bubbles up" from the leaf level.

    // At root. Note that we get here after propagating upward
    // from overflows at levels below, so insertPid should be passed in.g
    if (curDepth == 0) {
        // Pop off top of visited stack into non-leaf node
        BTNonLeafNode current; 
        PageId curPid = visited.top();
        visited.pop();
        int rc = current.read(curPid, pf);
        if (rc < 0) {
            return rc;
        }

        // Try insertion first (PageId as this is a non-leaf node)
        rc = current.insert(key, insertPid);
        // Node full?
        if (rc == RC_NODE_FULL) {
            // insertAndSplit() into a new sibling
            BTNonLeafNode sibling;
            int midKey = 0;
            PageId siblingPid = pf.endPid();
            current.insertAndSplit(key, insertPid, sibling, midKey);

            // Write out updated sibling and current
            rc = current.write(curPid, pf);
            if (rc < 0) {
                return rc;
            }
            rc = sibling.write(siblingPid, pf);
            if (rc < 0) {
                return rc;
            }

            // Create a new root and initialize it
            BTNonLeafNode new_root;
            rc = new_root.initializeRoot(curPid, midKey, siblingPid);
            if (rc < 0) {
                return rc;
            }

            PageId rootPid = pf.endPid(); 
            rc = new_root.write(rootPid, pf);
            if (rc < 0) {
                return rc;
            }

            // Update rootPid and treeHeight
            setRootPid(rootPid);
            setTreeHeight(getTreeHeight() + 1);
        }
        else {
            // Insert succeded. Write out contents to PageFile
            current.write(curPid, pf);
            return 0;
        }

        
    }

    // Else if at a leaf node (with insertPid ignored)
    else if (curDepth == getTreeHeight()) {
        // Pop off top of visited stack into leaf node
        BTLeafNode current; 
        PageId curPid = visited.top(); 
        visited.pop();
        int rc = current.read(curPid, pf); 
        if (rc < 0) {
            return rc;
        }

        // Try insertion (RecordId as this is a leaf node)
        rc = current.insert(key, rid);

        // Overflow? 
        if (rc == RC_NODE_FULL) {
            // insertAndSplit() into a new sibling
            BTLeafNode sibling;
            int siblingKey = 0;
            PageId siblingPid = pf.endPid();
            current.insertAndSplit(key, rid, sibling, siblingKey);
            
            // Write out updated sibling and current
            rc = current.write(curPid, pf);
            if (rc < 0) {
                return rc;
            }
            rc = sibling.write(siblingPid, pf);
            if (rc < 0) {
                return rc;
            }

            // Update sibling pointer/PageId
            current.setNextNodePtr(siblingPid);
            sibling.setNextNodePtr(0);

            // Recurse with:
            // insertPid = siblingPid, the PageId of the new sibling
            // key = siblingKey
            // depth = depth - 1. 
            //
            // This will terminate in some level up above, when the 
            // siblingKey and siblingPid are inserted into some ancestor, 
            // possibly in a new root.
            //
            // NOTE: visited stack already modified by previous pop()
            helperInsert(curDepth - 1, siblingKey, rid, siblingPid, visited);
        }
        // Insertion attempt succeeded
        else {
            current.write(curPid, pf);
            return 0;
        }
    }
        

    // Else at a non-leaf node (0 < curDepth < treeHeight)
    else {
        // Pop off top of visited stack into leaf node
        BTNonLeafNode current; 
        PageId curPid = visited.top();
        visited.pop();
        int rc = current.read(curPid, pf);
        if (rc < 0) {
            return rc;
        }

        // Try insertion first (PageId as this is a non-leaf node)
        rc = current.insert(key, insertPid);


        // Try insertion with passed-in key and insertPid

        // Overflow?
        if (rc == RC_NODE_FULL) {
            // insertAndSplit() into a new sibling
            BTNonLeafNode sibling;
            int midKey = 0;
            PageId siblingPid = pf.endPid();
            current.insertAndSplit(key, insertPid, sibling, midKey);

            // Write out updated sibling and current
            rc = current.write(curPid, pf);
            if (rc < 0) {
                return rc;
            }
            rc = sibling.write(siblingPid, pf);
            if (rc < 0) {
                return rc;
            }

            // Recurse with:
            // curDepth = curDepth - 1
            // insertPid = siblingPid, the PageId of the new sibling
            // key = midKey
            //
            // NOTE: visited stack already modified by previous pop()
            helperInsert(curDepth - 1, midKey, rid, siblingPid, visited);
        }
        else {
            // Insert succeded. Write out contents to PageFile
            current.write(curPid, pf);
            return 0;
        }
    }

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
    // getInit() == -1 means it's not initialized, getInit() == 0 means it's empty
    if (getInit() <= 0) {

        // Initial rootPid and treeHeight are both 0
        char buffer[1024];

        // We cannot use the setter helpers, as 
        // page 0 is not yet allocated.
        memset(buffer, 0, 1024);

        // printf("DEBUG: endPid after write of metadata buffer: %d\n", pf.endPid());

        // endPid will automatically be updated to 1
        int rc = pf.write(0, buffer);
        if (rc < 0) {
            // printf("DEBUG: pf.write() gave error: %d\n", rc);
            return rc;
        }

        // printf("DEBUG: endPid after write of metadata buffer: %d\n", pf.endPid());

        // CRYSTAL 
        // Setting Init
        setInit(1);

        // Create leaf node and insert into page 1,
        // as no nodes at all existed until now
        BTLeafNode leaf_root;
        fprintf(stderr, "DEBUG: Created first leaf node: %d\n", key);
        rc = leaf_root.insert(key, rid);
        if (rc < 0) {
            // DEBUG
            // printf("Empty tree case leaf_root.insert() failed!\n");
            return rc;
        }

        // No sibling yet, so set sibling pointer/PageId to -1
        leaf_root.setNextNodePtr(0);

        // Make sure we write to page 1
        rc = leaf_root.write(1, pf);
        // DEBUG
        // printf("Wrote leaf_root to page 1. endPid() should now return 2: %d\n", pf.endPid());
        if (rc < 0) {
            return rc;
        }

        // DEBUG
        // if (key == 4) {
        //     IndexCursor cursor;
        //     int rc = locate(4, cursor);
        //     if (rc < 0) {
        //         assert(0);
        //     }
        // }

        // printf("DEBUG: Got past the leaf_root.write()!");

        // Update root pointer
        setRootPid(1);
        setTreeHeight(0);

        // DEBUG: Print out page 0 contents
        // printf("rootPid right after initial insert(): %d\n", getRootPid());
        // pf.read(0, buffer);
        // PageId testPid;
        // int testHeight;
        // // Order of storage: rootPid, treeHeight
        // memcpy(&testPid, &buffer, sizeof(PageId));
        // memcpy(&testHeight, &buffer[sizeof(PageId)], sizeof(int));

        // DEBUG
        // printf("Initial root PageId should be 1: %d\n", testPid);
        // printf("Initial tree height should be 0: %d\n", testHeight);

        // DEBUG: print page 1 contents
        // typedef struct LeafEntry {
        //     int key;
        //     RecordId rid;
        // } LeafEntry;    

        // pf.read(1, buffer);
        // int keyCount;
        // PageId nextPage;
        // memcpy(&keyCount, &buffer, sizeof(int));
        // memcpy(&nextPage, &buffer[sizeof(int)], sizeof(PageId));
        // printf("\nRight after initial insert()\n");
        // printf("Key count in buffer: %d\n", keyCount);
        // printf("Next PageId in buffer: %d\n", nextPage);

        // LeafEntry inserted; 
        // int offset = sizeof(int) + sizeof(PageId);
        // int x = 0;
        // printf("Entries in buffer: \n");
        // while(x < keyCount) {
        //     memcpy(&inserted, &buffer[offset], sizeof(LeafEntry));
        //     printf("key: %d\n", inserted.key);
        //     printf("pid: %d\n", inserted.rid.pid);
        //     printf("sid: %d\n", inserted.rid.sid);
        //     offset = offset + sizeof(LeafEntry);
        //     x++;
        // }
        
        // printf("DONE WITH INSERT IN INDEX\n");

        return 0;
    }

    // CASE 1: Only the root node exists
    //
    // Our root node begins as a leaf node, where we insert our 
    // first few (key, RecordId) pairs. After insertion, we use BTLeafNode::write()
    // to write the node contents to our internal PageFile to save them. 
    else if (getTreeHeight() == 0) {
        // Get leaf node
        BTLeafNode leaf_root; 
        fprintf(stderr, "DEBUG: Reached root for key: %d\n", key);
        int rc = leaf_root.read(getRootPid(), pf);
        if (rc < 0) {
            return rc;
        }

        // DEBUG: For seeing if unit-test logic is incorrect
        // int eid = -1;
        // rc = leaf_root.locate(4, eid);
        // printf("rc, eid: %d %d\n", rc, eid);


        // DEBUG:
        // printf("ROOT NODE EXISTS! YAY!\n");
        // printf("ROOTPID: %d\n", getRootPid());
        // printf("WORKING ON KEY %d\n", key);

        // printf("key count before: %d\n", leaf_root.getKeyCount());
        // rc = leaf_root.insert(key, rid);
        // printf("key count after: %d\n", leaf_root.getKeyCount());

        // Try insertion
        rc = leaf_root.insert(key, rid);

        // If our root node is full, we have leaf node overflow
        // We split into two leaf nodes, and create a parent non-leaf node
        // using BTNonLeafNode::initializeRoot()
        if (rc == RC_NODE_FULL) {
            // These will be filled out by insertAndSplit()
            BTLeafNode sibling;
            int siblingKey;
            leaf_root.insertAndSplit(key, rid, sibling, siblingKey);

            // Write out sibling to a new page
            int siblingPid = pf.endPid();
            rc = sibling.write(siblingPid, pf);
            if (rc < 0) {
                return rc;
            }

            // Update current leaf_root
            rc = leaf_root.write(getRootPid(), pf);
            if (rc < 0) {
                return rc;
            }

            // Set sibling pointer/PageId
            leaf_root.setNextNodePtr(siblingPid);
            sibling.setNextNodePtr(0);

            // Key inserted into parent is the first one in the new sibling
            // No need to have the new root be the first page,
            // which would be an expensive rearrangement. 
            BTNonLeafNode new_root;
            fprintf(stderr, "DEBUG: Created non leaf node: %d\n", siblingKey);

            // insertAndSplit() lets us know the key that should be stored in parent
            // getRootPid() here gets us the PageId for leaf_root, which is now left child
            rc = new_root.initializeRoot(getRootPid(), siblingKey, siblingPid);
            PageId rootPid = pf.endPid();
            rc = new_root.write(rootPid, pf);
            if (rc < 0) {
                return rc;
            }

            // Update rootPid and treeHeight
            setRootPid(rootPid);
            setTreeHeight(getTreeHeight() + 1);
        }
        else {
            // Insert succeded. Write out contents to PageFile
            leaf_root.write(getRootPid(), pf);
            return 0;
        }
    }

    // CASE 2: Root node + children exist
    else { 
        // Modified find() will record PageId's of nodes visited
        // Lets us avoid duplicating traversal algorithm
        // Initial insertPid is -1, as it's only used when we have overflow
        // Initial curDepth is tree height, as find() should ended on a leaf node
        std::stack<PageId> visited; 
        IndexCursor ignoreThis;
        find(key, ignoreThis, getTreeHeight(), getRootPid(), visited);
        int curDepth = getTreeHeight();
        int insertPid = -1;
        int rc = helperInsert(curDepth, key, rid, insertPid, visited);
        if (rc < 0) {
            return rc;
        }
        fprintf(stderr, "DEBUG: Inserted into an already-made node: %d\n", key);
    }
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

    // Insert succeeded
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
RC BTreeIndex::find(int searchKey, IndexCursor& cursor, int cur_tree_height, PageId cur_pid, std::stack<PageId>& visited) {
    // Update stack of visited nodes
    visited.push(cur_pid);

    // DEBUG: 
    // printf("cur_pid of find(): %d\n", cur_pid);

	// If we are at the leaf node
	if (cur_tree_height == 0) {
		// Try to get the node that the pid is pointing to
		BTLeafNode leafnode;
		int rc = leafnode.read(cur_pid, pf);
		if (rc < 0) {
            // DEBUG
            // printf("ERROR: the leafnode.read() failed in find()\n");
			return rc;
    	}

        // DEBUG
        // printf("\nKEY COUNT inside of find(): %d\n", leafnode.getKeyCount());

        // typedef struct LeafEntry {
        //     int key;
        //     RecordId rid;
        // } LeafEntry;    

        // int x = 0;

        // // Inspect contents of page 1
        // char buffer[1024];
        // pf.read(1, buffer);

        // PageId next_pageid;
        // memcpy(&next_pageid, &buffer[sizeof(int)], sizeof(PageId));
        // printf("next page id: %d\n", next_pageid);

        // LeafEntry inserted; 
        // int offset = sizeof(int) + sizeof(PageId);

        // while(x < leafnode.getKeyCount()) {
        //     memcpy(&inserted, &buffer[offset], sizeof(LeafEntry));
        //     printf("key: %d\n", inserted.key);
        //     printf("pid: %d\n", inserted.rid.pid);
        //     printf("sid: %d\n", inserted.rid.sid);
        //     offset = offset + sizeof(LeafEntry);
        //     x++;
        // }


    	// look for the searchKey, set cursor.eid
    	rc = leafnode.locate(searchKey, cursor.eid);
    	if (rc < 0) {
            printf("ERROR: the no such record occurred in leafnode.locate() [Line: %d]\n", __LINE__);
    		return rc;
    	}

        // DEBUG
        // printf("FOUND KEY: %d\n", searchKey);
        // printf("FOUND PID: %d\n", cur_pid);
        // printf("FOUND EID: %d\n", cursor.eid);
        // DEBUG
        int size = visited.size();
        fprintf(stderr, "DEBUG: visited size: %d\n", size);
        for (int i = 0; i < visited.size(); i++) {
            fprintf(stderr, "DEBUG: Visited: %d\n", visited.top());          
        }

        // Output PageId of located entry
        // cursor.eid is already set by BTLeafNode::locate() above
    	cursor.pid = cur_pid;
    	return 0;
	}
	// If we are at a non-leaf node
	else if (cur_tree_height > 0) {
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
            // DEBUG
            // printf("ERROR: the nonleafnode.locateChildPtr() gave back an error code.\n");
			return rc;
		}

        std::stack<PageId> ignoreThis;
		return find(searchKey, cursor, cur_tree_height - 1, new_pid, ignoreThis);
	} 
	// Shouldn't get to this point, but if for some reason, the height is 0
	else {
        // DEBUG 
        // printf("ERROR: No such record happened as else-case of find()\n");
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
	if (getTreeHeight() == -1) {
        // DEBUG
        // printf("ERROR: No such record happened at treeHeight check\n");
		return RC_NO_SUCH_RECORD;
	}
    else {
        // find() is our recursive helper above, 
        // and implements the standard B+ tree search algorithm
        // TODO: Double-check this function's last arg, 
        // which is now getting rootPid via helper
        std::stack<PageId> ignoreThis;
        // DEBUG
        // printf("rootPid just before find() invocation [Line %d]: %d\n", __LINE__, getRootPid());
        return find(searchKey, cursor, getTreeHeight(), getRootPid(), ignoreThis);
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