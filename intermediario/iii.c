#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int remove_element(int** array, int sizeOfArray, int indexToRemove)
{
  printf("Beginning processing. Array is currently: ");
  for (int i = 0; i < sizeOfArray; ++i)
    printf("%d ", (*array)[i]);
  printf("\n");

  int* temp = malloc((sizeOfArray - 1) * sizeof(int)); // allocate an array with a size 1 less than the current one

  memmove(temp,*array,(indexToRemove+1)*sizeof(int)); // copy everything BEFORE the index

  memmove(temp+indexToRemove,(*array)+(indexToRemove+1),(sizeOfArray - indexToRemove)*sizeof(int)); // copy everything AFTER the index


  printf("Processing done. Array is currently: ");
  for (int i = 0; i < sizeOfArray - 1; ++i)
    printf("%d ", (temp)[i]);
  printf("\n");

  free (*array);
  *array = temp;
  return 0;

}

int main()
{
  int howMany = 20;
  int* test = malloc(howMany * sizeof(int));


  for (int i = 0; i < howMany; ++i)
    (test[i]) = i;



  printf("%d\n", test[16]);
  remove_element(&test, howMany, 1);
  --howMany;
  printf("%d\n", test[16]);
  return 0;

}

  
