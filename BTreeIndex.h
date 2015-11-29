/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#ifndef BTREEINDEX_H
#define BTREEINDEX_H

#include <stack>

#include "Bruinbase.h"
#include "PageFile.h"
#include "RecordFile.h"
             
/**
 * The data structure to point to a particular entry at a b+tree leaf node.
 * An IndexCursor consists of pid (PageId of the leaf node) and 
 * eid (the location of the index entry inside the node).
 * IndexCursor is used for index lookup and traversal.
 */
typedef struct {
  // PageId of the index entry
  PageId  pid;  
  // The entry number inside the node
  int     eid;  
} IndexCursor;

/**
 * Implements a B-Tree index for bruinbase.
 * 
 */
class BTreeIndex {
 public:
  BTreeIndex();

  /**
   * Open the index file in read or write mode.
   * Under 'w' mode, the index file should be created if it does not exist.
   * @param indexname[IN] the name of the index file
   * @param mode[IN] 'r' for read, 'w' for write
   * @return error code. 0 if no error
   */
  RC open(const std::string& indexname, char mode);

  /**
   * Close the index file.
   * @return error code. 0 if no error
   */
  RC close();
    
  /**
   * Insert (key, RecordId) pair to the index.
   * @param key[IN] the key for the value inserted into the index
   * @param rid[IN] the RecordId for the record being inserted into the index
   * @return error code. 0 if no error
   */
  RC insert(int key, const RecordId& rid);

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
   * @return 0 if searchKey is found. Othewise, an error code
   */
  RC locate(int searchKey, IndexCursor& cursor);

  /**
   * Read the (key, rid) pair at the location specified by the index cursor,
   * and move forward the cursor to the next entry.
   * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
   * @param key[OUT] the key stored at the index cursor location
   * @param rid[OUT] the RecordId stored at the index cursor location
   * @return error code. 0 if no error
   */
  RC readForward(IndexCursor& cursor, int& key, RecordId& rid);

  // TODO: PUT THESE TWO FUNCTIONS BACK TO PRIVATE

  /**
  * Return the height of the tree, stored in page 0 of our internal PageFile
  * Assumes that the PageFile has already been loaded.
  */
  int getTreeHeight() const;

  /**
  * Return the rootPid of the tree, stored in page 0 of our internal PageFile
  * Assumes that the PageFile has already been loaded.
  */
  PageId getRootPid() const;

 private:

  /**
  * Recursive function to search through the nodes
  * to find the searchKey 
  * @param searchKey[IN] the key that we're looking for
  * @param cur_tree_height[IN] current tree height
  * @param cur_pid[IN] current page id of the current node
  * @param cursor[OUT] the cursor pointing to the index entry
  * @param visited[OUT] the stack of PageId's of visited nodes
  * @return error code if error. 0 if successful.
  */
  RC find(int searchKey, IndexCursor& cursor, int cur_tree_height, PageId cur_pid, std::stack<PageId>& visited);

  /**
  * Recursive function to insert (key, RecordId) pairs
  * into the B+ tree index
  * @param curDepth[IN] the depth this recursive call is at
  * @param key[IN] the key we're inserting
  * @param rid[IN] the RecordId we're inserting
  * @param insertPid[IN] the PageId we're inserting (>= 1 when recursing on non-leaf; -1 otherwise)
  * @param visited[OUT] the stack of PageId's of visited nodes, most recent on top
  * @return error code if error. 0 if successful.
  */
  RC helperInsert(int curDepth, int key, const RecordId& rid, PageId insertPid, std::stack<PageId>& visited);



  /**
  * Set new height of the tree, stored in page 0
  * Assumes that the PageFile has already been loaded.
  * @param newHeight[IN] the new height of the tree
  * @return error code. 0 if no error.
  */
  RC setTreeHeight(int newHeight);



  /**
  * Set new rootPid of the tree, stored in page 0
  * Assumes that the PageFile has already been loaded.
  * @param newRootPid[IN] the new rootPid of the tree
  * @return error code. 0 if no error.
  */
  RC setRootPid(int newRootPid);

  /**
  * Return the init status of the tree, stored in page 0 of our internal PageFile
  * Assumes that the PageFile has already been loaded.
  * -1 means not initialized, 0 means empty, 1 means at least 1 node in the tree
  */
  int getInit() const;

  /**
  * Set init value of the tree, stored in page 0
  * Assumes that the PageFile has already been loaded.
  * This is meant to indicate whether it has been initialized or not - or just empty.
  * @param newRootPid[IN] should be -1 if not initialized, 0 if empty, 1 if initialized
  * @return error code. 0 if no error.
  */
  RC setInit(int status);

  PageFile pf;         /// the PageFile used to store the actual b+tree in disk

  // See if PageFile loaded yet
  bool isInitialized;
  // PageId   rootPid;    /// the PageId of the root node
  // int      treeHeight; /// the height of the tree

  // PageId rootPid and int treeHeight are stored 
  // in Page 0 of our internal PageFile.
  // Order of storage: rootPid, treeHeight, 
  // occupying the first sizeof(PageId) + sizeof(int) bytes
  //
  // Get/set them through the helpers above only. 
  // This keeps us from forgetting to update
  // the PageFile with the latest values of our class members


  /// Note that the content of the above two variables will be gone when
  /// this class is destructed. Make sure to store the values of the two 
  /// variables in disk, so that they can be reconstructed when the index
  /// is opened again later.
};

#endif /* BTREEINDEX_H */
