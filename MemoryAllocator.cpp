/* MemoryAllocator - allocate pages in memory
 * 
 * COMP3361 Fall 2018 - Lab 3 Sample Solution
 * 
 * File:   MemoryAllocator.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 */

#include "MemoryAllocator.h"

#include <MMU.h>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>

MemoryAllocator::MemoryAllocator(uint32_t page_frame_count, mem::MMU* memoryPtr ) 
: memory(memoryPtr)
{
  if (page_frame_count <= 1) {
    throw std::runtime_error("page_frame_count must be > 1");
  }

  // Create the kernel page table
  mem::PageTable kernel_page_table;  // local copy of page table to build, initialized to 0
  mem::Addr num_pages = memory->get_frame_count();  // size of physical memory
  // Build page table entries
  uint32_t pageAddr;

  for (mem::Addr i = 0; i < num_pages; ++i) {
    //std::cout << ((i << mem::kPageSizeBits) | 0x3) << " ";
    pageAddr = (i << mem::kPageSizeBits);
    kernel_page_table.at(i) = 
      (pageAddr | 0x3);
    freeList.push_back(pageAddr);
  }

  std::cout << freeList[0] << "," << std::hex << freeList[1] << "\n";
  // Write page table to memory
  memory->movb(mem::kPageSize, &kernel_page_table, mem::kPageTableSizeBytes);

  // Enter kernel Virtual Mode
  mem::PMCB kernel_pmcb(mem::kPageSize);
  memory->enter_virtual_mode(kernel_pmcb);

  // remove first two frames in free memory
  freeList.erase(freeList.begin(),freeList.begin()+1);

  // Initialize list info in page 0
  // #Austin: not sure if we still need this with the stack method
  // set_free_list_head(mem::kPageSize*2);
  // set_page_frames_free(page_frame_count - 2);
  // set_page_frames_total(page_frame_count);

}

bool MemoryAllocator::Alloc(mem::Addr address, int numFrames, bool hasPageTable){

  if(!hasPageTable){
    // 1 page for the PT
    std::vector<uint32_t> procPageAddr;
    AllocatePageFrames(1,procPageAddr);
    // convert to virtual address
    uint32_t procPageVirtAddr = (procPageAddr[0]/0x4000) << mem::kPageSizeBits;
    mem::PageTable procPageTable;
    memory->movb(procPageVirtAddr, &procPageTable, mem::kPageTableSizeBytes);
    mem::PMCB user_pmcb(procPageVirtAddr);
    memory->set_user_PMCB(user_pmcb);
  }

  std::vector<uint32_t> allocatedFrameAddr;
  AllocatePageFrames(numFrames, allocatedFrameAddr);
  std::vector<uint32_t> allocatedFrameVirtAddr;
  allocatedFrameVirtAddr.resize(allocatedFrameAddr.size());

  std::transform(allocatedFrameAddr.begin(),allocatedFrameAddr.end(),
                 allocatedFrameVirtAddr.begin(),
                 [](uint32_t addr)
                 {return (((addr/0x4000) << 14) | 0x3);});

  mem::PMCB user;
  memory->get_user_PMCB(user);
  memory->set_kernel_PMCB();

  memory->movb(user.page_table_base,
               &allocatedFrameVirtAddr,
               sizeof(uint32_t)*allocatedFrameVirtAddr.size());

  memory->set_user_PMCB(user);

  return true;

}

bool MemoryAllocator::AllocatePageFrames(uint32_t count, 
                                         std::vector<uint32_t> &page_frames) {
  mem::Addr freeFrame;
  if(count <= freeList.size()){
    for(int frameNum = 0; frameNum < count; frameNum++){
      freeFrame = freeList[freeList.size()-1];
      freeList.pop_back();
      page_frames.push_back(freeFrame);
    }
    return true;
  }else{
    return false;
  }
}


bool MemoryAllocator::FreePageFrames(uint32_t count,
                                     std::vector<uint32_t> &page_frames) {
  // Fetch free list info
  uint32_t page_frames_free = get_page_frames_free();
  uint32_t free_list_head = get_free_list_head();

  // If enough to deallocate
  if(count <= page_frames.size()) {
    while(count-- > 0) {
      // Return next frame to head of free list
      uint32_t frame = page_frames.back();
      page_frames.pop_back();
      memcpy(&memory[frame], &free_list_head, sizeof(uint32_t));
      free_list_head = frame;
      ++page_frames_free;
    }

    // Update free list info
    set_free_list_head(free_list_head);
    set_page_frames_free(page_frames_free);

    return true;
  } else {
    return false; // do nothing and return error
  }
}

std::string MemoryAllocator::FreeListToString(void) const {
  std::ostringstream out_string;
  
  uint32_t next_free = get_free_list_head();
  
  while (next_free != kEndList) {
    out_string << " " << std::hex << next_free;
    memcpy(&next_free, &memory[next_free], sizeof(uint32_t));
  }
  
  return out_string.str();
}

uint32_t MemoryAllocator::get_page_frames_free() const {
  uint32_t page_frames_free[1];

  std::cout << sizeof(uint32_t) << "in get\n";

  memory->movb(&page_frames_free,(mem::Addr) kPageFramesFree,(mem::Addr) sizeof(uint32_t));
  std::cout << "in get\n";
  return page_frames_free[0];
}

uint32_t MemoryAllocator::get_page_frames_total() const {
  uint32_t page_frames_total;
  memory->movb(&page_frames_total, kPageFramesTotal, sizeof(uint32_t));
  return page_frames_total;
}

uint32_t MemoryAllocator::get_free_list_head() const {
  uint32_t free_list_head;
  memory->movb(&free_list_head,(mem::Addr) kFreeListHead,(mem::Addr) sizeof(uint32_t));
  std::cout << free_list_head <<"\n";
  return free_list_head;
}

void MemoryAllocator::set_page_frames_free(uint32_t page_frames_free) {
  memory->movb(kPageFramesFree, &page_frames_free, sizeof(uint32_t));
}

void MemoryAllocator::set_page_frames_total(uint32_t page_frames_total) {
  memory->movb(kPageFramesTotal, &page_frames_total, sizeof(uint32_t));
}

void MemoryAllocator::set_free_list_head(uint32_t free_list_head) {
  memory->movb(kFreeListHead, &free_list_head, sizeof(uint32_t));
}
