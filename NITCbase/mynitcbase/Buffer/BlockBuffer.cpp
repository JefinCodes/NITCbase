#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>


int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {
  // returns 1 if attr1 greater 0 if equal and -1 if lesser
  double diff;
  if(attrType == STRING)
      diff = strcmp(attr1.sVal, attr2.sVal);
  else
      diff = attr1.nVal - attr2.nVal;

  if(diff > 0) return 1;
  if(diff < 0) return -1;
  if(diff == 0) return 0;

  return 0;
}


// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
  this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head) {
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;   // return any errors that might have occured in the process
  }

  // populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->numSlots, bufferPtr + 24, 4);
  memcpy(&head->numEntries, bufferPtr + 16, 4);
  memcpy(&head->numAttrs, bufferPtr + 20, 4);
  memcpy(&head->rblock, bufferPtr + 12, 4);
  memcpy(&head->lblock, bufferPtr + 8, 4);

  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  struct HeadInfo head;

  // get the header using this.getHeader() function
  this->getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;
  
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotCount + (recordSize * slotNum);

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;

    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if(ret!=SUCCESS)
      return ret;

    /* get the header of the block using the getHeader() function */
    HeadInfo head;
    this->getHeader(&head); // same as BlockBuffer::getHeader(&head); or this->getHeader(&head)

    // get number of attributes in the block.
    int attrCount=head.numAttrs;

    // get the number of slots in the block.
    int slotCount=head.numSlots;
    if(slotNum<0 || slotNum>=slotCount)
      return E_OUTOFBOUND;

    /* calculate buffer + offset */
    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       (hint: a record will be of size ATTR_SIZE * numAttrs)
       */
      
    int recordSize=attrCount*ATTR_SIZE;
    unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotCount + recordSize*slotNum; 
      
    // copy the record from `rec` to buffer using memcpy
    memcpy(slotPointer, rec, recordSize);

    // update dirty bit using setDirtyBit()
    StaticBuffer::setDirtyBit(this->blockNum);

    /* (the above function call should not fail since the block is already
    in buffer and the blockNum is valid. If the call does fail, there
    exists some other issue in the code) */
    
    return SUCCESS;
}


int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **bufferPtr){
  // check if block is alr present
  int bufferNum=StaticBuffer::getBufferNum(this->blockNum);
  if(bufferNum != E_BLOCKNOTINBUFFER){
    // block alraedy in buffer
    
    // timestamps of all other occupied buffers in BufferMetaInfo.
    for(int i=0;i<BUFFER_CAPACITY;i++)
      if(!StaticBuffer::metainfo[i].free)
      StaticBuffer::metainfo[i].timeStamp++;
    
    StaticBuffer::metainfo[bufferNum].timeStamp=0;
    // set the timestamp of the corresponding buffer to 0 and increment the
  }else{
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
    
    if(bufferNum == E_OUTOFBOUND)
      return E_OUTOFBOUND;

    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }
    
  
  *bufferPtr=StaticBuffer::blocks[bufferNum];
  return SUCCESS;
}


/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  // get the header of the block using getHeader() function
  this->getHeader(&head);

  int slotCount = head.numSlots;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  memcpy(slotMap, slotMapInBuffer, slotCount);

  return SUCCESS;
}
