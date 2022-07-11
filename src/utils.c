#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
//#include <byteswap.h>

#define BUS_WIDTH (32)


// Utility Functions for Emulator.

// Utility function to check whether a bit is 1 or 0.
// Uses bit positions from specification
bool bit(uint32_t instruction, int pos) {
  return instruction >> pos & 1;
}

// Utility function to read a portion of the 32bit instruction.
// Uses bit positions from spec.
uint32_t subByte(uint32_t instruction, int start, int numBits) {
  return (instruction >> (start - numBits + 1)) & ((1 << numBits) - 1);
}

// Utility function to read a portion of the 32bit instruction
// and outputs int in Binary representation suitable for use in switch cases.
// E.g. 12 => 1100
uint32_t subBinary(uint32_t instruction, int start, int numBits) {
  instruction = subByte(instruction, start, numBits);
  uint32_t sum = 0;
  int base = 1;
  for ( ; instruction > 0; instruction /= 2) {
    int rem = instruction % 2;
    sum += rem * base;
    base *= 10;
  }
  return sum;
}

// Shift operations.
int32_t logicalLeft(int32_t contents, uint32_t shiftAmount) {
  return contents << shiftAmount;
}

int32_t logicalRight(int32_t contents, uint32_t shiftAmount) {
  return ((uint32_t) contents >> shiftAmount);
}

int32_t arithmeticRight(int32_t contents, uint32_t shiftAmount) {
  return contents >> shiftAmount;
}

int32_t rotateRight(int32_t contents, uint32_t shiftAmount) {
  return (contents >> shiftAmount) | (contents << (BUS_WIDTH - shiftAmount));
}

int32_t rotateLeft(int32_t contents, uint32_t shiftAmount) {
  return (contents << shiftAmount) | (contents >> (BUS_WIDTH - shiftAmount));
}

bool carryOut(int32_t contents, uint32_t shiftAmount, uint32_t type) {
  // Get the carry out depending on the shift function used.
  if (type > 0) {
    return bit(contents, shiftAmount - 1);
  } else {
    return bit(contents, 31 - shiftAmount);
  }
}

// Returns the contents shifted a certain amount by a specified shift operation
uint32_t shift(int32_t contents, uint32_t shiftAmount, uint32_t type){
  int32_t (*shiftPtr[4]) (int32_t, uint32_t) = {logicalLeft, logicalRight, arithmeticRight, rotateRight};
  return (*shiftPtr[type]) (contents, shiftAmount);
}

// -----------------------------------------------------------------------------

// Utility Functions for Assembler.

// Utility function to tokenize the operands with delimeters ' ' and ','.
// Assumed that max number of operands is 6, and max length of each operand is 20.
// First element returned is the opcode (e.g. mov) followed by the operands.
void tokenize(char *instruction, char result[6][20]) {
  char buffer[strlen(instruction) + 1];
  memset(result, '\0', sizeof(char) * 6 * 20);
  strcpy(buffer, instruction);
  char *token = strtok(buffer, " ,");
  int tokenNo = 0;
  while (token != NULL) {
    strcpy(result[tokenNo], token);
    token = strtok(NULL, " ,");
    tokenNo++;
  }
}

// Utility function to setBits within each uint32_t instruction to value.
// Usage similar to subByte where you pass in the start position as indicated
// on the specifications, and the number of bits you wish to write to.
// Warning: when passing value, pass as normal int (e.g. 14) not as binary (e.g. 1110).
void setBits(uint32_t *instruction, uint32_t value, int start, int numBits) {
  *instruction &= ~(((1 << numBits) - 1) << (start + 1 - numBits));
  *instruction |= value << (start + 1 - numBits);
}


// Utility function to convert the operand string and extract the int value.
uint32_t getRegister(char *operand) {
  uint32_t registerInt = operand[1] - '0';
  if (registerInt > 16) {
    perror("Register not available");
    exit(EXIT_FAILURE);
  }
  return registerInt;
}
