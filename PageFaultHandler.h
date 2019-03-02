#ifndef PF_HANDLER
#define PF_HANDLER

#include <MMU.h>

#include <iostream>
#include  <iomanip>

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
      std::cout << "Write Page Fault at address "
                << std::setfill('0') << std::setw(7)
                << std::hex << pmcb.next_vaddress << "\n";
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
      //std::cout << pmcb.operation_state << "!!!\n";
      if(pmcb.operation_state == mem::PMCB::WRITE_OP){
        /* std::cout << "READ\n"; */
        /* std::cout << std::hex << unsigned(*pmcb.user_buffer) << "\n"; */
        std::cout << "Write Page Fault Error at address "
                  << std::setfill('0') << std::setw(7)
                  << std::hex << pmcb.next_vaddress << "\n";
      } else{

        std::cout << "Read Page Fault Error at address "
                  << std::setfill('0') << std::setw(7)
                  << std::hex << pmcb.next_vaddress << "\n";
      }

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

#endif
