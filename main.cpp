/* 
 * File:   main.cpp
 * Author: aarmstrong
 *
 * Created on February 24, 2019, 10:51 AM
 */

#include "MemoryAllocator.h"

#include <MMU.h>
//Can't find .h file even though it's in the same folder and auto complete works
//#include <WritePermissionFaultHandler.h> 
//#include <PageFaultHandler.h>
#include <cstdlib>
#include <iostream>
#include <memory>

/**
   * WritePermissionFaultHandler - page fault handler
   * 
   * This handler always returns false to abort the operation.
   */
  class WritePermissionFaultHandler : public mem::MMU::FaultHandler {
  public:
    WritePermissionFaultHandler(){}
    
    /**
     * Run - handle fault
     * 
     * Increment fault count, save pmcb.
     * 
     * @param pmcb Processor Memory Control Block
     * @return bool, false
     */ 
    virtual bool Run(const mem::PMCB &pmcb) {
      std::cout << "Write Page Fault at address " << 
                pmcb.next_vaddress << "\n";
      return false;
    }
  };
  
  /**
   * PageFaultHandler - page fault handler
   * 
   * The handler returns true if it successfully allocated the required page
   * frames, false otherwise.
 */
  class PageFaultHandler : public mem::MMU::FaultHandler {
  public:
    PageFaultHandler(){}
    
    /**
     * Run - handle fault
     * 
     * 
     * @param pmcb Processor Memory Control Block
     * @return bool, true if successfully allocated page frame, false otherwise
     */
    virtual bool Run(const mem::PMCB &pmcb) {
        std::cout << "Read Page Fault Error at address " << 
                pmcb.next_vaddress << "\n";
        
//        if(allocator.AllocatePageFrames(
//                (std::hex >> pmcb.remaining_count)/kPageSize, 
//                &pmcb.next_vaddress)){
//            return true;
//        }
//        else{
//            return false;
//        }
        return false;
    }
  };


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

  delete memory;
  delete allocator;

  memory = NULL;
  allocator = NULL;
  return 0;
}
