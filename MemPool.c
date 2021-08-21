#include "pch.h"
#include <iostream>
#include "MemPool.h"

/***************************全局变量定义***************************/
tMemFreePoolRec              *pMemFreePoolRecList = NULL;
tMemPoolCfg                   gtMemPoolCfg;

/***********************一些静态函数声明********************************/
static unsigned int MemPoolInitializeFreePoolList(tMemPoolCfg *pMemPoolCfg);
static unsigned int MemGetFreePoolId(void);
static unsigned int MemIsValidBlock(unsigned int PoolId, unsigned char *pu1Block);
/***************************************************************
 * @file       MemPool.c
 * @brief      PoolList初始化
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
static unsigned int MemPoolInitializeFreePoolList(tMemPoolCfg *pMemPoolCfg)
{
    unsigned int      PoolId = 0;
    gtMemPoolCfg.MaxMemPools = pMemPoolCfg->MaxMemPools;

    /*分配一大块内存块,用来记录每一个PooId的具体信息*/
    pMemFreePoolRecList = POOL_MEM_CALLOC(sizeof(tMemFreePoolRec), (gtMemPoolCfg.MaxMemPools), tMemFreePoolRec);
    if (!pMemFreePoolRecList)
    {
        BUDDY_MEM_FREE(pMemFreePoolRecList);
        return MEM_FAILURE;
    }
    /*PooId数组初始化*/
    for (PoolId = 0; PoolId < gtMemPoolCfg.MaxMemPools; PoolId++)
    {
        pMemFreePoolRecList[PoolId].Size = 0;
        pMemFreePoolRecList[PoolId].UnitsCount = 0;
        pMemFreePoolRecList[PoolId].FreeUnitsCount = 0;

        pMemFreePoolRecList[PoolId].pu1StartAddr = NULL;
        pMemFreePoolRecList[PoolId].pu1EndAddr = NULL;

        pMemFreePoolRecList[PoolId].pu1ActualBase = NULL;

        pMemFreePoolRecList[PoolId].BufList.pu1Base = NULL;
        pMemFreePoolRecList[PoolId].BufList.pu1Head = NULL;
    }
    return MEM_SUCCESS;
}
/***************************************************************
 * @file       MemPool.c
 * @brief      Pool初始化
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
unsigned int MemInitMemPool(tMemPoolCfg *pMemPoolCfg)
{
    unsigned int     ret;
    /*给PooId数组里的每一个元素进行初始化*/
    if ((ret = MemPoolInitializeFreePoolList(pMemPoolCfg)) == MEM_FAILURE)
    {
        return MEM_FAILURE;
    }
    return MEM_SUCCESS;
}
/***************************************************************
 * @file       MemPool.c
 * @brief      从内存池当中分配一小块内存
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
static unsigned int MemGetFreePoolId(void)
{
    unsigned int    PoolId;
    for (PoolId = 0; PoolId < gtMemPoolCfg.MaxMemPools; PoolId++)
    {
        if (!pMemFreePoolRecList[PoolId].UnitsCount)
        {
            /*分配出去的PooId会被记录为1*/
            pMemFreePoolRecList[PoolId].UnitsCount = 1;
            return PoolId;
        }
    }
    return (unsigned int)(MEM_FAILURE);
}
/***************************************************************
 * @file       MemPool.c
 * @brief      创建一个Pool
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
unsigned int MemCreateMemPool(unsigned int BlockSize, unsigned int NumberOfBlocks, unsigned int *pPoolId)
{
    unsigned char     *pu1PoolBase;
    unsigned char     *pu1BufStart;
    unsigned int       CurrObj;
    int                PoolId;
    /*检查形参是否合法*/
    if ((BlockSize == 0) || (NumberOfBlocks == 0) || (pPoolId == NULL))
    {
        printf("形参不合法，创建一个内存池失败!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        return (MEM_FAILURE);
    }
    /*4字节对齐*/
    BlockSize = (BlockSize + MEM_ALIGN_BYTE) & MEM_ALIGN;
    /*从所有(1500)的内存池当中回去空闲的内存池--即PooId*/
    if ((PoolId = MemGetFreePoolId()) == (int)MEM_FAILURE)
    {
        printf("获取空闲的POOID失败\n");
        return MEM_FAILURE;
    }
    /*根据字节数和块数分配内存，这里分配多出来一块，主要是因为字节对齐*/
    if (!(pu1PoolBase = POOL_MEM_CALLOC(BlockSize, NumberOfBlocks + 1, unsigned char)))
    {
        printf("分配内存失败\n");
        return MEM_FAILURE;
    }
    /*记录实际分配到的内存块的起始地址*/
    pMemFreePoolRecList[PoolId].pu1ActualBase = pu1PoolBase;
    /*对分配到的内存块地址做地址偏移，如果分配到的内存块地址本身就对齐了，那么分配后的偏移地址不会变*/
    pu1PoolBase = (unsigned char *)(((unsigned long)pu1PoolBase + MEM_ALIGN_BYTE) & MEM_ALIGN);
    /*开始地址是偏移之后的*/
    pu1BufStart = pu1PoolBase;
    /*链表记录*/
    pMemFreePoolRecList[PoolId].BufList.pu1Base = pu1PoolBase;
    pMemFreePoolRecList[PoolId].BufList.pu1Head = pu1PoolBase;
    pMemFreePoolRecList[PoolId].Size = BlockSize;
    /*内存池当中的节点数*/
    pMemFreePoolRecList[PoolId].UnitsCount = NumberOfBlocks;
    /*内存池当中空余的节点数*/
    pMemFreePoolRecList[PoolId].FreeUnitsCount = NumberOfBlocks;
    /*内存池当中的起始地址(从偏移以后的开始)*/
    pMemFreePoolRecList[PoolId].pu1StartAddr = (unsigned char *)pu1BufStart;
    /*内存池当中的末尾地址*/
    pMemFreePoolRecList[PoolId].pu1EndAddr = (unsigned char *)(pu1BufStart + (NumberOfBlocks * BlockSize));

    /*将所有块以单链表的结构链起来*/
    for (CurrObj = 1; CurrObj < NumberOfBlocks; CurrObj++)
    {
        ((tCRU_SLL_NODE *)(void *)pu1PoolBase)->pNext = (pu1PoolBase + BlockSize);
        pu1PoolBase += BlockSize;
    }
    /*内存池当中最后一个块的next指向NULL*/
    ((tCRU_SLL_NODE *)(void *)pu1PoolBase)->pNext = NULL;
    /*最后返回PooId*/
    *pPoolId = PoolId;
    return MEM_SUCCESS;
}
/***************************************************************
 * @file       MemPool.c
 * @brief      从内存池当中分配一小块内存(根据PooId进行分配)
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
unsigned int MemAllocateMemBlock(unsigned int PoolId, unsigned char **ppu1Block)
{
    unsigned char              *pNode;
    tCRU_SLL                   *pPool;
    tMemFreePoolRec            *pPoolRecPtr;

    /*判断形参是否合法*/
    if ((PoolId < 0) || (PoolId >= gtMemPoolCfg.MaxMemPools) || (ppu1Block == NULL)
        || pMemFreePoolRecList[PoolId].UnitsCount == 0)
    {
        return MEM_FAILURE;
    }
    /*用一些临时变量指向对应的PooId*/
    pPoolRecPtr = &pMemFreePoolRecList[PoolId];
    pPool = &pMemFreePoolRecList[PoolId].BufList;
    pNode = pPool->pu1Head;
    if (pNode)
    {
        /*将第一块空闲的内存块分配给*ppu1Block*/
        *ppu1Block = pNode;
        /*PooId对应的空闲内存块个数减一*/
        pPoolRecPtr->FreeUnitsCount--;
        /*Head指向下一个空闲的内存块*/
        pPool->pu1Head = ((tCRU_SLL_NODE *)(void *)pNode)->pNext;
        /*最后再判断分配出去的内存块是否在有效的范围内*/
        if (MemIsValidBlock(PoolId, *ppu1Block) == MEM_FAILURE)
        {
            *ppu1Block = NULL;
            return MEM_FAILURE;
        }
        return MEM_SUCCESS;
    }
    else
    {
        return  MEM_FAILURE;
    }
}
/***************************************************************
 * @file       MemPool.c
 * @brief      从内存池当中分配一小块内存
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
unsigned int MemReleaseMemBlock(unsigned int PoolId, unsigned char *pu1Block)
{
    tMemFreePoolRec    *pPoolRecPtr;
    /*指向PooId数组的首地址*/
    pPoolRecPtr = &pMemFreePoolRecList[PoolId];
    /*判断PooId是否在有效范围内*/
    if ((PoolId < 0) || (PoolId >= gtMemPoolCfg.MaxMemPools))
    {
        return MEM_FAILURE;
    }
    /*判断该内存块是否属于该PoolId内*/
    if (MemIsValidBlock(PoolId, pu1Block) == MEM_FAILURE)
    {
        return MEM_FAILURE;
    }
    /*假释放只是将该内存块的数据清0*/
    memset(pu1Block, '\0', pPoolRecPtr->Size);

    /*然后改变指针的指向*/
    ((tCRU_SLL_NODE *)(void *)pu1Block)->pNext = pPoolRecPtr->BufList.pu1Head;
    pPoolRecPtr->BufList.pu1Head = pu1Block;

    /*空闲块的个数加一*/
    pPoolRecPtr->FreeUnitsCount++;

    return (MEM_SUCCESS);
}
/***************************************************************
 * @file       MemPool.c
 * @brief      判断这一小块内存是否属于内存池里的内存
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
static unsigned int MemIsValidBlock(unsigned int PoolId, unsigned char *pu1Block)
{
    tMemFreePoolRec    *pPoolRecPtr;
    unsigned int        BlockSize;
    unsigned char       *pu1Start;
    unsigned char       *pu1End;

    /*判断形参是否合法*/
    if ((PoolId < 0) || (PoolId >= gtMemPoolCfg.MaxMemPools) || !pu1Block)
    {
        return MEM_FAILURE;
    }
    /*用一些临时变量指向对应的PooId*/
    pPoolRecPtr = &pMemFreePoolRecList[PoolId];
    pu1Start = pPoolRecPtr->pu1StartAddr;
    pu1End = pPoolRecPtr->pu1EndAddr;

    BlockSize = pPoolRecPtr->Size;

    /*判断这一小块内存是否属于该内存池*/
    if (pu1Block >= pu1Start && (pu1Block <= pu1End))
    {
        if ((pu1Block - pu1Start) % BlockSize == 0)
        {
            return MEM_SUCCESS;
        }
        else
        {
            return MEM_OK_BUT_NOT_ALIGNED;
        }
    }
    else
    {
        return MEM_FAILURE;
    }
}
/***************************************************************
 * @file       MemPool.c
 * @brief      从内存池当中回收一小块内存
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
unsigned int MemDeleteMemPool(unsigned int PoolId)
{
    if ((PoolId < 0) || (PoolId >= gtMemPoolCfg.MaxMemPools)
        || pMemFreePoolRecList[PoolId].UnitsCount == 0)
    {
        return MEM_FAILURE;
    }
    /*将这些变量恢复为默认值*/
    pMemFreePoolRecList[PoolId].FreeUnitsCount = 0;
    POOL_MEM_FREE(pMemFreePoolRecList[PoolId].pu1ActualBase);
    pMemFreePoolRecList[PoolId].BufList.pu1Base = NULL;
    pMemFreePoolRecList[PoolId].BufList.pu1Head = NULL;
    pMemFreePoolRecList[PoolId].Size = 0;

    return MEM_SUCCESS;
}
/***************************************************************
 * @file       MemPool.c
 * @brief      获取每一个内存块的字节数
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
unsigned int GetBlockSize(unsigned int PoolId)
{
    return pMemFreePoolRecList[PoolId].Size;
}
/***************************************************************
 * @file       MemPool.c
 * @brief      获取内存池当中的总共内存块个数
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
unsigned int GetUnitsCount(unsigned int PoolId)
{
    return pMemFreePoolRecList[PoolId].UnitsCount;
}
/***************************************************************
 * @file       MemPool.c
 * @brief      获取空闲的内存块个数
 * @author     txj
 * @version    v1
 * @date       2021/1/10
 **************************************************************/
unsigned int GetFreeUnitsCount(unsigned int PoolId)
{
    return pMemFreePoolRecList[PoolId].FreeUnitsCount;
}