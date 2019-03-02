/*
 * Process implementation
 *
 * File:   Process.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 * COMP3361 Winter 2019 Lab 4 Sample Solution
 */

#include "Process.h"
#include "MemoryAllocator.h"

#include <MMU.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

using mem::Addr;

using std::cin;
using std::cout;
using std::cerr;
using std::getline;
using std::istringstream;
using std::string;
using std::vector;

Process::Process(string file_name_, mem::MMU* memoryPtr, MemoryAllocator* allocatorPtr)
  : file_name(file_name_), line_number(0),
    memory(memoryPtr), allocator(allocatorPtr),
    hasPageTable(false){
  // Open the trace file.  Abort program if can't open.
  trace.open(file_name, std::ios_base::in);
  if (!trace.is_open()) {
    cerr << "ERROR: failed to open trace file: " << file_name << "\n";
    exit(2);
  }
}

Process::~Process() {
  trace.close();
}

void Process::Exec(void) {
  // Read and process commands
  string line;                // text line read
  string cmd;                 // command from line
  vector<uint32_t> cmdArgs;   // arguments from line
  int lineNumber = 0;

  // Select the command to execute
  while (ParseCommand(line, cmd, cmdArgs)) {
    if(cmd == "alloc"){
      CmdAlloc(line, cmd, cmdArgs);
      (debug? outStream: std::cout) << "\n";
    } else if (cmd == "cmp") {
      (debug? outStream: std::cout) << "\n";
      CmdCmp(line, cmd, cmdArgs);        // get and compare multiple bytes
    } else if (cmd == "set") {
      (debug? outStream: std::cout) << "\n";
      CmdSet(line, cmd, cmdArgs);        // put bytes
    } else if (cmd == "fill") {
      (debug? outStream: std::cout) << "\n";
      CmdFill(line, cmd, cmdArgs);       // fill bytes with value
    } else if (cmd == "dup") {
      (debug? outStream: std::cout) << "\n";
      CmdDup(line, cmd, cmdArgs);        // duplicate bytes to dest from source
    } else if (cmd == "print") {
      (debug? outStream: std::cout) << "\n";
      CmdPrint(line, cmd, cmdArgs);      // dump byte values to output
    } else if (cmd == "perm"){
      (debug? outStream: std::cout) << "\n";
      CmdPerm(line, cmd, cmdArgs);
    } else if (cmd == "*"){
      (debug? outStream: std::cout) << "\n";
    } else if (cmd != "*") {
      cerr << "ERROR: invalid command\n";
      exit(2);
    }

    // newline, so TODO messages get put on their relevant command lines
    //(debug? outStream: std::cout) << "\n";
  }
}

std::string Process::getStream(){
  return outStream.str();
}

void Process::setDebug(){
  debug=true;
}

bool Process::ParseCommand(
    string &line, string &cmd, vector<uint32_t> &cmdArgs) {
  cmdArgs.clear();
  line.clear();

  // Read next line
  if (std::getline(trace, line)) {
    ++line_number;
    (debug? outStream: std::cout) << std::dec << line_number << ":" << line;

    // No further processing if comment or empty line
    if (line.size() == 0 || line[0] == '*') {
      cmd = "*";
      return true;
    }

    // Make a string stream from command line
    istringstream lineStream(line);

    // Get address
    uint32_t addr = 0;
    if (!(lineStream >> std::hex >> addr)) {
      if (lineStream.eof()) {
        // Blank line, treat as comment
        cmd = "*";
        return true;
      } else {
        cerr << "ERROR: badly formed address in trace file: "
                << file_name << " at line " << line_number << "\n";
        exit(2);
      }
    }
    cmdArgs.push_back(addr);

    // Get command
    if (!(lineStream >> cmd)) {
      cerr << "ERROR: no command name following address in trace file: "
              << file_name << " at line " << line_number << "\n";
      exit(2);
    }

    // Get any additional arguments
    Addr arg;
    while (lineStream >> std::hex >> arg) {
      cmdArgs.push_back(arg);
    }
    return true;
  } else if (trace.eof()) {
      return false;
  } else {
    cerr << "ERROR: getline failed on trace file: " << file_name
            << " at line " << line_number << "\n";
    exit(2);
  }
}

void Process::CmdAlloc(const std::string &line,
                       const std::string &cmd,
                       const std::vector<uint32_t> &cmdArgs){
  // Request n pages starting from vaddr from the allocator
  // Vaddr must be a multiple of 0x4000
  // pages allocated must be writable and initialized to 0
  allocator->Alloc((mem::Addr) cmdArgs.at(0), (int) cmdArgs.at(1), hasPageTable);
  if(!hasPageTable){hasPageTable = true;}
}

void Process::CmdCmp(const string &line,
                     const string &cmd,
                     const vector<uint32_t> &cmdArgs) {
  if (cmdArgs.size() != 3) {
    cerr << "ERROR: badly formatted cmp command\n";
    exit(2);
  }
  Addr addr1 = cmdArgs.at(0);
  Addr addr2 = cmdArgs.at(1);
  uint32_t count = cmdArgs.at(2);

  // Compare specified byte values
  for (uint32_t i = 0; i < count; ++i) {
    Addr a1 = addr1 + i;
    uint8_t v1 = 0;
    memory->movb(&v1, a1);
    Addr a2 = addr2 + i;
    uint8_t v2 = 0;
    memory->movb(&v2, a2);
    if(v1 != v2) {
      (debug? outStream: std::cout) << std::setfill('0') << std::hex
              << "cmp error"
              << ", addr1 = "  << std::setw(7) << a1
              << ", value = " << std::setw(2) << static_cast<uint32_t>(v1)
              << ", addr2 = "  << std::setw(7) << a2
              << ", value = " << std::setw(2) << static_cast<uint32_t>(v2) << "\n";
    }
  }
}

void Process::CmdSet(const string &line,
                     const string &cmd,
                     const vector<uint32_t> &cmdArgs) {
  // Store multiple bytes starting at specified address
  Addr addr = cmdArgs.at(0);
  for (int i = 1; i < cmdArgs.size(); ++i) {
    uint8_t b = cmdArgs.at(i);
    memory->movb(addr++, &b);
  }
}

void Process::CmdDup(const string &line,
                     const string &cmd,
                     const vector<uint32_t> &cmdArgs) {
  if (cmdArgs.size() != 3) {
    cerr << "ERROR: badly formatted dup command\n";
    exit(2);
  }

  // Copy specified number of bytes to destination from source
  Addr dst = cmdArgs.at(1);
  Addr src = cmdArgs.at(0);
  uint32_t count = cmdArgs.at(2);

  // Buffer for copy (copy a block at a time for efficiency)
  uint8_t buffer[1024];
  while (count > 0) {
    uint32_t block_size = std::min((unsigned long) count, sizeof(buffer));
    memory->movb(buffer, src, block_size);
    src += block_size;
    memory->movb(dst, buffer, block_size);
    dst += block_size;
    count -= block_size;
  }
}

void Process::CmdFill(const string &line,
                     const string &cmd,
                     const vector<uint32_t> &cmdArgs) {
  // Fill destination range with specified value
  uint8_t value = cmdArgs.at(1);
  uint32_t count = cmdArgs.at(2);
  Addr addr = cmdArgs.at(0);

  // Use buffer for efficiency
  uint8_t buffer[1024];
  memset(buffer, value, std::min((unsigned long) count, sizeof(buffer)));

  // Write data to memory
  while (count > 0) {
    uint32_t block_size = std::min((unsigned long) count, sizeof(buffer));
    memory->movb(addr, buffer, block_size);
    addr += block_size;
    count -= block_size;
  }
}

void Process::CmdPrint(const string &line,
                     const string &cmd,
                     const vector<uint32_t> &cmdArgs) {
  Addr addr = cmdArgs.at(0);
  uint32_t count = cmdArgs.at(1);

  // Output the specified number of bytes starting at the address
  for (int i = 0; i < count; ++i) {
    if ((i % 16) == 0) { // Write new line with address every 16 bytes
      if (i > 0) (debug? outStream: std::cout) << "\n";  // not before first line
      (debug? outStream: std::cout) << std::hex << std::setw(7) << std::setfill('0') << addr << ":";
    }
    uint8_t b;
    memory->movb(&b, addr++);
    (debug? outStream: std::cout) << " " << std::setfill('0') << std::setw(2) << static_cast<uint32_t> (b);
  }
  (debug? outStream : std::cout) << "\n";
}

void Process::CmdPerm(const std::string &line,
             const std::string &cmd,
             const std::vector<uint32_t> &cmdArgs){

  //Goss' code
  // Make page present and writable, write, and check modified bit
  // page_table[pt_offset] = kPhysStart | kPTE_PresentMask | kPTE_WritableMask;
  // vm.movb(kPageTableBase, &page_table, kPageTableSizeBytes);


  // Save the user PMCB so we can go back
  mem::PMCB user;
  memory->get_user_PMCB(user);
  memory->set_kernel_PMCB();

  uint32_t PTE_buffer;

  // Loop over the number of pages to make writable or not
  for (int i = 0; i < cmdArgs[1]; i++){
    mem::Addr pt_offset =
      user.page_table_base
      +((cmdArgs[0]/0x4000) * sizeof(uint32_t))
      +(sizeof(uint32_t)*i);

    // get the UPTE so we can apply the writable bit mask
    memory->movb(&PTE_buffer, pt_offset, sizeof(uint32_t));
    PTE_buffer = ((bool) cmdArgs[2]? (PTE_buffer | 0x1) : ((PTE_buffer >> 1) << 1));
    // set the UPTE back in the PTE
    memory->movb(pt_offset, &PTE_buffer, sizeof(uint32_t));
  }

  // go back to user level
  memory->set_user_PMCB(user);
}
