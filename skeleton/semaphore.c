#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "semaphore.h"

int sem_wait(m_sem_t *s);
int sem_post(m_sem_t *s);

int mysem_init(m_sem_t* mysem, int value)
{
	// init semaphore
	mysem->value = value;
	pthread_mutex_init(&mysem->sem_mutex, NULL);
	pthread_cond_init(&mysem->sem_cond, NULL);
	return 0;
}
int mysem_destroy(m_sem_t* mysem)
{
	pthread_cond_destroy(&mysem->sem_cond);
	pthread_mutex_destroy(&mysem->sem_mutex);
	return 0;
}

int mysem_clean(void* arg)
{
	pthread_mutex_t *mutex = (pthread_mutex_t*)arg;
	pthread_mutex_unlock(mutex);
	return 0;
}
int mysem_wait(m_sem_t *s)
{
	pthread_mutex_lock(&s->sem_mutex);
	pthread_cleanup_push(mysem_clean, (void*)&s->sem_mutex);
	while(s->value <= 0)
	{
		pthread_cond_wait(&s->sem_cond,&s->sem_mutex);
	}
	s->value--;
	pthread_cleanup_pop(0);
	pthread_mutex_unlock(&s->sem_mutex);
	return 0;
}

int mysem_post(m_sem_t *s)
{
	pthread_mutex_lock(&s->sem_mutex);
	s->value++;
	pthread_cond_signal(&s->sem_cond);
	pthread_mutex_unlock(&s->sem_mutex);
    return 0;
}
