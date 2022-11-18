#include <stdlib.h>
extern volatile char *VIDEO_MEMORY;
void* operator new(size_t sz)
{
  void* m = malloc(sz);

  return m;
}
// Overloading Global delete operator
void operator delete(void* m)
{
  free(m);
}
// Overloading Global new[] operator
void* operator new[](size_t sz)
{
  void* m = malloc(sz);
  return m;
}
// Overloading Global delete[] operator
void operator delete[](void* m)
{
  free(m);
}