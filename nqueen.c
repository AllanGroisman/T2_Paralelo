#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // Necessário para usar bool
#include <mpi.h>     // Header correto do MPI costuma ser <mpi.h>

// Protótipos das funções
void mandarTrabalhoParaEscravo(int processoEscravo);
bool temTrabalho();
void matarEscravo(int processoEscravo);
int trabalhar(int colunaInicial);

// Variáveis Globais
int my_rank;       
int proc_n;        
int solucoes_possiveis = 0; 
int tamanho_tabuleiro = 8; // Inicializado com 8 (ex: tabuleiro 8x8)
int coluna_atual = 0; 
int escravos_vivos;

int main(int argc, char **argv)
{
    // Inicialização correta do MPI
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);   
    
    // Corrigido: tirado o '&' comercial
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
            // Assinatura correta do MPI_Recv
            MPI_Recv(&solucoes_possiveis_local, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  
            solucoes_possiveis += solucoes_possiveis_local; 

            mandarTrabalhoParaEscravo(status.MPI_SOURCE); 
        }

        // Trabalho acabou, esperando os escravos terminarem e mandando sinal de morte
        while (escravos_vivos != 0) {
            MPI_Recv(&solucoes_possiveis_local, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  
            solucoes_possiveis += solucoes_possiveis_local; 
            
            matarEscravo(status.MPI_SOURCE); 
        }
        
        printf("Mestre: Total de solucoes possiveis calculadas = %d\n", solucoes_possiveis);
    }              
    else               
    {
        // Papel do escravo
        int trabalho;
        MPI_Status status;
        
        while(1){
            // Assinatura correta do MPI_Recv para receber do Mestre (source 0)
            MPI_Recv(&trabalho, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);    
            
            if (trabalho == -1){
                break; // Sinal de morte recebido
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
    // Assinatura correta do MPI_Send
    MPI_Send(&coluna_atual, 1, MPI_INT, processoEscravo, 0, MPI_COMM_WORLD); 
    coluna_atual++;
}

bool temTrabalho() {
    // Corrigido para garantir que pegue todas as colunas de 0 até (tamanho_tabuleiro - 1)
    return coluna_atual < tamanho_tabuleiro;
}

void matarEscravo(int processoEscravo) {
    int sinal_de_morte = -1;
    // Precisamos enviar a referência de uma variável, não a constante diretamente
    MPI_Send(&sinal_de_morte, 1, MPI_INT, processoEscravo, 0, MPI_COMM_WORLD); 
    escravos_vivos--;
}

// === FUNÇÕES ESCRAVO ===
// Retorno alterado de void para int para bater com a lógica do código
int trabalhar(int colunaInicial){
    printf("Escravo %d: Recebi e estou calculando a coluna %d\n", my_rank, colunaInicial);
    solucoes_possiveis = 0;

    //A PARTIR DAQUI É UMA TAREFA ()
    //cria um board local para esta thread
    int board_local[tamanho_tabuleiro];

    memset(board_local, -1, sizeof(board_local));


    /* fixa rainha da linha 0 */
    board_local[0] = colunaInicial;

    //continua na subarvore deste
    queen(board_local, 2, n, &solucoes_possiveis);
    
    // Operação dummy apenas para teste
    return solucoes_possiveis;
}

// === FUNCOES NQUEENS ===
//função que verifica se pode por rainha
int place(int board_local[], int row, int col) {
    //pra cada linha anterior onde já tem rainhas
    for (int i = 0; i < row; i++) {
        //verifica se ta na mesma coluna
        if (board_local[i] == col)                       
            return 0;
        // verifica se ta na mesma diagonal
        if (abs(board_local[i] - col) == abs(i - row))  
            return 0;
    }
    return 1;
}


void queen(int board_local[], int row, int n, long long *count) {
    //caso base -> se cheguei no fim do tabuleiro, conto como solução pois todas as rainhas foram postas neste cenario
    if (row == n) {          
        (*count)++;
        return;
    }

    //se ainda não chegamos,
    //tenta-se colocar mais uma rainha na linha atual
    //pra cada coluna do tabuleiro
    for (int col = 0; col < n; col++) {
        //se é possível colocar uma rainha
        if (place(board_local, row, col)) {
            //poe a rainha
            board_local[row] = col;
            //e continua na linha de baixo (row + 1) recursivamente
            queen(board_local, row + 1, n, count);
            //tanto faz se achou solucao ou nao, esta rainha é apagada desta posição
            //assim no proximo ciclo do for é possivel testar a rainha na proxima posicao
            //gerando o proximo cenario possivel
            board_local[row] = -1;  
        }
    }
}