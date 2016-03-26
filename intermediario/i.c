#include <stdio.h>
#include <stdarg.h>

/* print all args one at a time until a negative argument is seen;
   all args are assumed to be of int type */
void printargs(int arg1, ...)
{
  va_list ap;
  int i;

  va_start(ap, arg1); 
  for (i = 0; i < arg1; i++)
    printf("%d ", va_arg(ap, int));
  va_end(ap);
  putchar('\n');
}

int main(void)
{
  printargs(5, 2, 14, 84, 97, 15, 2);
  printargs(2, 51, -1);
  printargs(0, 23);
  printargs(1, -1);
  printargs(-1);
  return 0;
}
