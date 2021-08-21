#ifndef _MEMPOOL_H
#define _MEMPOOL_H

#include "pch.h"
/********************结构体定义**************************/
typedef struct MemPoolCfg
{
    unsigned int MaxMemPools;      /*系统当中最大的内存池个数--目前为1500*/
}tMemPoolCfg;

/*定义链表结构-用来将内存池当中的所有内存块链接起来*/
typedef struct CRU_SLL_NODE {
    unsigned char  *pNext;
}tCRU_SLL_NODE;

typedef struct CRU_SLL {
    unsigned char  *pu1Base;
    unsigned char  *pu1Head;
} tCRU_SLL;
/*以下结构体用来记录内存池的一些具体信息*/
typedef struct MEM_FREE_POOL_REC {
    unsigned int    Size;
    unsigned int    UnitsCount;
    unsigned int    FreeUnitsCount;
    unsigned char  *pu1ActualBase;
    unsigned char  *pu1StartAddr;
    unsigned char  *pu1EndAddr;
    tCRU_SLL  BufList;
}tMemFreePoolRec;

/***********************************一些宏定义***************************/
#define  POOL_MEM_CALLOC(s,c,t)            (t *)calloc(s, c) 
#define  BUDDY_MEM_FREE(p)                 free(p)
#define  POOL_MEM_FREE(p)                  free(p)
#define  MEM_SUCCESS                       0
#define  MEM_OK_BUT_NOT_ALIGNED            1
#define  MEM_FAILURE                       (unsigned int) (~0UL)
#define  MEM_ALIGN_BYTE                    (sizeof(unsigned long) - 1)
#define  MEM_ALIGN                         ((~0UL) & ~(unsigned long)(sizeof(unsigned long) - 1))



/**************************函数声明*******************************/
unsigned int MemInitMemPool(tMemPoolCfg *pMemPoolCfg);
unsigned int MemCreateMemPool(unsigned int BlockSize, unsigned int NumberOfBlocks, unsigned int *pPoolId);
unsigned int MemAllocateMemBlock(unsigned int PoolId, unsigned char **ppu1Block);
unsigned int MemReleaseMemBlock(unsigned int PoolId, unsigned char *pu1Block);
unsigned int MemDeleteMemPool(unsigned int PoolId);
unsigned int GetBlockSize(unsigned int PoolId);
unsigned int GetUnitsCount(unsigned int PoolId);
unsigned int GetFreeUnitsCount(unsigned int PoolId);
#endif

