#include <linux/kfifo.h>

#define FIFO_OK       0
#define FIFO_EMPTY   -1
#define FIFO_FULL    -2
#define FIFO_NOMEM   -3
#define FIFO_INVALID -4

#define DEFAULT_FIFO_SIZE 100

struct fifo_entry {
	int data;
};

struct fifo_device {
	struct kfifo queue;
	int max_size;
	struct mutex lock;
};

extern struct fifo_device fifo_dev;

int fifo_init(int size);
void fifo_destroy(void);
void fifo_clear(void);
int fifo_capacity(void);

int fifo_enqueue(int value);
int fifo_dequeue(void);
int fifo_peek(void);
int fifo_size(void);
int fifo_available(void);
int fifo_is_empty(void);
int fifo_is_full(void);
