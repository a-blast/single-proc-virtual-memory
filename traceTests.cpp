
#include "gtest/gtest.h"
#include "Process.h"
# include <algorithm>
# include <string>
# include <iostream>

# TODO need to redirect process output to a logging class so it can be
# validated in this test.

auto processOutputGetter =
  [](std::string filePath){
    Process proc(filePath);
    return; // std::istringstream(proc.Exec());
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
  [](std::istringstream outputStream, std::ifstream validationStream,
     bool debug=false, int breakCount = 5){
    std::string outputLine;
    std::string validationLine;
    bool linesAreEqual;
    int failureCounter = 0;
    while(getline(outputStream, outputLine)){
      getline(validationStream, validationLine);
      if(debug){std::cout << "O: " << outputLine << "\n"
                          << "E: " << validationLine << "\n~~~\n";}
      linesAreEqual = (validationLine == outputLine);
      EXPECT_EQ(true, linesAreEqual);
      if(!linesAreEqual){failureCounter++;}
      if(failureCounter == breakCount){break;}
    }
  };

TEST(ProcessOutput, trace1){
    EXPECT_EQ(true,true);
}

int main(int argc, char* argv[]){
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
