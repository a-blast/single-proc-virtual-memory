
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

  // returns the base of the user page table
  mem::Addr allocated = allocator->Alloc(0x2340000,1,false);

  // newly allocated virtual address, as guided by the UPT,
  // should point to an empty page (0 initialized)
  memory->movb(&pt_entry, 0x2340000, sizeof(uint32_t));
  EXPECT_EQ(pt_entry,0);


  delete memory;
  delete allocator;
}

// TEST(MemAllocator, initKPT){
//   mem::MMU* memory2 = new mem::MMU(128);
//   MemoryAllocator* allocator = new MemoryAllocator();
//   allocator->memory = memory2;

//   allocator->init_KPT();

//   uint32_t kernel_pt_entry;
//   for(int i = 0; i < 0x7d; ++i){
//     memory2->movb(&kernel_pt_entry, mem::kPageSize + (i*4), sizeof(uint32_t));
//     //std::cout << std::hex << kernel_pt_entry << " : " << i <<"\n";
//   }

//   delete memory2;
//   delete allocator;

// }

int main(int argc, char* argv[]){
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
