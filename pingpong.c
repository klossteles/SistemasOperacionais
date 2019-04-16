#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "datatypes.h"
#include "pingpong.h"

// #define DEBUG

#define STACKSIZE 32768

int tid;
ucontext_t contextMain;
task_t mainTask, *taskAtual;

// funções gerais ==============================================================

// Inicializa o sistema operacional; deve ser chamada no inicio do mainTask()
void pingpong_init () {
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
    tid = 0;
    getcontext(&contextMain);
    mainTask.next = NULL;
    mainTask.prev = NULL;
    mainTask.tid = tid;
    mainTask.context = contextMain;

    taskAtual = &mainTask;
    #ifdef DEBUG
        printf("pinpong_init: Inicialização finalizada. Tarefa principal (id: %d) criada. \n", mainTask.tid);
    #endif
};

// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task,			// descritor da nova tarefa
                 void (*start_func)(void *),	// funcao corpo da tarefa
                 void *arg) {
    char *stack ;
    tid++;

    ucontext_t context;
    getcontext(&context);
    stack = malloc (STACKSIZE) ;
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

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode) {
    task_switch(&mainTask);
};

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) {
    task_t *aux = taskAtual;
    taskAtual = task;
    swapcontext(&aux->context, &task->context);
    #ifdef DEBUG
        printf("\ntask_switch: trocando de contexto.");
    #endif
    return 0;
};

// retorna o identificador da tarefa corrente (mainTask eh 0)
int task_id () {
    return taskAtual->tid;
};

// suspende uma tarefa, retirando-a de sua fila atual, adicionando-a à fila
// queue e mudando seu estado para "suspensa"; usa a tarefa atual se task==NULL
void task_suspend (task_t *task, task_t **queue) {

};

// acorda uma tarefa, retirando-a de sua fila atual, adicionando-a à fila de
// tarefas prontas ("ready queue") e mudando seu estado para "pronta"
void task_resume (task_t *task) { 

};

// operações de escalonamento ==================================================

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield () {

};

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) {

};

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) {
    return 0;
};

// operações de sincronização ==================================================

// a tarefa corrente aguarda o encerramento de outra task
int task_join (task_t *task) {
    return 0;
};

// operações de gestão do tempo ================================================

// suspende a tarefa corrente por t segundos
void task_sleep (int t) {
    
};

// retorna o relógio atual (em milisegundos)
unsigned int systime () ;