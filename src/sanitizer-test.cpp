#include <stdlib.h>
#include <stdio.h>

int main()  // leak sanitizer test
{
  void *p = malloc(32);
  printf("%p\n", p);
  return 0;
}
