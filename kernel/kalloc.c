// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

/*
  initialize the allocator
  initializes the free list to hold every page between the end of the kernel and PHYSTOP.
*/
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

// add memory to the free list via per-page calls to kfree
void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  // uses PGROUNDUP to ensure that it frees only aligned physical addresses
  p = (char*)PGROUNDUP((uint64)pa_start);

  // The allocator starts with no memory; these calls to kfree give it some to manage.
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  
  // 检查传入的地址 pa 是否合法
  // 如果 pa 不是 PGSIZE（4096）的倍数，或者 pa 小于 end（内核结束位置），或者 pa 大于等于 PHYSTOP（物理内存结束位置）// every page between the end of the kernel and PHYSTOP
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  // begins by setting every byte in the memory being freed to the value 1
  // This will cause code that uses memory after freeing it (uses “dangling references”) to read garbage instead of the old valid contents
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // Then kfree prepends the page to the free list
  // 将释放的页面添加到空闲链表的头部
  acquire(&kmem.lock);
  r->next = kmem.freelist;  // 将释放的页面连接到空闲链表
  kmem.freelist = r;   // 将空闲链表的头部更新为释放的页面
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
