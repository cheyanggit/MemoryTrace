#ifndef  MEMORY_TRACER_H
#define  MEMORY_TRACER_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

void * yh_malloc(unsigned int size, const char * file, unsigned int line);
void * yh_calloc(unsigned int elements, unsigned int size, const char * file, unsigned int line);
void * yh_realloc (void *ptr,int size,const char *file,unsigned int line);
char * yh_strdup (const char *ptr, const char * file, unsigned int line);
void yh_free(void * ptr);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif //MEMORY_TRACER_H