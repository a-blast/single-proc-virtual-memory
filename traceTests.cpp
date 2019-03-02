
#include "gtest/gtest.h"
#include "Process.h"
# include "MemoryAllocator.h"
# include "PageFaultHandler.h"
# include <MMU.h>
# include <algorithm>
# include <string>
# include <iostream>

// TODO need to redirect process output to a logging class so it can be
// validated in this test.

auto processOutputGetter =
  [](std::string filePath, mem::MMU *memory, MemoryAllocator *allocator){
    Process proc(filePath, memory, allocator);
    proc.Exec();
    std::istringstream outStream(proc.getStream());
    return outStream;
  };
auto getExpectedOutput =
  [](std::string filePath){
    std::ifstream testFile;
    testFile.open(filePath);
    if (testFile.fail()){
      throw std::runtime_error("ERROR: file not found: "+ filePath);
    }
    return testFile;
  };
auto validateOutput =
  [](std::string output, std::ifstream validationStream,
     bool debug, int breakCount = 5){
    std::string outputLine;
    std::string validationLine;
    std::istringstream outputStream(output);
    bool linesAreEqual;
    int failureCounter = 0;
    while(getline(outputStream, outputLine)){
      getline(validationStream, validationLine);
      linesAreEqual = (validationLine == outputLine);
      if(debug){std::cout << (linesAreEqual?"\n~~~\n":"")
                          << "O: " << outputLine << "\n"
                          << "E: " << validationLine
                          << (linesAreEqual?"\n~~~\n":"\n");}
      EXPECT_EQ(true, linesAreEqual);
      if(!linesAreEqual){failureCounter++;}
      if(failureCounter == breakCount){break;}
    }
  };


mem::MMU* memory; // = new mem::MMU(128);

TEST(ProcessOutput, trace1){
  delete memory;
  memory = nullptr;
  memory = new mem::MMU(128);
  MemoryAllocator* allocator = new MemoryAllocator(128, memory);
  std::istringstream ss;
  ss = processOutputGetter("./trace1v.txt", memory, allocator);
  validateOutput(ss.str(), getExpectedOutput("./trace1v.txt.out"),false);
  delete memory;
  memory = nullptr;
}

TEST(ProcessOutput, trace2){
  delete memory;
  memory = nullptr;
  memory = new mem::MMU(128);
  MemoryAllocator* allocator = new MemoryAllocator(128, memory);
  std::istringstream ss;
  ss = processOutputGetter("./trace2v_multi-page.txt", memory, allocator);
  validateOutput(ss.str(), getExpectedOutput("./trace2v_multi-page.txt.out"),false);
  delete memory;
  memory = nullptr;
}

TEST(ProcessOutput, trace3){
  delete memory;
  memory = nullptr;
  memory = new mem::MMU(128);
  MemoryAllocator* allocator = new MemoryAllocator(128, memory);
  std::istringstream ss;
  ss = processOutputGetter("./trace3v_edge-addr.txt", memory, allocator);
  validateOutput(ss.str(), getExpectedOutput("./trace3v_edge-addr.txt.out"),false);
  delete memory;
  memory = nullptr;
}

TEST(ProcessOutput, trace4){
  delete memory;
  memory = nullptr;
  memory = new mem::MMU(128);
  MemoryAllocator* allocator = new MemoryAllocator(128, memory);
  std::istringstream ss;
  ss = processOutputGetter("./trace4v_wprotect.txt", memory, allocator);
  validateOutput(ss.str(), getExpectedOutput("./trace4v_wprotect.txt.out"),true);
  delete memory;
  memory = nullptr;
}

TEST(ProcessOutput, trace5){
  delete memory;
  memory = nullptr;
  memory = new mem::MMU(128);

  // Check that present bit is observed
  std::shared_ptr<PageFaultHandler>
    pf_handler(std::make_shared<PageFaultHandler>());  // define a page fault handler
  memory->SetPageFaultHandler(pf_handler);
    
  // Set up write fault handler
  std::shared_ptr<WritePermissionFaultHandler>
    wpf_handler(std::make_shared<WritePermissionFaultHandler>());
  memory->SetWritePermissionFaultHandler(wpf_handler);

  MemoryAllocator* allocator = new MemoryAllocator(128, memory);
  std::istringstream ss;
  ss = processOutputGetter("./trace5v_pagefaults.txt", memory, allocator);
  validateOutput(ss.str(), getExpectedOutput("./trace5v_pagefaults.txt.out"),true);
  delete memory;
  memory = nullptr;
}

TEST(MemAllocator, Allocate){

  delete memory;
  memory = nullptr;
  memory = new mem::MMU(128);

  // initialize an allocator that points to the MMU physical mem instance
  MemoryAllocator* allocator = new MemoryAllocator(128, memory);

  // Allocate a single page frame
  std::vector<uint32_t> pageFrames;
  allocator->AllocatePageFrames(1,pageFrames);
  std::cout << pageFrames[0]/0x4000 << "\n";

  uint32_t pt_entry;
  memory->movb(&pt_entry,
               mem::kPageTableSizeBytes+(sizeof(uint32_t)*3), sizeof(uint32_t));
  std::cout << std::hex << pt_entry << "\n";

  // returns the base of the user page table
  mem::Addr allocated = allocator->Alloc(0x2340000,1,false);

  // newly allocated virtual address, as guided by the UPT,
  // should point to an empty page (0 initialized)
  memory->movb(&pt_entry, 0x2340000, sizeof(uint32_t));
  EXPECT_EQ(pt_entry,0);


  delete memory;
  delete allocator;
}

int main(int argc, char* argv[]){
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
