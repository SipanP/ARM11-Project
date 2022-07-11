#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define LINE_LENGTH (511)

typedef struct Node {
  char key[LINE_LENGTH + 1];
  uint32_t value;
  struct Node *next;
} Node_t;

void push(Node_t *head, char *key, uint32_t value);

bool exists(Node_t *head, char *key);

uint32_t getValue(Node_t *head, char *key);

void freeTable(Node_t *head);
