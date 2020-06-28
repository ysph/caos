#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdatomic.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define THREADS 100
#define ELEMENTS 1000

typedef struct Data {
  int i;
  int k;
} DataT;

typedef struct Item {
   struct Item* next;
   int64_t value;
} ItemT;

typedef struct List {
  _Atomic (ItemT*) head;
} List;

List list;
void insertListItem(_Atomic (ItemT*) item) {
  ItemT* prev_head = atomic_load(&list.head);

  item->next = prev_head;
  while (!atomic_compare_exchange_strong(&list.head, &prev_head, item)) {
    item->next = prev_head;
  }
}

void* func (void* args) {
  int i = ((DataT *)args)->i;
  int k = ((DataT *)args)->k;

  for (int val = i*k; val < (i+1)*k; val++) {
    _Atomic (ItemT*) item = calloc(1, sizeof(ItemT));
    item->value = val;
    while (val != list.head->value + 1 && list.head->value != -1) {
      sched_yield();
    }
    insertListItem(item);
  }

  return 0;
}

int main(int argc, const char * argv[]) {
  uint64_t N = THREADS;
  uint64_t k = ELEMENTS;

  pthread_t *threads = calloc(N, sizeof(pthread_t));

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
  pthread_attr_setguardsize(&attr, 0);

  _Atomic (ItemT*) item = calloc(1, sizeof(ItemT));
  item->value = -1;
  list.head = item;
  item->next = NULL;

  for (int i = 0; i < N; i++) {
    DataT* data = calloc(1, sizeof(DataT*));
    data->i = i;
    data->k = (int)k;
    pthread_create(&threads[i], &attr, func, (void *)data);
  }

  for (int i = 0; i < N; ++i) {
    pthread_join(threads[i], NULL);
  }

  ItemT* no_atomic_head = atomic_load(&list.head);

  int64_t* result = calloc(N*k, sizeof(int64_t));
  int64_t i = 0;
  while (no_atomic_head->next != NULL) {
    result[i] = no_atomic_head->value;
    i++;
    no_atomic_head = no_atomic_head->next;
  }

  while (i--) {
    printf("%lld ", result[i]);
  }

  return 0;
}
