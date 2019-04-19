#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "datatypes.h"
#include "pingpong.h"

// #define DEBUG

#define STACKSIZE 32768 // Tamanho da pilha das threads.

ucontext_t contextMain;

task_t mainTask, *taskAtual; // Descritor da tarefa principal e ponteiro para a tarefa atual.

int tid; // ID declarado globalmente, para torná-lo único para cada tarefa criada.

// Funções gerais ==============================================================

// Inicializa o sistema operacional; deve ser chamada no inicio do mainTask()
void pingpong_init () {
    /* Desativa o buffer da saida padrao (stdout), usado pela função printf
     * para evitar condições de disputa que podem ocorrer nesse buffer ao
     * usar as funções de troca de contexto. */
    setvbuf (stdout, 0, _IONBF, 0) ;

    tid = 0;

    getcontext(&contextMain); // Inicializa contexto principal.

    mainTask.next = NULL;
    mainTask.prev = NULL;
    mainTask.tid = tid;
    mainTask.context = contextMain;

    taskAtual = &mainTask;

    #ifdef DEBUG
        printf("pinpong_init: Inicialização finalizada. Tarefa principal (id: %d) criada. \n", mainTask.tid);
    #endif
};

// Gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um 'ID > 0' ou erro.
int task_create (task_t *task, // descritor da nova tarefa
                 void (*start_func)(void *), // função corpo da tarefa
                 void *arg) { // argumentos para a tarefa
    char *stack ;
    
    tid++;

    ucontext_t context;
    getcontext(&context);

    stack = malloc (STACKSIZE);
    if (stack)
    {
        context.uc_stack.ss_sp = stack ;
        context.uc_stack.ss_size = STACKSIZE;
        context.uc_stack.ss_flags = 0;
        context.uc_link = 0;
    }
    else
    {
        perror ("Erro na criação da pilha: ");
        exit (1);
    }

    makecontext (&context, (void*)(*start_func), tid,  arg);
    
    task->tid = tid;
    task->next = NULL;
    task->prev = NULL;
    task->context = context;

    #ifdef DEBUG
        printf ("task_create: criou tarefa %d\n", task->tid) ;
    #endif

    return task->tid;
};

// Termina a tarefa corrente, indicando um valor de status encerramento.
void task_exit (int exitCode) {
    task_switch(&mainTask);
};

// Alterna a execução para a tarefa indicada.
int task_switch (task_t *task) {
    task_t *aux = taskAtual;
    taskAtual = task;

    swapcontext(&aux->context, &task->context);

    #ifdef DEBUG
        printf("\ntask_switch: trocando de contexto.");
    #endif
        
    return 0;
};

// Retorna o identificador da tarefa corrente (mainTask eh 0).
int task_id () {
    return taskAtual->tid;
};

// Suspende uma tarefa, retirando-a de sua fila atual, adicionando-a à fila
// queue e mudando seu estado para 'suspensa'; usa a tarefa atual se 'task == NULL'.
void task_suspend (task_t *task, task_t **queue) {

};

// Acorda uma tarefa, retirando-a de sua fila atual, adicionando-a à fila de
// tarefas prontas ('ready queue') e mudando seu estado para 'pronta'.
void task_resume (task_t *task) { 

};

// Operações de escalonamento ==================================================

// Libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ('ready queue').
void task_yield () {

};

// Define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) {

};

// Retorna a prioridade estática de uma tarefa (ou a tarefa atual).
int task_getprio (task_t *task) {
    return 0;
};

// Operações de sincronização ==================================================

// A tarefa corrente aguarda o encerramento de outra task.
int task_join (task_t *task) {
    return 0;
};

// Operações de gestão do tempo ================================================

// Suspende a tarefa corrente por t segundos.
void task_sleep (int t) {
    
};

// Retorna o relógio atual (em milisegundos).
unsigned int systime () ;