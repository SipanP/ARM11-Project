#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "symbolTable.h"
#include "utils.h"

#define MEMORY_CAPACITY (16384)
#define LINE_LENGTH (511)

struct State {
  char input[MEMORY_CAPACITY][LINE_LENGTH + 1];
  uint32_t output[MEMORY_CAPACITY];
  Node_t *symbolTable;
  int endOfProgram;
 } state;

void readFile(char *fileName) {
  FILE *fp;
  // Check if user has given a valid file path to program,
  // if they have, open it as a readable file.
  fp = fopen(fileName, "r");
  if(fp == NULL) {
    perror("Error opening the assembly file!\n");
    exit(EXIT_FAILURE);
  }

  char buffer[LINE_LENGTH + 1];
  int lineNo = 0;

  // Read everything in the file into the state input memory.
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    buffer[strlen(buffer) - 1] = '\0';
    strcpy(state.input[lineNo], buffer);
    lineNo++;
  }
  state.input[lineNo][0] = '\0';
  if(ferror(fp)){
    perror("Error reading from stream.\n");
  }
  fclose(fp);
}

void writeFile(char *fileName) {
  FILE *fp;
  // Check if user has given a valid file path to program,
  // if they have, open it as a readable file.
  fp = fopen(fileName, "wb");
  if(fp == NULL) {
    perror("Error opening the binary file!\n");
    exit(EXIT_FAILURE);
  }

  fwrite(state.output, sizeof(uint32_t), state.endOfProgram, fp);
  if(ferror(fp)){
    perror("Error reading from stream.\n");
  }
  fclose(fp);
}


void firstPass(void) {
  int lineNo = 0;
  state.endOfProgram = 0;
  while(state.input[lineNo][0] != '\0') {
    if (strchr(state.input[lineNo], ':')) {
      char label[strlen(state.input[lineNo]) + 1];
      strcpy(label, state.input[lineNo]);
      push(state.symbolTable, strtok(label, ":"), state.endOfProgram * 4);
      state.endOfProgram--;
    }
    lineNo++;
    state.endOfProgram++;
  }
}

void dataProcessing(int instNo, char operands[6][20]) {
  uint32_t instBinary = 0;
  char dollarOpcode[4] = "$";
  strcat(dollarOpcode, operands[0]);
  uint32_t opcode = getValue(state.symbolTable, dollarOpcode);
  int op2Size = 2;

  if (opcode >= 8 && opcode <= 10) {
    // For instructions that compute results
    // Set S bit
    setBits(&instBinary, 1, 20, 1);
    
    // Set Rn
    setBits(&instBinary, getRegister(operands[1]), 19, 4);
  } else if (opcode == 13) {
    // For single operand assignment
    // Set Rd
    setBits(&instBinary, getRegister(operands[1]), 15, 4);
  } else {
    // For instructions that do not compute results
    op2Size = 3;

    // Set Rd
    setBits(&instBinary, getRegister(operands[1]), 15, 4);

    // Set Rn
    setBits(&instBinary, getRegister(operands[2]), 19, 4);
  }

  if (operands[op2Size][0] == '#') {
    // Operand is an expression
    uint32_t operand2 = strtol(&operands[op2Size][1], NULL, 0);
    bool representable = false;
    uint32_t newRepresentation = 0;
    int rotate = 0;

    for ( ; rotate <= 30; rotate += 2) {
      newRepresentation = rotateLeft(operand2, rotate);
      // Check if the constant can be represented
      if (newRepresentation == (newRepresentation & ((1 << 8) - 1))) {
        representable = true;
        break;
      }
    }

    if (!representable) {
      perror("Immediate constant cannot be represented\n");
      exit(EXIT_FAILURE);
    }

    // Set I bit
    setBits(&instBinary, 1, 25, 1);

    // Set Immediate
    setBits(&instBinary, newRepresentation, 7, 8);

    // Set Rotate
    setBits(&instBinary, rotate / 2, 11, 4);

  } else {
    // Operand is a shifted register
    // Set Rm
    setBits(&instBinary, getRegister(operands[op2Size]), 3, 4);

    // Set shift type
    if (exists(state.symbolTable, operands[op2Size+1])) {
      setBits(&instBinary, getValue(state.symbolTable, operands[op2Size+1]), 6, 2);
    
      if (operands[op2Size+2][0] == '#') {
        // Shift by a constant
        // Set integer
        setBits(&instBinary, strtol(&operands[op2Size+2][1], NULL, 0), 11, 5);
      } else {
        // Shift by a register
        setBits(&instBinary, 1, 4, 1);

        // Set register
        setBits(&instBinary, getRegister(operands[op2Size+2]), 11, 4);
      }
    }
  }

  // Set Cond bits
  setBits(&instBinary, 14, 31, 4);

  // Set Opcode bits
  setBits(&instBinary, opcode, 24, 4);

  state.output[instNo] = instBinary;
}

void multiply(int instNo, char operands[6][20]) {
  uint32_t instBinary = 0;

  // Set conditional field
  setBits(&instBinary, 14, 31, 4);

  // Set bits 4-7 to 1001
  setBits(&instBinary, 9, 7, 4);

  // Add Rd to instruction
  setBits(&instBinary, getRegister(operands[1]), 19, 4);

  // Add Rm to instruction
  setBits(&instBinary, getRegister(operands[2]), 3, 4);

  // Add Rs to instruction
  setBits(&instBinary, getRegister(operands[3]), 11, 4);

  // Set A bit to 1 and add Rm, if instruction is mla
  if (strcmp("mla", operands[0]) == 0) {
    // Set A bit
    setBits(&instBinary, 1, 21, 1);
    // Add Rn to instruction
    setBits(&instBinary, getRegister(operands[4]), 15, 4);
  }

  state.output[instNo] = instBinary;

}

// Given a number that has been tokenized, extract it and set flags
// about whether it is a register, and the sign
uint32_t getNumber(char *expression, bool *isRegister, bool *sign) {
  char *endPtr = strchr(expression, ']');
  if(endPtr) {
    // Remove square bracket
    expression[(int) (endPtr - expression)] = '\0';
  }

  if(strchr(expression, '-')) {
    *sign = false;
  } else {
    *sign = true;
  }

  if(strchr(expression, 'r')) {
    *isRegister = true;
    return getRegister(&expression[((!*sign) ? 1 : 0)]); 
  }

  *isRegister = false;

  // Shift by 1 due to #
  int32_t num = strtol(&expression[1], NULL, 0);
  return abs(num);
}

// Given a multiplier encode expression into binary
int32_t decodeMultiplicand(char expression[2][20], uint32_t num) {
  char *command = expression[0];
  bool a, b;
  int32_t additionalOffset = getNumber(expression[1], &a, &b);

  // Return binary rep of instr + binary rep of additionalOffset + binary rep of num
  uint32_t offset = 0;
  
  if (exists(state.symbolTable, command)) {
    setBits(&offset, getValue(state.symbolTable, command), 11, 4);
  }
 
  setBits(&offset, additionalOffset, 7, 4);
  setBits(&offset, num, 3 , 4);
  return offset;
}

// Translate instruction from single data processing format into data processing format
void translateDataTransferToDataProcessing(char operands[6][20], uint32_t offset) {
  memcpy(&operands[0], "mov", 20);
  char expr[20];
  strcpy(expr, "#");
  snprintf(&expr[1], sizeof(expr), "%d", offset);
  memcpy(&operands[2], expr, 20);
}

// Calculate the offset value based on its string representation.
// Sets flags to say whether it is an:
// I: Immediate offset: (shifted register / unsigned 12 bit immediate offset)
// P: Pre/post indexing
// U: Up bit: added to base reg / subtracted from base reg
void calculateOffsetValue(char expression[4][20], int32_t *Rn, uint32_t *offset, bool *I, bool *P, bool *U) {
  if(expression[0][0] == '=') { // Constant of the form <=expression>
    *offset = getNumber(expression[0], I, U);
    *P = true; // Add to base register before transferring data
    return;
  }

  // Find index of ending square bracket
  int range = 0;
  for(; !strchr(expression[range], ']'); range++);
  
  // The first argument is always going to be a register
  *Rn = getNumber(&expression[0][1], I, U); // Offset by 1 due to '['

  // Set flags
  *I = false;
  *P = true;

  int preIndexedOffset = 0;
  int postIndexedOffset = 0;
  
  // Calculate offsets
  if(expression[1][0] != '\0') {
    if(1 <= range) { // Still within the brackets
      preIndexedOffset = getNumber(expression[1], I, U);
    } else { // Outside the brackets
      postIndexedOffset = getNumber(expression[1], I, U);
      *P = false;
    }
  }
  if(expression[2][0] != '\0') {
    if(2 <= range) { // if its still within the brackets
      preIndexedOffset = decodeMultiplicand(&expression[2], preIndexedOffset);
    } else { // if it is outside the brackets
      postIndexedOffset = getNumber(expression[2], I, U);
    }
  }

  *offset = preIndexedOffset + postIndexedOffset;
}

void singleDataTransfer(int instNo, char operands[6][20]) {
  // Initialize registers
  int32_t Rn = -1;
  int32_t Rd = getRegister(operands[1]);

  // Decode instruction type:
  bool L; // Memory to register
  if(strcmp(operands[0], "ldr") == 0) {
    L = true;
  } else {
    L = false; // i.e. register to memory
  }

  bool I, P, U;
  uint32_t offset = 0;

  calculateOffsetValue(&operands[2], &Rn, &offset, &I, &P, &U);

  if(offset < 0xff && L && Rn == -1) {
    // ldr is used as a mov instruction
    translateDataTransferToDataProcessing(operands, offset);
    dataProcessing(instNo, operands);
    return;
  } else if (L && offset >= 0xffffff) {
    // The assembler should put the value of offset in four bytes at the end of the assembled program
    // and use the address of this value with the PC as the base register and a calculated offset

    // Save data at the end of assembled program
    Rn = 15; // PC
    U = true;
    state.output[state.endOfProgram] = offset;
    offset = (state.endOfProgram - instNo) * 4 - 8;
    state.endOfProgram++;
  }

  // Set bits
  uint32_t instBinary = 0;

  // Set conditional field
  setBits(&instBinary, 14, 31, 4);

  // Set bits 27 - 26 to 01
  setBits(&instBinary, 1, 27, 2);

  // Set I bit
  setBits(&instBinary, I, 25, 1);

  // Set P bit
  setBits(&instBinary, P, 24, 1);

  // Set U bit
  setBits(&instBinary, U, 23, 1);

  // Set bits 22 - 21 to 0
  setBits(&instBinary, 0, 22, 2);

  // Set L bit
  setBits(&instBinary, L, 20, 1);

  // Set bits 19 - 16 to Rn
  setBits(&instBinary, Rn, 19, 4);

  // Set bits 15 - 12 to Rd
  setBits(&instBinary, Rd, 15, 4);

  // Set bits 11 - 0 to offset
  setBits(&instBinary, offset, 11, 12);  

  state.output[instNo] = instBinary;
}

void branch(int instNo, char operands[6][20]) {
  // Write condition code to instruction
  if (strcmp(operands[0], "b") == 0) {
    setBits(&state.output[instNo], getValue(state.symbolTable, "al"), 31, 4);
  } else {
    char cond[3];
    strncpy(cond, &operands[0][1], 2);
    cond[2] = '\0';
    setBits(&state.output[instNo], getValue(state.symbolTable, cond), 31, 4);
  }

  // Set constant bits for all branch instruction
  setBits(&state.output[instNo], 10, 27, 4); // 1010

  // Calculate branch offset
  int32_t offset = getValue(state.symbolTable, operands[1]) - instNo * 4 - 8;
  offset = (offset >> 2) & ((1 << 24) - 1);
  setBits(&state.output[instNo], offset, 23, 24);
}

void special(int instNo, char operands[6][20]) {
  if (strcmp(operands[0], "andeq") == 0) {
    // andeq termination instruction
    state.output[instNo] = 0;
  } else {
    // lsl logical left shift operation passed to data processing
    strcpy(operands[4], operands[2]);
    strcpy(operands[3], operands[0]);
    strcpy(operands[2], operands[1]);
    strcpy(operands[0], "mov");
    dataProcessing(instNo, operands);
  }
}

void pushOpcodes() {
  // Push opcodes into symbol table for later reference by function pointers.
  push(state.symbolTable, "lsl", 0);
  push(state.symbolTable, "andeq", 0);
  push(state.symbolTable, "mul", 1);
  push(state.symbolTable, "mla", 1);
  push(state.symbolTable, "ldr", 2);
  push(state.symbolTable, "str", 2);

  // Push data processing opcodes into symbol table.
  push(state.symbolTable, "$and", 0); // 0000
  push(state.symbolTable, "$eor", 1); // 0001
  push(state.symbolTable, "$sub", 2); // 0010
  push(state.symbolTable, "$rsb", 3); // 0011
  push(state.symbolTable, "$add", 4); // 0100
  push(state.symbolTable, "$orr", 12); // 1100
  push(state.symbolTable, "$mov", 13); // 1101
  push(state.symbolTable, "$tst", 8); // 1000
  push(state.symbolTable, "$teq", 9); // 1001
  push(state.symbolTable, "$cmp", 10); // 1010

  // Storing shift type into symbol table.
  // push(state.symbolTable, "lsl", 0); // 0000 (Already in)
  push(state.symbolTable, "lsr", 1); // 0001
  push(state.symbolTable, "asr", 8); // 1000
  push(state.symbolTable, "ror", 9); // 1001

  // Storing condition codes in symbol table for function pointers.
  push(state.symbolTable, "eq", 0); // 0000
  push(state.symbolTable, "ne", 1); // 0001
  push(state.symbolTable, "ge", 10); // 1010
  push(state.symbolTable, "lt", 11); // 1011
  push(state.symbolTable, "gt", 12); // 1100
  push(state.symbolTable, "le", 13); // 1101
  push(state.symbolTable, "al", 14); // 1110
}

void secondPass(void) {
  // Storing opcodes and condition codes in symbol table for function pointers.
  pushOpcodes();

  int lineNo = 0;
  int instNo = 0;
  while(state.input[lineNo][0] != '\0') {
    // Check not label.
    if (strchr(state.input[lineNo], ':')) {
      lineNo++;
      continue;
    }

    // Assumed that max number of operands is 6, and max length of each operand is 20.
    char operands[6][20];
    tokenize(state.input[lineNo], operands);

    // Function pointers to instruction types with only 2 possible opcode.
    void (*instructionType[3])(int, char[5][20]) = {special, multiply, singleDataTransfer};
    
    if (exists(state.symbolTable, operands[0])) {
      instructionType[getValue(state.symbolTable, operands[0])](instNo, operands);
    } else if (operands[0][0] == 'b') {
      branch(instNo, operands);
    } else {
      dataProcessing(instNo, operands);
    }

    instNo++;
    lineNo++;
  }
}


int main(int argc, char **argv) {
  // Check that the user has entered both arguments.
  if(argc != 3) {
    perror("You must provide one input and one output file.\n");
    exit(EXIT_FAILURE);
  }

  readFile(argv[1]);

  // Initialize the head node of symbol table. 
  // Head will not contain any key value pair, only pointer to next node.
  state.symbolTable = (Node_t *) malloc(sizeof(Node_t));

  firstPass();

  secondPass();

  // Output binary file name.
  char *outputFileName = argv[2];
  writeFile(outputFileName);

  // Free symbol table.
  freeTable(state.symbolTable);

  return EXIT_SUCCESS;
}
