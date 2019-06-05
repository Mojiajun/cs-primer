/*
 * 线程创建
 * 临界区出错
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREAD 100
#define MAX_RUN 50000000
void *thread_inc(void *arg);
void *thread_des(void *arg);
long long sum = 0;

int main(int argc, char **argv) {
  pthread_t thread_id[NUM_THREAD];
  int i;

  printf("sizeof long long: %ld\n", sizeof(long long));
  for(i = 0; i < NUM_THREAD; ++i) {
	if(i%2)
	  pthread_create(&thread_id[i], NULL, thread_inc, NULL);
	else
	  pthread_create(&thread_id[i], NULL, thread_des, NULL);
  }
  for(i = 0; i < NUM_THREAD; ++i)
	pthread_join(thread_id[i], NULL);
  printf("result: %lld\n", sum);
  return 0;
}

void *thread_inc(void *arg) {
  int i;
  for(i = 0; i < MAX_RUN; ++i)
	sum += 1;
  return NULL;
}

void *thread_des(void *arg) {
  int i;
  for(i = 0; i < MAX_RUN; ++i)
	sum -= 1;
  return NULL;
}
