struct semaphore {
    uint8_t     lock;
    task_node   *held_by;
};

typedef struct semaphore semaphore;

void spin_acquire(volatile struct semaphore *semaphore);
void acquire(volatile struct semaphore *semaphore) __attribute__((optimize(0)));
void release(volatile struct semaphore *semaphore);
void init_semaphore(volatile struct semaphore *semaphore);
