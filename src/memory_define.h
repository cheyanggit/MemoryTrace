#ifndef  MEMORY_DEFINE_H
#define  MEMORY_DEFINE_H

#ifdef _WIN64
#define PLATFORM_TARGET_64 1
#endif //_WIN64

typedef struct _MemoryBlockHeader
{
        struct _MemoryBlockHeader * blockHeaderNext;
        struct _MemoryBlockHeader * blockHeaderPrev;
        char *                      fileName;
        int                         line;
#ifdef PLATFORM_TARGET_64
        /* These items are reversed on Win64 to eliminate gaps in the struct
         * and ensure that sizeof(struct)%16 == 0, so 16-byte alignment is
         * maintained in the debug heap.
         */
        int                         blockUse;
        size_t                      dataSize;
#else  /* PLATFORM_TARGET_64 */
        size_t                      dataSize;
        int                         blockUse;
#endif  /* PLATFORM_TARGET_64 */
        long                        request;
        unsigned char               gap[nNoMansLandSize];
        /* followed by:
         *  unsigned char           data[nDataSize];
         *  unsigned char           anotherGap[nNoMansLandSize];
         */
} MemoryBlockHeader;

#define Memory_GetDataPtr(pblock) ((unsigned char *)((MemoryBlockHeader *)pblock + 1))
#define Memory_GetBlockPtr(pbData) (((MemoryBlockHeader *)pbData)-1)

//Block Type
#define MEMORY_FREE_BLOCK      0
#define MEMORY_NORMAL_BLOCK    1
#define MEMORY_CRT_BLOCK       2
#define MEMORY_IGNORE_BLOCK    3
#define MEMORY_CLIENT_BLOCK    4
#define MEMORY_MAX_BLOCKS      5
#endif //MEMORY_DEFINE_H