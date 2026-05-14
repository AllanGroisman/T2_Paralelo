
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int l,c,i,j;

    l = 6;
    c = 4;

    int (*M)[c] = malloc (l * sizeof *M);

    printf("\n Matrix M[%d,%d]", l , c );
    printf("\n Number of bytes: %d ", l * sizeof * M );
    printf("\n Number of int values: %d ", l *c );
    printf("\n Size of an int value: %d \n\n", sizeof(int) );

   	for(i=0;i<l;i++)
        for(j=0;j<c;j++)
            M[i][j] = i*j;

	for(i=0;i<l;i++)
        {
        for(j=0;j<c;j++)
            printf("%2d ",M[i][j]);
        printf("\n");
        }

    free(M);
}