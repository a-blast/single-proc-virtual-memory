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

void setPageTable(mem::MMU &vm, mem::Addr pt_addr){
  mem::PageTable kernel_page_table;  // local copy of page table to build, initialized to 0
  mem::Addr num_pages = vm.get_frame_count();  // size of physical memory
  // Build page table entries
  for (mem::Addr i = 0; i < num_pages; ++i) {
    kernel_page_table.at(i) = 
      (i << mem::kPageSizeBits);
  }
  // Write page table to memory
  vm.movb(pt_addr, &kernel_page_table, mem::kPageTableSizeBytes);
}

int main(int argc, char** argv) {

  if (argc != 2) {
    std::cerr << "usage: Lab4 trace_file\n";
    exit(1);
  }

  // 1. Create a instance of MMU w/ 128 (0x80) page frames
  mem::MMU* memory = new mem::MMU(128);

  // initialize an allocator that points to the MMU physical mem instance
  MemoryAllocator* allocator = new MemoryAllocator(128, memory);

  // Set kernel page table
  setPageTable(*memory, mem::kPageSize);
  mem::PMCB kernel_pmcb(mem::kPageSize);
  memory->enter_virtual_mode(kernel_pmcb);


  delete memory;
  delete allocator;

  memory = NULL;
  allocator = NULL;
  return 0;
}

