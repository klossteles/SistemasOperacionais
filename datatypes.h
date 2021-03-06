// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__

#include <ucontext.h>

// Estrutura que define uma tarefa
typedef struct task_t
{
  struct task_t *prev, *next; // para usar com a biblioteca de filas (cast)
  ucontext_t context;
  int tid; // ID da tarefa
  int static_priority;
  int dinamic_priority;
  int task_type;
  int cpu_time;
  int activations;
  int task_state;
  int exit_code;
  int awaking;
  int preempcao;
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  struct task_t *fila;
  int contador; 
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
