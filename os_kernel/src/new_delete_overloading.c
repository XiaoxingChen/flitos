#include <stdlib.h>
#include "cs251_os.h"
extern volatile char *VIDEO_MEMORY;

#define NEW_DELETE_TRACE 0

void* operator new(size_t sz)
{
#if NEW_DELETE_TRACE
  uint32_t return_address;
  asm volatile ("mv %0, ra" : "=r"(return_address));
  LOGD("%s:%d; thread: %d; ra: 0x%X\n", __FILE__, __LINE__, -1, return_address);
#endif
  void* m = malloc(sz);
  return m;
}
// Overloading Global delete operator
void operator delete(void* m)
{
#if NEW_DELETE_TRACE
  uint32_t return_address;
  asm volatile ("mv %0, ra" : "=r"(return_address));
  LOGD("%s:%d; thread: %d; ra: 0x%X, pts: 0x%X\n", __FILE__, __LINE__, cs251::schedulerInstance().runningThreadID(), return_address, m);
#endif
  free(m);
}
// Overloading Global new[] operator
void* operator new[](size_t sz)
{
#if NEW_DELETE_TRACE
  uint32_t return_address;
  asm volatile ("mv %0, ra" : "=r"(return_address));
  LOGD("%s:%d; thread: %d; ra: 0x%X\n", __FILE__, __LINE__, cs251::schedulerInstance().runningThreadID(), return_address);
#endif
  void* m = malloc(sz);
  return m;
}
// Overloading Global delete[] operator
void operator delete[](void* m)
{
#if NEW_DELETE_TRACE
  uint32_t return_address;
  asm volatile ("mv %0, ra" : "=r"(return_address));
  LOGD("%s:%d; thread: %d; ra: 0x%X\n", __FILE__, __LINE__, cs251::schedulerInstance().runningThreadID(), return_address);
#endif
  free(m);
}