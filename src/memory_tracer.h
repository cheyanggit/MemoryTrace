#ifndef  MEMORY_TRACER_H
#define  MEMORY_TRACER_H
#include "memory_define.h"
#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

void * yh_malloc(unsigned int size, int blockType,const char * file, unsigned int line);
void * yh_calloc(unsigned int elements, unsigned int size, int blockType, const char * file, unsigned int line);
void * yh_realloc (void *ptr,int size, int blockType, const char *file,unsigned int line);
char * yh_strdup (const char *str,  const char * file, unsigned int line);
void yh_free(void * ptr, int blockType);


#define  malloc(size)				yh_malloc(size,MEMORY_NORMAL_BLOCK, __FILE__, __LINE__)
#define  calloc(elements, size)		yh_calloc(elements, size, MEMORY_NORMAL_BLOCK,__FILE__, __LINE__)
#define  realloc(ptr,size)			yh_realloc(ptr,size,MEMORY_NORMAL_BLOCK, __FILE__, __LINE__)
#define  strdup(str)				yh_strdup(str, __FILE__, __LINE__)
#define  free(ptr)				yh_free(ptr,MEMORY_NORMAL_BLOCK)

int check_memory();

void dump_memory_info();
void report_memory_info(const char* out_file);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif //MEMORY_TRACER_H