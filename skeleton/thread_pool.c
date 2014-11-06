#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "thread_pool.h"
#include "semaphore.h"
/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

#define MAX_THREADS 20
#define STANDBY_SIZE 8
#define P_LEVEL 3
typedef struct pool_task{
    void (*function)(void *);
    void *argument;
	struct pool_task* next;
	struct pool_task* prev;
} pool_task_t;

struct pool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  m_sem_t* mysem;
  pool_task_t *queue;
  int thread_count;
  int task_queue_size_limit;
};

static void *thread_do_work(void *pool);

void queue_add_task(pool_task_t* queue, pool_task_t* task)
{
	if(!queue)
	{
		queue = task;
		queue->next = queue;
		queue->prev = queue;
	}
	else
	{
		task->next = queue;
		task->prev = queue->prev;
		queue->prev->next = task;
		queue->prev = task;
	}
}

pool_task_t* queue_remove_job(pool_task_t* queue)
{
	pool_task_t* task = queue;
	if(queue == queue->next)
		queue = NULL;
	else
	{
		queue = queue->next;
		queue->prev = task->prev;
		task->prev->next = queue;
	}
	return task;
}

void queue_free(pool_task_t* queue)
{
	pool_task_t* cur = queue;
	while(cur->next != cur)
	{
		cur = cur->next;
		free(cur->prev);
	}
	free(cur);
	return;
}
/*
 * Create a threadpool, initialize variables, etc
 *
 */
pool_t *pool_create(int queue_size, int num_threads)
{
    if(queue_size <= 0) queue_size = STANDBY_SIZE;
	if(num_threads <= 0) num_threads = MAX_THREADS;
	struct pool_t* pool = (struct pool_t*)malloc(sizeof(struct pool_t) + num_threads * sizeof(pthread_t));
	pool->threads = (pthread_t*)((char*)pool + sizeof(struct pool_t));
	pool->thread_count = num_threads;
	pool->task_queue_size_limit = queue_size;
	
	// Create the queue
	pool->queue = NULL;

	// Create all threads
	int i;	
	for(i = 0; i < num_threads; i++)
	{
		pthread_create((pool->threads + i), NULL, thread_do_work, (void*)pool);
	}
	// Init mutex for queue
	pthread_mutex_init(&pool->lock, NULL);
	
	// Init semaphore
	mysem_init(pool->mysem, 0);
	return NULL;
}


/*
 * Add a task to the threadpool
 *
 */
int pool_add_task(pool_t *pool, void (*function)(void *), void *argument)
{
    int err = 0;
	struct pool_task *task;
	// lock the queue
	pthread_mutex_lock(&pool->lock);
	
	// add task to the queue
	task = (struct pool_task*)malloc(sizeof(struct pool_task));
	task-> function = function;
	task-> argument = argument;
	queue_add_task(pool->queue, task);
	
	// sem_post
	mysem_post(pool->mysem);
	
	// unlock the queue
	pthread_mutex_unlock(&pool->lock);
	return err;
}



/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int pool_destroy(pool_t *pool)
{
    int err = 0;
	int i = 0;
	// destroy treads
	for (i = 0; i < pool->thread_count; i++)
	{
		pthread_cancel(*(pool->threads + i));
	}

	// destroy the semaphore and mutex
	mysem_destroy(pool->mysem);
	pthread_mutex_destroy(&pool->lock);

	// free the memory	
	queue_free(pool->queue);

	// Destroy the threadpool
	free(pool);
    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *pool)
{ 
	struct pool_t* my_pool = (struct pool_t*) pool;
	struct pool_task *task = NULL;	
	while(1)
	{
		// sem_wait
		mysem_wait(my_pool->mysem);	     
		
		// lock work queue
		pthread_mutex_lock(&my_pool->lock);	
		
		// remove job from work queue
		task = queue_remove_job(my_pool->queue);
		
		// unlock work queue
		pthread_mutex_unlock(&my_pool->lock);

		// do work
		task->function(task->argument);
		free(task);
	}

    pthread_exit(NULL);
    return NULL;
}
