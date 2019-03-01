
#include "gtest/gtest.h"
#include "Process.h"
# include "MemoryAllocator.h"
# include <MMU.h>
# include <algorithm>
# include <string>
# include <iostream>

// TODO need to redirect process output to a logging class so it can be
// validated in this test.

auto processOutputGetter =
  [](std::string filePath){
    Process proc(filePath);
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
     bool debug=true, int breakCount = 5){
    std::string outputLine;
    std::string validationLine;
    std::istringstream outputStream(output);

    // getline(validationStream, validationLine);
    // std::cout << validationLine << "\n";

    // getline(outputStream, outputLine);
    // std::cout << outputLine << "\n";

    // getline(outputStream, outputLine);
    // std::cout << outputLine << "\n";
    bool linesAreEqual;
    int failureCounter = 0;
    while(getline(outputStream, outputLine)){
      getline(validationStream, validationLine);
      linesAreEqual = (validationLine == outputLine);
      if(debug){std::cout << (linesAreEqual?"\n~~~\n":"")
                          << "O: " << outputLine << "\n"
                          << "E: " << validationLine
                          << (linesAreEqual?"\n~~~\n":"");}
      EXPECT_EQ(true, linesAreEqual);
      if(!linesAreEqual){failureCounter++;}
      if(failureCounter == breakCount){break;}
    }
  };

void setPageTable_TEMP_TEST(mem::MMU &vm, mem::Addr pt_addr){
  mem::PageTable kernel_page_table;  // local copy of page table to build, initialized to 0
  mem::Addr num_pages = vm.get_frame_count();  // size of physical memory
  // Build page table entries


  for (mem::Addr i = 0; i < num_pages; ++i) {
    std::cout << ((i << mem::kPageSizeBits) | 0x3) << " ";
    kernel_page_table.at(i) = 
      ((i << mem::kPageSizeBits) | 0x3);
  }

  // Write page table to memory
  vm.movb(pt_addr, &kernel_page_table, mem::kPageTableSizeBytes);
}

TEST(ProcessOutput, trace1){
  std::istringstream ss;
  ss = processOutputGetter("./trace1v.txt");
  validateOutput(ss.str(), getExpectedOutput("./trace1v.txt.out"));
}
TEST(ProcessOutput, trace2){
  std::istringstream ss;
  ss = processOutputGetter("./trace2v_multi-page.txt");
  validateOutput(ss.str(), getExpectedOutput("./trace2v_multi-page.txt.out"));
}
TEST(ProcessOutput, trace3){
  std::istringstream ss;
  ss = processOutputGetter("./trace3v_edge-addr.txt");
  validateOutput(ss.str(), getExpectedOutput("./trace3v_edge-addr.txt.out"));
}
TEST(ProcessOutput, trace4){
  std::istringstream ss;
  ss = processOutputGetter("./trace4v_wprotect.txt");
  validateOutput(ss.str(), getExpectedOutput("./trace4v_wprotect.txt.out"));
}
TEST(ProcessOutput, trace5){
  std::istringstream ss;
  ss = processOutputGetter("./trace5v_pagefaults.txt");
  validateOutput(ss.str(), getExpectedOutput("./trace5v_pagefaults.txt.out"));
}

TEST(MemAllocator, Allocate){
  mem::MMU* memory = new mem::MMU(128);

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


  allocator->Alloc(0x2340000,1,false);



}

int main(int argc, char* argv[]){
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
