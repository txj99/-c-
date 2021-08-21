#include "pch.h"
#include <iostream>
#include "MemPool.h"
using namespace std;
static tMemPoolCfg LrMemPoolCfg;

typedef struct A
{
    int a;
    int b;
}tA;

int main()
{
    unsigned int PooId = 0;
    unsigned int node = 10;
    unsigned int BlockSize = 0;
    unsigned int UnitsCount = 0;
    unsigned int FreeUnitsCount = 0;
    tA *obj1 = NULL;
    tA *obj2 = NULL;
    tA *obj3 = NULL;
    tA *obj4 = NULL;
    tA *obj5 = NULL;

    /* 内存池全局配置初始化 */
    LrMemPoolCfg.MaxMemPools = 1500;   /*内存池数量--初始化为1500个*/
    if (MemInitMemPool(&LrMemPoolCfg) != MEM_SUCCESS)
    {
        printf("MemInitMemPool Fails !!!!!!!!!!!!!!!!!!!!!!");
        return 1;
    }

    /*创建一个内存池，这里最多可以创建1500个*/
    MemCreateMemPool(sizeof(tA), node, &PooId);

    /*根据PooId分配一小块内存*/
    MemAllocateMemBlock(PooId, (unsigned char**)&obj1);
    MemAllocateMemBlock(PooId, (unsigned char**)&obj2);
    MemAllocateMemBlock(PooId, (unsigned char**)&obj3);
    MemAllocateMemBlock(PooId, (unsigned char**)&obj4);
    MemAllocateMemBlock(PooId, (unsigned char**)&obj5);

    /*查看内存池里的信息*/
    BlockSize      = GetBlockSize(PooId);
    UnitsCount     = GetUnitsCount(PooId);
    FreeUnitsCount = GetFreeUnitsCount(PooId);

    /*释放内存块*/
    MemReleaseMemBlock(PooId, (unsigned char*)&obj4);
    return 0;
}

/*
note:









*/
