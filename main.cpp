/* 
 * File:   main.cpp
 * Author: aarmstrong
 *
 * Created on February 24, 2019, 10:51 AM
 */

#include "MemoryAllocator.h"

#include <MMU.h>
#include <cstdlib>
#include <iostream>
#include <memory>

int main(int argc, char** argv) {
  
  if (argc != 2) {
    std::cerr << "usage: Program2 trace_file\n";
    exit(1);
  }
  
  // 1. Create a instance of MMU w/ 128 (0x80) page frames
  mem::MMU *memory = new mem::MMU(128);
  
  // initialize an allocator that points to the MMU physical mem instance
  MemoryAllocator *allocator = new MemoryAllocator((uint32_t)128, memory);
  
  delete memory;
  delete allocator;

  memory = NULL;
  allocator = NULL;
  return 0;
}


