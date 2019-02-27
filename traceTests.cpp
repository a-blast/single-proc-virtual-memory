
#include "gtest/gtest.h"
#include "Process.h"
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

int main(int argc, char* argv[]){
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
