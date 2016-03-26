#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct mystruct
{
  int op;
  char val[5];
}MS;

typedef struct mmm
{
  MS pin;
}MMM;
  
int main()
{
  MS algo;
  algo.op = 4;
  strcpy(algo.val, "pru");
  printf("Valores de algo int:%d, char:%s", algo.op, algo.val);

  MMM *aaa;
  aaa=calloc(1, sizeof(MMM));
  memcpy(&(aaa[0].pin), &algo, sizeof(MS));

  printf("Valores de aaa[0] int:%d, char:%s", aaa[0].pin.op, aaa[0].pin.val);

  free(aaa);
  return 0;
  
}
