#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


typedef struct m_sem_t {
    pthread_mutex_t sem_mutex;
	pthread_cond_t sem_cond;
	int value;
} m_sem_t;

int sem_wait(m_sem_t *s);
int sem_post(m_sem_t *s);

int sem_wait(m_sem_t *s)
{
    //TODO
	pthread_mutex_lock(&s->sem_mutex);
    return 0;
}

int sem_post(m_sem_t *s)
{
    //TODO
	pthread_mutex_lock(&s->sem_mutex);
    return 0;
}
