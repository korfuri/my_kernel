#ifndef MUTEX_H_
#define MUTEX_H_

#define MUTEX_BUSY (1)
#define MUTEX_FREE (0)

#define MUTEX_INIT (MUTEX_FREE)

typedef unsigned long mutex_t;

void mutex_lock(mutex_t*);
void mutex_unlock(mutex_t*);

// Returns 0 and locks the mutex if it is not locked, nonzero
// otherwise
int mutex_trylock(mutex_t*);

#endif
