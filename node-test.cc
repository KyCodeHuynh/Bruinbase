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

    // Now try setting the sibling PageId/pointer 
    // to an arbitrary PageId and getting it back
    assert(leafNode.setNextNodePtr(10) == 0);
    assert(leafNode.getNextNodePtr() == 10);

    pf.close();
    return 0;
}