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


  initKernalPageTable();

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

MemoryAllocator::MemoryAllocator(){};

void MemoryAllocator::initKernalPageTable(void){
  
  // Create the kernel page table
  mem::PageTable kernel_page_table;  // local copy of page table to build, initialized to 0
  mem::Addr num_pages = memory->get_frame_count();  // size of physical memory
  // Build page table entries
  uint32_t pageAddr;

  for (mem::Addr i = 0; i < num_pages; ++i) {
    // std::cout << std::hex << ((i << mem::kPageSizeBits) | 0x3) << " : " << i << "\n";
    pageAddr = (i << mem::kPageSizeBits);
    kernel_page_table.at(i) = 
      (pageAddr | 0x3);
    // std::cout << std::hex << kernel_page_table.at(i) << " ?\n";
    freeList.push_back(pageAddr);

  }

  // Write page table to memory in frame 2
  memory->movb(mem::kPageSize, &kernel_page_table, mem::kPageTableSizeBytes);
}

void MemoryAllocator::initUserPageTable(void){
  // allocate 1 page for the UPT (user page table)
  std::vector<uint32_t> procPageAddr;
  AllocatePageFrames(1,procPageAddr);

  // convert physical addr of alocated page to virtual address valid with the
  // KPT
  uint32_t procPageVirtAddr = (procPageAddr[0]/0x4000) << mem::kPageSizeBits;
  //std::cout << "UPT BASE: " << std::hex << (procPageVirtAddr) << "\n";

  // Store the empty UPT in memory
  mem::PageTable procPageTable;
  memory->movb(procPageVirtAddr, &procPageTable, mem::kPageTableSizeBytes);

  // NOTE: Test code, keeping here for now. Basically ensures the
  // UPT has been put in memory propery and the the first entry is 0.

  // // Checking the uPT entry (UPTE)... (should be 0)
  // uint32_t pt_entry;
  // std::cout << "KPT ADDR: " << std::hex << (procPageVirtAddr >> 14) << "\n";
  // memory->movb(&pt_entry, procPageVirtAddr , sizeof(uint32_t));
  // std::cout << "UPTE: " <<  std::hex << pt_entry << "\n";

  // Set to user mode
  mem::PMCB user_pmcb(procPageVirtAddr);
  memory->set_user_PMCB(user_pmcb);
}

mem::Addr MemoryAllocator::Alloc(mem::Addr address, int numFrames, bool hasPageTable){

  if(!hasPageTable){
    initUserPageTable();
  }

  // allocate the requested # of page frames & get their physical address
  std::vector<uint32_t> allocatedFrameAddr;
  AllocatePageFrames(numFrames, allocatedFrameAddr);

  // Convert these addr to user page table entries (PTE)
  std::vector<uint32_t> allocatedFramePTE;
  allocatedFramePTE.resize(allocatedFrameAddr.size());

  std::transform(allocatedFrameAddr.begin(),allocatedFrameAddr.end(),
                 allocatedFramePTE.begin(),
                 [](uint32_t addr)
                 {return (((addr/0x4000) << 14) | 0x3);});

  // Save the user PMCB so we can go back
  mem::PMCB user;
  memory->get_user_PMCB(user);
  memory->set_kernel_PMCB();

  // 0 initialize the allocated (non PT) pages
  std::for_each(allocatedFrameAddr.begin(), allocatedFrameAddr.end(),
                [this](uint32_t addr){
                  int init[mem::kPageSize] = {0};
                  uint32_t pageNum = (addr/0x4000);
                  // std::cout << "vAddr: " << (pageNum << 4) << "\n";
                  this->memory->movb((pageNum << 14), &init, mem::kPageSize);
                });


  // NOTE: havent tested if this works for multiple page frames.
  // set the UPTE to point to the allocated page frame at the specified virtual address
  for(int i = 0; i < allocatedFramePTE.size(); i++){
    memory->movb((user.page_table_base                   // KPT index address that points to the UPT
                  +((address/0x4000) * sizeof(uint32_t)) // offset for specific UPTE
                  +(sizeof(uint32_t)*i)),                // ?offset for multiple page frames?
                 &allocatedFramePTE[i],
                 sizeof(uint32_t));
  }

  // NOTE: below is test code, need to clean up and put into the test script,
  // uint32_t pt_entry;
  // // memory->movb(&pt_entry, (127 << 14) + (address/0x4000), sizeof(uint32_t));
  // // std::cout << "TEST KPT: " << std::hex << (127 << 14) << " : " << pt_entry << "\n";
  // memory->movb(&pt_entry, user.page_table_base + ((address/0x4000) * sizeof(uint32_t)), sizeof(uint32_t));
  // std::cout << "TEST: " << std::hex << pt_entry << ":" << allocatedFramePTE[0] << "\n";

  // read out user level page table entry by entry
  // for(int pte = 0; pte < mem::kPageTableEntries; ++pte){
  //   memory->movb(&pt_entry, user.page_table_base + (pte * sizeof(uint32_t)), sizeof(uint32_t));
  //   std::cout << std::hex << pt_entry << " : " << pte << "\n";
  // }

  // std::cout << "START: " << std::hex << user.page_table_base
  //           << " : " << address
  //           << " : " << (address/0x4000)
  //           << " : " << user.page_table_base+(address/0x4000) 
  //           << " : " << ((user.page_table_base+(address/0x4000))>>14) << "\n";

  memory->set_user_PMCB(user);

  // std::cout << "address test: " << std::hex << (address >> 14) << " : " << (address/0x4000) << "\n"; 
  // memory->movb(&pt_entry, address, sizeof(uint32_t));
  // std::cout << "TEST UPT: " << std::hex << pt_entry << ":" << allocatedFramePTE[0] << "\n";

return user.page_table_base;

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
