#define IDLE_TASK_BASE (uint32_t *)0x200

typedef struct task_node_struct {
    struct task_node_struct *prev;
    struct task_node_struct *next;
    struct task_struct      *task;
} task_node;

typedef struct task_node_list {
    task_node   *head;
    task_node   *tail;
} task_node_list;
   
typedef struct task_struct {
    uint32_t    *stack_top;
    uint32_t    *stack_base;
    void        (*fptr)(void);
    uint32_t    period;
    uint8_t     priority;
    uint8_t     running;
} task_ctrl;

task_node *curr_task;
extern task_node * volatile task_to_free;
extern uint8_t task_switching;

void idle_task(void);
void end_task(void) __attribute__((section(".kernel"),naked));;
void kernel_task(void) __attribute__((section(".kernel")));
void start_task_switching(void) __attribute__((section(".kernel"), optimize(1)));
void switch_task(void) __attribute__((section(".kernel")));
task_ctrl *create_task(void (*fptr)(void), uint8_t priority, uint32_t period) __attribute__((section(".kernel")));
task_node *register_task(task_ctrl *task_ptr) __attribute__((section(".kernel")));
void new_task(void (*fptr)(void), uint8_t priority, uint32_t period) __attribute__((section(".kernel")));
void append_task(task_node_list *list, task_node *task) __attribute__((section(".kernel")));
void remove_task(task_node *tasknode) __attribute__((section(".kernel")));
void free_task(task_node *tasknode) __attribute__((section(".kernel")));
