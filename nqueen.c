#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> // Adicionado para o memset
#include <mpi.h>     

// Protótipos das funções
void mandarTrabalhoParaEscravo(int processoEscravo);
bool temTrabalho();
void matarEscravo(int processoEscravo);
int trabalhar(int colunaInicial);
int place(int board_local[], int row, int col);
void queen(int board_local[], int row, int n, int *count); // Alterado para int *

// Variáveis Globais (O Mestre usa para controle, cada Escravo terá sua cópia na sua memória)
int my_rank;       
int proc_n;        
int solucoes_possiveis = 0; 
int tamanho_tabuleiro = 8; 
int coluna_atual = 0; 
int escravos_vivos;

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);   
    
    escravos_vivos = proc_n - 1;

    // Se sou o mestre
    if ( my_rank == 0 ) 
    {
        MPI_Status status;
        int solucoes_possiveis_local;

        // Rajada inicial de trabalho
        for (int i = 1; i < proc_n; i++) 
        {
            if (temTrabalho()) {
                mandarTrabalhoParaEscravo(i);
            }
        } 

        // Loop de recebimento e envio (enquanto houver trabalho)
        while (temTrabalho()) {
            MPI_Recv(&solucoes_possiveis_local, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  
            solucoes_possiveis += solucoes_possiveis_local; 

            mandarTrabalhoParaEscravo(status.MPI_SOURCE); 
        }

        // Trabalho acabou, finaliza os escravos
        while (escravos_vivos != 0) {
            MPI_Recv(&solucoes_possiveis_local, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  
            solucoes_possiveis += solucoes_possiveis_local; 
            
            matarEscravo(status.MPI_SOURCE); 
        }
        
        printf("Mestre: Total de solucoes possiveis encontradas = %d\n", solucoes_possiveis);
    }              
    else               
    {
        // Papel do escravo
        int trabalho;
        MPI_Status status;
        
        while(1){
            MPI_Recv(&trabalho, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);    
            
            if (trabalho == -1){
                break; 
            }
            
            int resultado = trabalhar(trabalho);
            MPI_Send(&resultado, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); 
        }
    }

    MPI_Finalize();
    return 0;
}

// === FUNÇÕES COORDENADOR ===
void mandarTrabalhoParaEscravo(int processoEscravo) {
    MPI_Send(&coluna_atual, 1, MPI_INT, processoEscravo, 0, MPI_COMM_WORLD); 
    coluna_atual++;
}

bool temTrabalho() {
    return coluna_atual < tamanho_tabuleiro;
}

void matarEscravo(int processoEscravo) {
    int sinal_de_morte = -1;
    MPI_Send(&sinal_de_morte, 1, MPI_INT, processoEscravo, 0, MPI_COMM_WORLD); 
    escravos_vivos--;
}

// === FUNÇÕES ESCRAVO ===
int trabalhar(int colunaInicial){
    printf("Escravo %d: Recebi e estou calculando a coluna inicial %d\n", my_rank, colunaInicial);
    
    // Variável local para armazenar as soluções desta subárvore específica
    int solucoes_locais = 0;

    int board_local[tamanho_tabuleiro];
    memset(board_local, -1, sizeof(board_local));

    /* Fixa a rainha da linha 0 na coluna recebida */
    board_local[0] = colunaInicial;

    queen(board_local, 1, tamanho_tabuleiro, &solucoes_locais);
    
    return solucoes_locais;
}

// === FUNCOES NQUEENS ===
int place(int board_local[], int row, int col) {
    for (int i = 0; i < row; i++) {
        if (board_local[i] == col)                      
            return 0;
        if (abs(board_local[i] - col) == abs(i - row))  
            return 0;
    }
    return 1;
}

void queen(int board_local[], int row, int n, int *count) { // Alterado para int *
    if (row == n) {          
        (*count)++;
        return;
    }

    for (int col = 0; col < n; col++) {
        if (place(board_local, row, col)) {
            board_local[row] = col;
            queen(board_local, row + 1, n, count);
            board_local[row] = -1;  
        }
    }
}