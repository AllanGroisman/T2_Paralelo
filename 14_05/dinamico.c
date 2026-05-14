
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000

int main(int argc, char **argv)
{
    int *vet = (int *)malloc(sizeof(int)*SIZE);
    int i;

    printf("\n Vetor vet[%d]", SIZE);
    printf("\n Number of bytes: %d ", SIZE * sizeof * vet );
    printf("\n Number of int values: %d ", SIZE);
    printf("\n Size of an int value: %d \n\n", sizeof(int) );

   	for(i=0;i<SIZE;i++)
        vet[i] = SIZE - i;

	for(i=0;i<SIZE;i++)
        printf("%2d  ",vet[i]);

    printf("\n");
    free(vet);
}