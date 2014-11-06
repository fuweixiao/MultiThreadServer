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
	pthread_mutex_lock(&pool->lock);
	if(pool->queue)
	{
		task = (struct pool_task*)malloc(sizeof(struct pool_task));
		task-> function = function;
		task-> argument = argument;
		task->next = pool->queue;
		mysem_post(pool->mysem);
	}
	else
	{
		err = -1;
	}
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
		mysem_wait(my_pool->mysem);	     
		pthread_mutex_lock(&my_pool->lock);	
		pthread_mutex_unlock(&my_pool->lock);
		if(my_pool->queue)
		{
			task = my_pool->queue;
			my_pool->queue = my_pool->queue->next;
			if(my_pool->queue->prev)
				my_pool->queue->prev = NULL;
		}
		task->function(task->argument);
		free(task);
	}

    pthread_exit(NULL);
    return(NULL);
}
