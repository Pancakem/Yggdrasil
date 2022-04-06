#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF_LEN 11

typedef struct Buffer {
  pthread_mutex_t mutex;
  char buff[MAX_BUF_LEN];
} buffer_t;

volatile int buffer_full;

buffer_t buffer;

void *read_buffer(void *args) {
  (void)args;
  int count = 0;
  if (buffer_full) {
    char *reader = buffer.buff;
    char read_buffer[MAX_BUF_LEN] = {0};
    while (*reader != '\0') {
      read_buffer[count] = *reader;
      count++;
      reader++;
    }
    read_buffer[count + 1] = '\0';
    printf("Received string: %s\n", read_buffer);
    if (strlen(buffer.buff) == MAX_BUF_LEN) {
      pthread_mutex_lock(&buffer.mutex);
      memset(buffer.buff, 0, MAX_BUF_LEN);
      pthread_mutex_unlock(&buffer.mutex);
      buffer_full = 0;
    }
  }

  return NULL;
}

void *write_buffer(void *arg) {
  (void)arg;
  srand(time(NULL));
  int size = 0;
  while (!buffer_full && size < MAX_BUF_LEN) {
    pthread_mutex_lock(&buffer.mutex);
    char c = (char)rand() % 128;
    if (c != '\n' || c != '\r' || c != '\t')
      buffer.buff[size] = c;
    pthread_mutex_unlock(&buffer.mutex);
    size++;
  }
  if (size > 0) {
    pthread_mutex_lock(&buffer.mutex);
    buffer.buff[size + 1] = '\0';
    if (strlen(buffer.buff) == MAX_BUF_LEN)
      buffer_full = 1;
    pthread_mutex_unlock(&buffer.mutex);
    printf("Sent string: %s\n", buffer.buff);
  }

  return NULL;
}

int main(void) {
  pthread_mutex_init(&buffer.mutex, NULL);
  buffer_full = 0;
  while (1) {
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, write_buffer, NULL);
    pthread_create(&tid2, NULL, read_buffer, NULL);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    sleep(2);
  }
  // pthread_mutex_destroy(&buffer.mutex);
  return 0;
}
