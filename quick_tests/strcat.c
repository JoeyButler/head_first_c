#include <stdio.h>
#include <string.h>

int main(int argc, const char *argv[])
{
  char str1[50] = "Hello ";
  printf("%i", strlen(str1));
  char* str2 = "World!";
  strcat(str1, "other World!");
  printf("%s", str1); 
  return 0;
}
