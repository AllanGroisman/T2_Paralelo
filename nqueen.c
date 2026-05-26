#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> 
#include <mpi.h>     

// Protótipos das funções
void mandarTrabalhoParaEscravo(int processoEscravo);
bool temTrabalho();
void matarEscravo(int processoEscravo);
int trabalhar(int colunasIniciais[2]);
int place(int board_local[], int row, int col);
void queen(int board_local[], int row, int n, int *count); 

// Variáveis Globais 
int my_rank;       
int proc_n;        
int solucoes_possiveis = 0; 
int tamanho_tabuleiro; 
int escravos_vivos;
double t1, t2; // Tempo inicial e final

// Estrutura do saco de tarefas (Nível 2)
int saco_de_tarefas[1000][2]; 
int total_tarefas = 0;
int tarefa_atual = 0; 

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
        // 1. Gerar o saco de tarefas (combinações válidas das linhas 0 e 1)
        for(int c0 = 0; c0 < tamanho_tabuleiro; c0++) {
            for(int c1 = 0; c1 < tamanho_tabuleiro; c1++) {
                // Verifica se as rainhas nas duas primeiras linhas não se atacam
                if (c1 != c0 && abs(c1 - c0) != 1) {
                    saco_de_tarefas[total_tarefas][0] = c0;
                    saco_de_tarefas[total_tarefas][1] = c1;
                    total_tarefas++;
                }
            }
        }

        printf("Mestre: Iniciando calculo para tabuleiro %dx%d com %d processos.\n", tamanho_tabuleiro, tamanho_tabuleiro, proc_n);
        printf("Mestre: Total de tarefas geradas no saco de trabalho = %d\n", total_tarefas);

        // Tempo inicial
        t1 = MPI_Wtime(); 

        MPI_Status status;
        int solucoes_possiveis_local;

        // 2. Rajada inicial de trabalho
        for (int i = 1; i < proc_n; i++) 
        {
            if (temTrabalho()) {
                mandarTrabalhoParaEscravo(i);
            }
        } 

        // 3. Loop de recebimento e envio (enquanto houver trabalho)
        while (temTrabalho()) {
            MPI_Recv(&solucoes_possiveis_local, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  
            solucoes_possiveis += solucoes_possiveis_local; 

            mandarTrabalhoParaEscravo(status.MPI_SOURCE); 
        }

        // 4. Trabalho acabou, finaliza os escravos
        while (escravos_vivos != 0) {
            MPI_Recv(&solucoes_possiveis_local, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  
            solucoes_possiveis += solucoes_possiveis_local; 
            
            matarEscravo(status.MPI_SOURCE); 
        }

        t2 = MPI_Wtime(); // Tempo final

        printf("\nTempo de execucao: %f segundos\n", t2-t1);  
        printf("Mestre: Total de solucoes possiveis encontradas = %d\n", solucoes_possiveis);
    }              
    else               
    {
        // Papel do escravo
        int trabalho[2];
        MPI_Status status;
        
        while(1){
            // Recebe 2 inteiros (coluna da linha 0 e coluna da linha 1)
            MPI_Recv(trabalho, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);    
            
            // Verifica o sinal de morte no primeiro elemento do array
            if (trabalho[0] == -1){
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
    // Envia o array de 2 posições
    MPI_Send(saco_de_tarefas[tarefa_atual], 2, MPI_INT, processoEscravo, 0, MPI_COMM_WORLD); 
    tarefa_atual++;
}

bool temTrabalho() {
    return tarefa_atual < total_tarefas;
}

void matarEscravo(int processoEscravo) {
    int sinal_de_morte[2] = {-1, -1};
    // Envia um array com -1 para matar o escravo
    MPI_Send(sinal_de_morte, 2, MPI_INT, processoEscravo, 0, MPI_COMM_WORLD); 
    escravos_vivos--;
}

// === FUNÇÕES ESCRAVO ===
int trabalhar(int colunasIniciais[2]){
    int solucoes_locais = 0;
    
    int board_local[tamanho_tabuleiro];
    memset(board_local, -1, sizeof(int) * tamanho_tabuleiro);

    // Fixa as rainhas das linhas 0 e 1 com base no trabalho recebido
    board_local[0] = colunasIniciais[0];
    board_local[1] = colunasIniciais[1];

    // Inicia a recursão da linha 2 em diante
    queen(board_local, 2, tamanho_tabuleiro, &solucoes_locais);
    
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

