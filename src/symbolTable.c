#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "symbolTable.h"

void push(Node_t *head, char *key, uint32_t value) {
  Node_t *prev = head;
  Node_t *curr = head->next;
  while(curr) {
    prev = curr;
    curr = curr->next;
  }
  curr = (Node_t *) malloc(sizeof(Node_t));
  strcpy(curr->key, key);
  curr->value = value;
  curr->next = NULL;
  prev->next = curr;
}

bool exists(Node_t *head, char *key) {
  Node_t *curr = head->next;
  while(curr) {
    if (strcmp(curr->key, key) == 0) {
      return true;
    }
    curr = curr->next;
  }
  return false;
}

uint32_t getValue(Node_t *head, char *key) {
  Node_t *curr = head->next;
  while(curr) {
    
    if (strcmp(curr->key, key) == 0) {
      return curr->value;
    }
    curr = curr->next;
  }
  return 0;
}

void freeTable(Node_t *head) {
  if(head) {
    freeTable(head->next);
    free(head);
  }
}
