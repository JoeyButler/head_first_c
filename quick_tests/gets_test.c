#include <stdio.h>
#include <string.h>

int main(int argc, const char *argv[])
{
  char* there = "Who's there?";
  char* other = "Who's there?";
  printf("%i", strcmp(there, other));
  return 0;
}
