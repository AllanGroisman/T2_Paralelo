#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> 
#include <mpi.h>     

// Protótipos das funções
void mandarTrabalhoParaEscravo(int processoEscravo);
bool temTrabalho();
void matarEscravo(int processoEscravo);
int trabalhar(int colunaInicial);
int place(int board_local[], int row, int col);
void queen(int board_local[], int row, int n, int *count); 

// Variáveis Globais 
int my_rank;       
int proc_n;        
int solucoes_possiveis = 0; 
int tamanho_tabuleiro; // Agora não tem valor fixo inicial
int coluna_atual = 0; 
int escravos_vivos;
double t1, t2; // tempo inicial e final

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);   
    
    // Verificação dos argumentos de linha de comando
    if (argc != 2) {
        if (my_rank == 0) {
            printf("Uso correto: mpirun -np <processos> %s <numero_de_rainhas>\n", argv[0]);
        }
        MPI_Finalize();
        return 0;
    }

    // Pega o argumento digitado no terminal e converte para int
    tamanho_tabuleiro = atoi(argv[1]);

    if (tamanho_tabuleiro <= 0) {
        if (my_rank == 0) {
            printf("Erro: O numero de rainhas deve ser maior que 0.\n");
        }
        MPI_Finalize();
        return 0;
    }

    escravos_vivos = proc_n - 1;

    // Se sou o mestre
    if ( my_rank == 0 ) 
    {
        //tempo inicial
        t1 = MPI_Wtime(); // inicia a contagem do tempo

        MPI_Status status;
        int solucoes_possiveis_local;

        printf("Mestre: Iniciando calculo para tabuleiro %dx%d com %d processos.\n", tamanho_tabuleiro, tamanho_tabuleiro, proc_n);

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
        t2 = MPI_Wtime(); // termina a contagem do tempo
        printf("\nTempo de execucao: %f\n\n", t2-t1);  
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
    int solucoes_locais = 0;
    
    // O tamanho do array agora é dinâmico com base no argumento passado
    int board_local[tamanho_tabuleiro];
    memset(board_local, -1, sizeof(board_local));

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

void queen(int board_local[], int row, int n, int *count) { 
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