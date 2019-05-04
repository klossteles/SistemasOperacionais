#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include "datatypes.h"
#include "pingpong.h"
#include "queue.h"

#define STACKSIZE 32768
#define USER_TASK 1
#define SYSTEM_TASK 0 

int tid;
ucontext_t contextMain, contextDispatcher;
task_t mainTask, *taskAtual, taskDispatcher;
task_t *prontas;
int alpha = -1;

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action;

// estrutura de inicialização to timer
struct itimerval timer;

int quantum;

// funções gerais ==============================================================

task_t* scheduler() {
    if (queue_size((queue_t *)prontas) > 0) {
        task_t *aux = prontas->next;
        task_t *next = prontas;  
        while (aux != prontas) {
            if (aux->dinamic_priority < next->dinamic_priority) {
                next = aux;
            }
            aux = aux->next;
        }

        aux = prontas->next;
        while(aux != prontas){
            if (aux != next) {
                if (aux->dinamic_priority + alpha < -20) {
                    aux->dinamic_priority = -20;
                } else if (aux->dinamic_priority + alpha > 20){
                    aux->dinamic_priority = 20;
                } else {
                    aux->dinamic_priority = aux->dinamic_priority + alpha;
                }
            } else {
                next->dinamic_priority = next->static_priority;
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
        #ifdef DEBUG
            printf("dispatcher_body: próxima tarefa id: %d, prioridade: %d\n", next->tid, next->dinamic_priority);
        #endif
        if (next != NULL) {
            //... // ações antes de lançar a tarefa "next", se houverem
            queue_remove((queue_t **) &prontas,(queue_t*) next);
            #ifdef DEBUG
                printf("dispatcher_body: removeu task da fila id: %d\n", next->tid);
            #endif
            quantum = 20;
            task_switch (next) ; // transfere controle para a tarefa "next"
            #ifdef DEBUG
                printf("dispatcher_body: possui next com id: %d, removendo da fila e mudando contexto\n", next->tid);
            #endif
            //... // ações após retornar da tarefa "next", se houverem
        }
    }
    task_exit(0); // encerra a tarefa dispatcher
};

void tratador (int signum)
{
    if (taskAtual->task_type == USER_TASK){
        if (quantum == 0) {
            #ifdef DEBUG
                printf("tratador: quantum ZERO, voltando processador para dispatcher\n");
            #endif
            task_yield();
            // task_switch(&taskDispatcher);
        } else {
            quantum--;
            #ifdef DEBUG
                printf("tratador: quantum diferente de ZERO, decrementando quantum: %d\n", quantum);
            #endif
        }
    }
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
    mainTask.static_priority = 0;
    mainTask.dinamic_priority = 0;
    mainTask.task_type = USER_TASK;

    taskAtual = &mainTask;
   
    task_create(&taskDispatcher, dispatcher_body, "Dispatcher");

    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }

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
    task->static_priority = 0;
    task->dinamic_priority = 0;

    if (task == &taskDispatcher) {
        task->task_type = SYSTEM_TASK;
    } else {
        task->task_type = USER_TASK;
    }

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
    task_t *aux;
    aux = taskAtual;
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
        taskAtual->static_priority = prio;
        #ifdef DEBUG
            printf("task_setprio: taskAtual com id: %d e prioridade: %d muda para \n", taskAtual->tid, taskAtual->static_priority);
        #endif
    } else {
        task->static_priority = task->static_priority + prio;
        #ifdef DEBUG
            printf("task_setprio: task com id: %d e prioridade: %d\n", task->tid, task->static_priority);
        #endif
    }
};

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) {
    if (task == NULL) {
        return taskAtual->static_priority;
    } else {
        return task->static_priority;
    }
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