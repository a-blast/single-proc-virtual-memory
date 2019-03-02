/* 
 * File:   main.cpp
 * Author: aarmstrong
 *
 * Created on February 24, 2019, 10:51 AM
 */

#include "MemoryAllocator.h"
#include "PageFaultHandler.h"
#include "Process.h"

#include <MMU.h>
//Can't find .h file even though it's in the same folder and auto complete works
//#include <WritePermissionFaultHandler.h> 
#include <cstdlib>
#include <iostream>
#include <memory>



int main(int argc, char** argv) {

  if (argc != 2) {
    std::cerr << "usage: Program2 trace_file\n";
    exit(1);
  }

  // 1. Create a instance of MMU w/ 128 (0x80) page frames
  mem::MMU* memory = new mem::MMU(128);

  // initialize an allocator that points to the MMU physical mem instance
  MemoryAllocator* allocator = new MemoryAllocator(128, memory);

  // Check that present bit is observed
  std::shared_ptr<PageFaultHandler>
    pf_handler(std::make_shared<PageFaultHandler>());  // define a page fault handler
  memory->SetPageFaultHandler(pf_handler);
    
  // Set up write fault handler
  std::shared_ptr<WritePermissionFaultHandler>
    wpf_handler(std::make_shared<WritePermissionFaultHandler>());
  memory->SetWritePermissionFaultHandler(wpf_handler);

  Process proc(argv[1], memory, allocator);
  proc.Exec();
  //std::istringstream outStream(proc.getStream());


  delete memory;
  delete allocator;

  memory = NULL;
  allocator = NULL;
  return 0;
}
