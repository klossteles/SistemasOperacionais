#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "datatypes.h"
#include "pingpong.h"
#include "queue.h"

#define STACKSIZE 32768

int tid;
ucontext_t contextMain, contextDispatcher;
task_t mainTask, *taskAtual, taskDispatcher;
task_t *prontas;
int alpha = -1;

// funções gerais ==============================================================

task_t* scheduler() {
    if (queue_size((queue_t *)prontas) > 0) {
        task_t *aux = prontas->next;
        task_t *next = prontas;  
        while (aux != prontas) {
            if (task_getprio(aux) < task_getprio(next)) {
                next = aux;
            }
            aux = aux->next;
        }

        aux = prontas->next;
        while(aux != prontas){
            if (aux != next) {
                task_setprio(aux, alpha);
            }
            aux = aux->next;
        }        

        return next;
    }
    return 0;
};

// dispatcher é uma tarefa
void dispatcher_body ()  {
    task_t *next;
    while ( queue_size((queue_t *)prontas) > 0 ) {
        next = scheduler() ; // scheduler é uma função
        if (next) {
            //... // ações antes de lançar a tarefa "next", se houverem
            #ifdef DEBUG
                printf("dispatcher_body: possui next com id: %d, removendo da fila e mudando contexto\n", next->tid);
            #endif
            queue_remove((queue_t **) &prontas,(queue_t*) next);
            task_switch (next) ; // transfere controle para a tarefa "next"
            //... // ações após retornar da tarefa "next", se houverem
        }
    }
    task_exit(0); // encerra a tarefa dispatcher
};

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
    task_create(&taskDispatcher, dispatcher_body, "Dispatcher");
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
    task->priority = 0;

    if (task != &taskDispatcher) {
        queue_append((queue_t **) &prontas, (queue_t *)task);
    }

    #ifdef DEBUG
        printf ("task_create: criou tarefa %d\n", task->tid) ;
    #endif
    return task->tid;
};

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode) {
    if (taskAtual == &taskDispatcher){
        #ifdef DEBUG
            printf("task_exit: finalizando task dispatcher e indo para main\n");
        #endif
        task_switch(&mainTask);
    } else {
        #ifdef DEBUG
            printf("task_exit: finalizando task %d e indo para dispatcher\n", taskAtual->tid);
        #endif
        task_switch(&taskDispatcher);
    }
};

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) {
    task_t *aux = taskAtual;
    taskAtual = task;
    swapcontext(&aux->context, &task->context);
    #ifdef DEBUG
        printf("task_switch: trocando de contexto. task_id: %d\n", task->tid);
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
    // queue_remove((queue_t **) &prontas, (queue_t *)taskAtual); //remove da fila de prontas
    if (taskAtual != &mainTask && taskAtual != &taskDispatcher) {
        queue_append((queue_t **) &prontas, (queue_t *)taskAtual); //adiciona em último da fila de prontas
    }
    #ifdef DEBUG
        printf("task_yield: Mudando para task dispatcher\n");
    #endif
    task_switch(&taskDispatcher);
};

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) {
    if (task == NULL){
        #ifdef DEBUG
            printf("task_setprio: taskAtual com id: %d e prioridade: %d\n", taskAtual->tid, taskAtual->priority);
        #endif
        if (taskAtual->priority + prio > -20){
            taskAtual->priority = taskAtual->priority + prio;
        }
    } else {
        #ifdef DEBUG
            printf("task_setprio: task com id: %d e prioridade: %d\n", task->tid, task->priority);
        #endif
        if (task->priority + prio > -20){
            task->priority = task->priority + prio;
        }
    }
};

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) {
    return task->priority;
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