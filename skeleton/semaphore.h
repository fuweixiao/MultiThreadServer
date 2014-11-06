#ifndef __SEM_H__
#define __SEM_H__

#include <pthread.h>

typedef struct m_sem_t {
    pthread_mutex_t sem_mutex;
	pthread_cond_t sem_cond;
	int value;
} m_sem_t;
int mysem_init(struct m_sem_t *sem, int value);

int mysem_wait(struct m_sem_t *sem);

int mysem_post(struct m_sem_t *sem);

int mysem_destroy(struct m_sem_t *sem);

#endif
