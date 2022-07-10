#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.

  /* three input argument */
  //1, start virtual address to check
  //2, number page to check 
  //3, a user address to a buffer to store the results into a bitmask

  /* find the PTE corresponds to the virtual address va,  */
  uint64 va=0;
  int num=0;
  uint64 buffer=0;
  uint64 bitmap = 0;
  pte_t* pte;
  
  if (argaddr(0, &va) < 0 ||
      argint(1, &num)        < 0 ||
      argaddr(2, &buffer)    < 0) {
    printf("intput arg error!\n");
    return -1;
  }
  
  /* check condition */
  /* upper limit on the number of pages that can be scanned, we set 64 */
  if (va + num >= MAXVA || 
      num > 64) {
    printf("intput arg error!\n");
    return -1;
  }
  
  for (int i = 0; i < num; i++) {
    pte = walk(myproc()->pagetable, va, 0);
    if (*pte & PTE_A) {
      bitmap |= 1 << i;
      *pte &= ~PTE_A;
    }
    va += PGSIZE;
  }

  if (copyout(myproc()->pagetable, buffer, (char *)&bitmap, num>>3) < 0) {
    printf("copy out error occur!\n");
    return -1;
  }

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}