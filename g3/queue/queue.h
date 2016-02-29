#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_UNDERFLOW 1
#define QUEUE_OVERFLOW  2

struct node {
  int pri;
};

struct queue {
  struct node *root;
  struct node *next;
  size_t count;
  size_t size;
};

int queue_init(struct queue *queue);

int queue_push(struct queue *queue, int pri);

int queue_pop(struct queue *queue, int *pri_ptr);

int queue_destroy(struct queue *queue);

#endif // QUEUE_H
