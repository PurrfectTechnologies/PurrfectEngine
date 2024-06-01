#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char **argv) {
  if (argc != 2) return 1;

  const char *input_filename = argv[1];

  FILE* input_fd = fopen(input_filename, "rb");

  fseek(input_fd, 0, SEEK_END);
    long size = ftell(input_fd);
  fseek(input_fd, 0, SEEK_SET);

  char *buffer = malloc(size);
  fread(buffer, size, 1, input_fd);
  fclose(input_fd);

  printf("std::vector<char> code = {");
  for (long i = 0; i < size; ++i) {
    if (i > 0) printf(", ");
    printf("%d", buffer[i]);
  }
  printf("};\n");

  free(buffer);

  return 0;
}