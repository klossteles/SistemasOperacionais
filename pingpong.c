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

enum State {NEW = 0, READY = 1, RUNNING = 2, SUSPENDED = 3, TERMINATED = 4};

int tid;
ucontext_t contextMain, contextDispatcher;
task_t mainTask, *taskAtual, taskDispatcher;
task_t *prontas, *suspensas, *adormecidas;
int alpha = -1;

enum State {NEW = 0, READY = 1, RUNNING = 2, SUSPENDED = 3, TERMINATED = 4, SLEEPING = 5};

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action;

// estrutura de inicialização to timer
struct itimerval timer;

int quantum;

int ticks;

// funções gerais ==============================================================

// task_t* scheduler() {
//     if (queue_size((queue_t *)prontas) > 0) {
//         task_t *next = prontas;
//         return next;
//     }
//     return 0;
// };

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

       for (int i = 0; i < queue_size((queue_t *)prontas); i++) {
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
   return NULL;
};

// dispatcher é uma tarefa
void dispatcher_body ()  {
    task_t *next;
    while ( queue_size((queue_t *)prontas) > 0 || queue_size((queue_t *)adormecidas) > 0 ) {
        if (queue_size((queue_t *)adormecidas) > 0) {
            // printf("adormecidas possui elementos\n");
            int queueSize =  queue_size((queue_t *)adormecidas);
            task_t * nextAdormecida;
            task_t *adormecida = adormecidas;
            for (int i = 0; i < queueSize; i++) {
                nextAdormecida = adormecida;
                adormecida = adormecida->next;
                // printf("task adormecida (tid): %d awaking %d systime %d\n", adormecida->tid, adormecida->awaking, (int)systime());
                if (nextAdormecida->awaking <= (int)systime()) {
                    // printf("VAI REMOVER!!!!\n");
                    adormecida->task_state = READY;
                    queue_remove((queue_t **)&adormecidas, (queue_t *)nextAdormecida);
                    // printf("REMOVEU de adormecidas size: %d\n", queue_size((queue_t *)adormecidas));
                    queue_append((queue_t **)&prontas, (queue_t *)nextAdormecida);
                    // printf("ADICIONOU em prontas size: %d\n", queue_size((queue_t *)prontas));
                    nextAdormecida ->awaking = 0;
                }
            }
        }
        // printf("saiu do for\n");
        next = scheduler() ; // scheduler é uma função
        if (next != NULL) {
            //... // ações antes de lançar a tarefa "next", se houverem
            if (next->task_state == READY) {
                #ifdef DEBUG
                    printf("dispatcher_body: removeu task da fila id: %d\n", next->tid);
                #endif
                queue_remove((queue_t **) &prontas,(queue_t*) next);
            }
            quantum = 20;

            task_switch(next);

            #ifdef DEBUG
                printf("dispatcher_body: possui next com id: %d, removendo da fila e mudando contexto\n", next->tid);
            #endif
            if (next->task_state == READY) {
                queue_append((queue_t **) &prontas, (queue_t *) next);
                // next->task_state = READY;
            }
            //... // ações após retornar da tarefa "next", se houverem
        }
        #ifdef DEBUG
            printf("prontas size %d\n", queue_size((queue_t *)prontas));
            printf("adormecidas size %d\n", queue_size((queue_t *)adormecidas));
        #endif
    }
    printf("encerrando dispatcher\n");
    task_exit(0); // encerra a tarefa dispatcher
};

void tratador (int signum)
{
    // Incrementada a cada interrupção do temporizador (1 ms).
    ticks++;
    taskAtual->cpu_time++;
    // printf("TRATADOR\n");

    if (taskAtual->task_type == USER_TASK && taskAtual->preempcao == 1){
        if (quantum == 0) {
            #ifdef DEBUG
                printf("tratador: quantum ZERO, voltando processador para dispatcher\n");
            #endif
            task_yield();
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
    ticks = 0;
    getcontext(&contextMain);
    mainTask.next = NULL;
    mainTask.prev = NULL;
    mainTask.tid = tid;
    mainTask.context = contextMain;
    mainTask.static_priority = 0;
    mainTask.dinamic_priority = mainTask.static_priority;
    mainTask.task_type = USER_TASK;
    mainTask.task_state = READY;

    queue_append((queue_t**) &prontas, (queue_t*) &mainTask);
    task_create(&taskDispatcher, dispatcher_body, "Dispatcher");
    taskAtual = &mainTask;

    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1 ;      // primeiro disparo, em micro-segundos
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
int task_create (task_t *task,          // descritor da nova tarefa
                 void (*start_func)(void *),    // funcao corpo da tarefa
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
    task->dinamic_priority = task->static_priority;
    task->cpu_time = 0;
    task->activations = 1; // Primeira ativação.
    task->task_state = READY;

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
    taskAtual->exit_code = exitCode;
    taskAtual->task_state = TERMINATED;
    // Contabilização de tarefas. 
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations.\n", taskAtual->tid, systime(), taskAtual->cpu_time, taskAtual->activations);

    if (queue_size((queue_t*) suspensas) > 0) {
        task_resume(suspensas);
    }
    task_yield();
};

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) {
    if (task == NULL) {
        return -1;
    }
    task_t *aux;
    aux = taskAtual;
    taskAtual = task;
    taskAtual->activations++;
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
    if (task == NULL) {
        task = taskAtual;
    }

    if (task->task_state == READY && task != taskAtual){
        queue_remove((queue_t **) &prontas, (queue_t *)task);
    }

    task->task_state = SUSPENDED;
    queue_append((queue_t **) queue, (queue_t *)task);
    #ifdef DEBUG   
        printf("task_suspend: tamanho fila suspensas: %d\n", queue_size((queue_t *) suspensas));
    #endif
};

// acorda uma tarefa, retirando-a de sua fila atual, adicionando-a à fila de
// tarefas prontas ("ready queue") e mudando seu estado para "pronta"
void task_resume (task_t *task) { 
    if (task->task_state == SUSPENDED) {
        queue_remove((queue_t **) &suspensas, (queue_t*) task);
    }
    task->task_state = READY;
    queue_append((queue_t **) &prontas, (queue_t *) task);
};

// operações de escalonamento ==================================================

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield () {
    // queue_remove((queue_t **) &prontas, (queue_t *)taskAtual); //remove da fila de prontas
    // if (taskAtual != &taskDispatcher && taskAtual->task_state == RUNNING) {
        // queue_append((queue_t **) &prontas, (queue_t *)taskAtual); //adiciona em último da fila de prontas
    // }
    #ifdef DEBUG
        printf("task_yield: Mudando para task dispatcher\n");
    #endif
    task_switch(&taskDispatcher);
};

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) {
    if (task == NULL){
        taskAtual->static_priority = prio;
        taskAtual->dinamic_priority = taskAtual->static_priority;
        #ifdef DEBUG
            printf("task_setprio: taskAtual com id: %d e prioridade: %d muda para \n", taskAtual->tid, taskAtual->static_priority);
        #endif
    } else {
        task->static_priority = task->static_priority + prio;
        task->dinamic_priority = task->static_priority;
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
    if (task == NULL) {
        return -1;
    }

    if (task->task_state == TERMINATED) {
        return task->exit_code;
    } else {
        taskAtual->preempcao = 0;
        task_suspend(NULL, &suspensas);
        task_yield();
        taskAtual->preempcao = 1;
        return task->exit_code;
    }
};

// operações de gestão do tempo ================================================

// suspende a tarefa corrente por t segundos
void task_sleep (int t) {
    taskAtual->preempcao = 0;
    // printf("task going to sleep (tid): %d\n", taskAtual->tid);
    taskAtual->task_state = SLEEPING;
    queue_remove((queue_t **) &prontas, (queue_t *) taskAtual);
    queue_append((queue_t **) &adormecidas, (queue_t *) taskAtual);
    t = t *1000;
    taskAtual->awaking = systime() + t;
    taskAtual->preempcao = 1;
    task_yield();
};

// retorna o relógio atual (em milisegundos)
unsigned int systime () {
    return ticks;
};