#include <iostream>

#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

void printRelationData(){

  // create objects for the relation catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);

  HeadInfo relCatHeader;

  // load the header into relCatHeader
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);

  for (int i = 0; i < relCatHeader.numEntries; i++) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    int attrCatBlockNum = ATTRCAT_BLOCK;
    
    while(attrCatBlockNum != -1){
      
      // object for attribute catalog
      RecBuffer attrCatBuffer(attrCatBlockNum);
      
      HeadInfo attrCatHeader;
      attrCatBuffer.getHeader(&attrCatHeader);
    
      for (int j = 0; j < attrCatHeader.numEntries; j++) {

        // declare attrCatRecord and load the attribute catalog entry into it
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRecord, j);

        if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal) == 0) {
          const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
          printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
        }
      }
      
      attrCatBlockNum = attrCatHeader.rblock;
    }
    
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  
  printRelationData();
  
  // Program to change attribute name
  
  printf("Change attribute name of your desired relation attribute\n");
  char relName[16];
  char attrName[16];
  char newName[16];
  printf("Enter name of relation: ");
  scanf("%s", relName);
  printf("Enter name of attribute: ");
  scanf("%s", attrName);
  printf("New name: ");
  scanf("%s", newName);
  
  int attrCatBlockNum = ATTRCAT_BLOCK;
  
  while(attrCatBlockNum != -1){
    
    // object for attribute catalog
    RecBuffer attrCatBuffer(attrCatBlockNum);
    
    HeadInfo attrCatHeader;
    attrCatBuffer.getHeader(&attrCatHeader);
  
    for (int j = 0; j < attrCatHeader.numEntries; j++) {

      // declare attrCatRecord and load the attribute catalog entry into it
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBuffer.getRecord(attrCatRecord, j);

      if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relName) == 0 && strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrName) == 0) {
        strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName);
        attrCatBuffer.setRecord(attrCatRecord, j);
        printf("Name Changed Successfully!\n");
      }
    }
    
    attrCatBlockNum = attrCatHeader.rblock;
  }
  
  printRelationData();

  return 0;
}
