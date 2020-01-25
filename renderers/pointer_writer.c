#include "pointer_writer.h"
#include <stdio.h>

void write_pointer(void *pointer)
{
  FILE *file = fopen("pointers.txt", "a");
  fprintf(file, "0x%p\n", pointer);
  fclose(file);
}
