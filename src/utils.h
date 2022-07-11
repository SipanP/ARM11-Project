#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Utility Functions for Emulator.

bool bit(uint32_t instruction, int pos);

uint32_t subByte(uint32_t instruction, int start, int numBits);

uint32_t subBinary(uint32_t instruction, int start, int numBits);

int32_t logicalLeft(int32_t contents, uint32_t shiftAmount);

int32_t logicalRight(int32_t contents, uint32_t shiftAmount);

int32_t arithmeticRight(int32_t contents, uint32_t shiftAmount);

int32_t rotateRight(int32_t contents, uint32_t shiftAmount);

int32_t rotateLeft(int32_t contents, uint32_t shiftAmount);

bool carryOut(int32_t contents, uint32_t shiftAmount, uint32_t type);

uint32_t shift(int32_t contents, uint32_t shiftAmount, uint32_t type);

// -----------------------------------------------------------------------------

// Utility Functions for Assembler.

void tokenize(char *instruction, char result[6][20]);

void setBits(uint32_t *instruction, uint32_t value, int start, int numBits);

uint32_t getRegister(char *operand);