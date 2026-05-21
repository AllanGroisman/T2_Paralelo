#define TAREFAS 7; // Numero de tarefas no saco de trabalho para np = 8, processo 0 é o mestre

int my_rank;       // Identificador deste processo
int proc_n;        // Numero de processos disparados pelo usuário na linha de comando (np)
int message;       // Buffer para as mensagens 
int saco[TAREFAS]; // saco de trabalho
int solucoes_possiveis; // número de soluções possíveis do tabuleiro (coordenador: GLOBAL, trabalhador: LOCAL)
int tamanho_tabuleiro; // AxA (tamanho do tabuleiro = número de rainhas)
int coluna_atual; // usado para verificar se há trabalho (vai de 0 até tamanho_tabuleiro)
int escravos_vivos;

MPI_Init(); // funcao que inicializa o MPI, todo o código paralelo esta abaixo

MPI_Comm_rank( &my_rank );  // pega pega o numero do processo atual (rank)
MPI_Comm_size( &proc_n );   // pega informação do numero de processos (quantidade total)
escravos_vivos = &proc_n - 1;

if ( my_rank == 0 ) // qual o meu papel: sou o mestre ou um dos escravos?
{
    // papel do mestre

    for (i = 1; i < proc_n; i++) // mando o trabalho para os escravos fazerem
    {
        if (temTrabalho()) {
            mandarTrabalhoParaEscravo(i);
        }
    } 

    int solucoes_possiveis_local;
    // recebe a mensagem de soluções possíveis do escravo
    while (temTrabalho()) {
        // recebo mensagens de qualquer emissor e com qualquer etiqueta (TAG)
        MPI_Recv(&solucoes_possiveis_local, MPI_ANY_SOURCE, MPI_ANY_TAG, status);  // recebo por ordem de chegada com any_source
        solucoes_possiveis += solucoes_possiveis_local; // soma na variável global o nº de soluções possíveis enviadas pelo escravo

        mandarTrabalhoParaEscravo(status.MPI_SOURCE); // pega o id do escravo que enviou o nº de soluções possíveis e manda trabalho novamente para esse escravo
    }

    // trabalho acabou
    while (escravos_vivos != 0) {
        MPI_Recv(&solucoes_possiveis_local, MPI_ANY_SOURCE, MPI_ANY_TAG, status);  // recebo por ordem de chegada com any_source
        solucoes_possiveis += solucoes_possiveis_local; // soma na variável global o nº de soluções possíveis enviadas pelo escravo
        matarEscravo(status.MPI_SOURCE); // mata o escravo
    }
}              
else               
{
    int trabalho;
    while(1){
        MPI_Recv(&trabalho, 0);    // recebo do mestre
        MPI_Send(trabalhar(trabalho), 0); // envio trabalho para escravo com id = processoEscravo, com a coluna a ser trabalhada;
        if (trabalho == -1){
            break;
        }
    }
    // papel do escravo


    message = message+1;      // icremento conteúdo da mensagem

    MPI_Send(&message, 0);    // retorno resultado para o mestre
}

// === FUNÇÕES COORDENADOR ===
void mandarTrabalhoParaEscravo(int processoEscravo) {
    MPI_Send(&coluna_atual, processoEscravo); // envio trabalho para escravo com id = processoEscravo, com a coluna a ser trabalhada;
    coluna_atual++;
}

bool temTrabalho() {
    return coluna_atual != tamanho_tabuleiro - 1;
}

void matarEscravo(int processoEscravo) {
    MPI_Send(-1, processoEscravo); 
    escravos_vivos--;
}

// === FUNÇÕES ESCRAVO ===
void receberTrabalho() {

}
void trabalhar(int colunaInicial){
    return colunaInicial * colunaInicial;
}

MPI_Finalize();