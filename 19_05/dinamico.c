
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define SIZE 10

int main(int argc, char **argv)
{
    int *vet = (int *)malloc(sizeof(int) * SIZE);
    int i;
    int my_rank; // Identificador deste processo
    int proc_n;
    int message;

    printf("\n Vetor vet[%d]", SIZE);
    printf("\n Number of bytes: %d ", SIZE * sizeof *vet);
    printf("\n Number of int values: %d ", SIZE);
    printf("\n Size of an int value: %d \n\n", sizeof(int));

    // populando vetor
    for (i = 0; i < SIZE; i++)
        vet[i] = SIZE - i;

    printf("VETOR NORMAL:");
    for (i = 0; i < SIZE; i++)
        printf("%2d  ", vet[i]);

    printf("--------------------------------");

    printf("\n");

    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)

    // sou o coordenador
    if (my_rank == 0)
    {
        // numero de trabalhadores é o numero de processos -1
        int num_trabalhadores = proc_n - 1;

        // Pra cada trabalhador
        for (i = 0; i < num_trabalhadores; i++)
        {
            // enviar pro proximo trabalhador
            int num_msg = SIZE / num_trabalhadores;

            for (int j = 0; j < num_msg; j++)
            {
                // i = id do trabalhador (dezena)
                // j = numero da msg (0 a num de msg)
                message = vet[j + (i * num_msg)];
                MPI_Send(&message, 1, MPI_INT, i + 1, 1, MPI_COMM_WORLD); // envio para os trabalhadores
            }
        }
    }

    // se sou trabalhador
    else
    {
        MPI_Recv(&message, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // recebo do coordenador
        printf("Sou o trabalhador: %d", my_rank);
        printf("Recebi o numero: %d", message);

        &message = message * message;

        MPI_Send(&message, 1, MPI_INT, 0, 1, MPI_COMM_WORLD); // envio para os trabalhadores
    }

    // RECEBIMENTO DO COORDENADOR
    if (my_rank == 0)
    {
        // espera receber todas as mensagens
        for (i = 0; i < SIZE; i++)
        {
            // recebo de qualquer um
            MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            // vejo quem é o trabalhador
            printf("%d",&message);

        }

        

        printf("\n");
    }

    free(vet);
}