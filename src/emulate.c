#include <byteswap.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MEMORY_CAPACITY (16384)

// Enum for specifying which instruction type to execute on next cycle.
enum decodeType {
  DataProcessing,
  Multiply,
  SingleDataTransfer,
  Branch,
  Terminate
};

// Structure to define the state of the ARM machine and
// represent the memory, registers, and instructions
// to decode and execute on the next cycle along with the decoded type.
struct State {
  uint32_t memory[16384];
  uint32_t registers[17];
  uint32_t toDecode;
  uint32_t toExecute;
  enum decodeType decodedType;
} state;

void readFile(int argc, char *argv[]) {
  // Check that the user has entered an argument.
  if (argc != 2) {
    perror("No binary file provided.\n");
    exit(EXIT_FAILURE);
  }

  FILE *fp;
  // Check if user has given a valid file path to program,
  // if they have, open it as a readable binary file.
  fp = fopen(argv[1], "rb");
  if (fp == NULL) {
    perror("Error opening the binary file!\n");
    exit(EXIT_FAILURE);
  }

  // Read everything in the file into the state memory.
  fread(state.memory, sizeof(state.memory), 1, fp);
  if (ferror(fp)) {
    perror("Error reading from stream.\n");
  }
  fclose(fp);
}

// Fetch instruction from PC (r15).
uint32_t fetch(void) {
  int PC = state.registers[15] / 4;
  return state.memory[PC];
}

void termination(void) {
  // Output register states in decimal and hex aligned properly.
  printf("Registers:\n");
  for (int i = 0; i < 13; i++) {
    printf("$%-3d: %10d (0x%08x)\n", i, state.registers[i], state.registers[i]);
  }
  printf("PC  : %10d (0x%08x)\n", state.registers[15], state.registers[15]);
  printf("CPSR: %10d (0x%08x)\n", state.registers[16], state.registers[16]);

  // Output Non-zero memory in hex in little endian format.
  printf("Non-zero memory:\n");
  for (int i = 0; i < MEMORY_CAPACITY; i++) {
    if (state.memory[i] != 0) {
      printf("0x%08x: 0x%08x\n", i * 4, bswap_32(state.memory[i]));
    }
  }
}

enum decodeType decode(void) {
  if (state.toDecode == 0) {
    return Terminate;
  } else if (bit(state.toDecode, 27)) {
    return Branch;
  } else if (bit(state.toDecode, 26)) {
    return SingleDataTransfer;
  } else if (subBinary(state.toDecode, 27, 6) == 0 &&
             subBinary(state.toDecode, 7, 4) == 1001) {
    return Multiply;
  } else {
    return DataProcessing;
  }
}

// Utility function to decide whether CPSR register passes condition
bool cond(uint32_t instruction) {
  uint32_t cpsr = state.registers[16];
  bool v = bit(cpsr, 28);
  bool z = bit(cpsr, 30);
  bool n = bit(cpsr, 31);
  uint32_t flag = subBinary(instruction, 31, 4);

  switch (flag) {
    case 0:
      return z;
    case 1:
      return !z;
    case 1010:
      return n == v;
    case 1011:
      return n != v;
    case 1100:
      return !z && (n == v);
    case 1101:
      return z || (n != v);
    case 1110:
      return true;
    default:
      return false;
  }
}

//  Updates N, Z and C bits of CPSR according to previous operation
void setCPSR(uint32_t result, int cFlag) {
  int nFlag = bit(result, 31);
  int zFlag = result == 0;
  uint32_t newCPSR = state.registers[16];
  newCPSR = (newCPSR & 0x7fffffff) | (nFlag << 31);
  newCPSR = (newCPSR & 0xbfffffff) | (zFlag << 30);
  newCPSR = (newCPSR & 0xdfffffff) | (cFlag << 29);
  state.registers[16] = newCPSR;
}

// Returns the result of logical operations, updating CPSR if required
uint32_t aluLogic(uint32_t result, bool set, bool carry) {
  if (set) {
    setCPSR(result, carry);
  }
  return result;
}

// Returns the result of an addition, updating CPSR if required
uint32_t aluAdd(int32_t op1, int32_t op2, bool set) {
  int32_t result = op1 + op2;
  bool carry =
      (result < 0 && op1 > 0 && op2 > 0) || (result > 0 && op1 < 0 && op2 < 0);
  if (set) {
    setCPSR(result, carry);
  }
  return result;
}

// Returns the result of a subtraction, updating CPSR if required
uint32_t aluSub(int32_t op1, int32_t op2, bool set) {
  int32_t result = op1 - op2;
  bool carry = op1 >= op2;
  if (set) {
    setCPSR(result, carry);
  }
  return result;
}

// Performs arithmetic/logic operations based on opcode
void alu(uint32_t opCode, uint32_t op1, uint32_t op2, uint32_t destReg,
         bool set, bool carry) {
  switch (opCode) {
    case 0: {  // and
      state.registers[destReg] = aluLogic(op1 & op2, set, carry);
      break;
    }
    case 1: {  // eor
      state.registers[destReg] = aluLogic(op1 ^ op2, set, carry);
      break;
    }
    case 10: {  // sub
      state.registers[destReg] = aluSub(op1, op2, set);
      break;
    }
    case 11: {  // rsb
      state.registers[destReg] = aluSub(op2, op1, set);
      break;
    }
    case 100: {  // add
      state.registers[destReg] = aluAdd(op1, op2, set);
      break;
    }
    case 1000: {  // tst
      aluLogic(op1 & op2, set, carry);
      break;
    }
    case 1001: {  // teq
      aluLogic(op1 ^ op2, set, carry);
      break;
    }
    case 1010: {  // cmp
      aluSub(op1, op2, set);
      break;
    }
    case 1100: {  // orr
      state.registers[destReg] = aluLogic(op1 | op2, set, carry);
      break;
    }
    case 1101: {  // mov
      state.registers[destReg] = op2;
    }
  }
}

void dataProcessing(void) {
  // Gets the Immediate Operand & Set Condition Code bits
  bool i = bit(state.toExecute, 25);
  bool s = bit(state.toExecute, 20);

  // Gets the Opcode, Operand1 and Destination Register
  uint32_t opCode = subBinary(state.toExecute, 24, 4);
  uint32_t op1 = state.registers[subByte(state.toExecute, 19, 4)];
  uint32_t regd = subByte(state.toExecute, 15, 4);

  uint32_t contents, shiftAmount, shiftType;

  if (i) {  // Operand2 is an immediate value
    contents = subByte(state.toExecute, 7, 8);
    shiftAmount = 2 * subByte(state.toExecute, 11, 4);
    shiftType = 3;
  } else {  // Operand2 is a register
    // Gets contents of Register M
    contents = state.registers[subByte(state.toExecute, 3, 4)];
    shiftType = subByte(state.toExecute, 6, 2);

    if (bit(state.toExecute,
            4)) {  // Shift Register M by first byte stored in Register S
      uint32_t regsVal = state.registers[subByte(state.toExecute, 11, 4)];
      shiftAmount = subByte(regsVal, 7, 8);
    } else {  // Shift Register M by a constant amount
      shiftAmount = subByte(state.toExecute, 11, 5);
    }
  }

  // Gets the shifted Operand2 and 'barrel shifter' Carry bit
  uint32_t op2 = shift(contents, shiftAmount, shiftType);
  bool c = carryOut(contents, shiftAmount, shiftType);

  // Performs specified operation on operands
  alu(opCode, op1, op2, regd, s, c);
}

void multiply(void) {
  // Gets the index of the registers based on the instruction.
  uint32_t regd = subByte(state.toExecute, 19, 4);
  uint32_t regm = subByte(state.toExecute, 3, 4);
  uint32_t regs = subByte(state.toExecute, 11, 4);

  // If accumulate is set then multiply and accumulate
  // else just multiply.
  state.registers[regd] = state.registers[regm] * state.registers[regs];
  if (bit(state.toExecute, 21)) {
    uint32_t regn = subByte(state.toExecute, 15, 4);
    state.registers[regd] += state.registers[regn];
  }
  setCPSR(state.registers[regd], bit(state.registers[16], 29));
}

// Utility function to access 4 bytes from memory at given address.
uint32_t access(uint32_t address) {
  return *(int *)(((char *)&state.memory) + address);
}

// Utility function to store 4 bytes of data to memory at given address.
void store(uint32_t address, uint32_t data) {
  memcpy(((char *)&state.memory) + address, &data, 4);
}

bool checkMemoryInBounds(uint32_t address) {
  if (address / 4 <= MEMORY_CAPACITY) {
    return true;
  } else {
    printf("Error: Out of bounds memory access at address 0x%08x\n", address);
    return false;
  }
}

void transferData(bool mode, uint32_t source, uint32_t destination,
                  int32_t offset) {
  // given a mode it either:
  // true: loads the word from memory
  // false: stores into memory
  if (mode) {
    // the word is loaded from memory
    // check for valid memory range
    uint32_t address = state.registers[source] + offset;
    if (checkMemoryInBounds(address)) {
      state.registers[destination] = access(address);
    }
  } else {
    // the word is stored into memory
    if (checkMemoryInBounds((state.registers[source] + offset) / 4)) {
      // state.memory[(state.registers[source] + offset) / 4] =
      // state.registers[destination];
      store(state.registers[source] + offset, state.registers[destination]);
    }
  }
}

int getShiftAmount(uint32_t instruction) {
  uint32_t shiftType = subByte(instruction, 6, 2);
  uint32_t regm = subByte(instruction, 3, 4);
  uint32_t shiftAmount;

  if (bit(instruction, 4)) {
    //  Shift Register M by a value stored in a register
    uint32_t regs = subByte(instruction, 11, 4);
    uint32_t regsValue = state.registers[regs];
    shiftAmount = subByte(regsValue, 7, 8);
  } else {
    //  Shift Register M by a constant amount
    shiftAmount = subByte(instruction, 11, 5);
  }

  uint32_t regmVal = state.registers[regm];
  return shift(regmVal, shiftAmount, shiftType);
}

void singleDataTransfer() {
  // Pre: the condition has been met and the current instruction
  //      has been identified by the parent as a singleDataTransfer
  //      as well as wellfoundness of the command

  bool I = bit(state.toExecute, 25);
  bool P = bit(state.toExecute, 24);
  bool U = bit(state.toExecute, 23);
  bool L = bit(state.toExecute, 20);

  uint32_t Rn = subByte(state.toExecute, 19, 4);
  uint32_t Rd = subByte(state.toExecute, 15, 4);

  uint32_t offset = subByte(state.toExecute, 11, 12);

  if (I) {
    // Offset is interpreted as a shifted register
    offset = getShiftAmount(state.toExecute);
  }

  // NB: pre indexing will not change the value of the base register, however,
  // post-indexing will change the contents of the base register by the offset
  if (P) {
    // (pre - indexing) the offset is added/subtracted to the base register
    // before transferring the data
    transferData(L, Rn, Rd, (U ? 1 : -1) * offset);
  } else {
    // the offset is added/subtracted to the base register after transferring.
    transferData(L, Rn, Rd, 0);
    state.registers[Rn] += (U ? 1 : -1) * offset;
  }
}

void branch(void) {
  int32_t offset = subByte(state.toExecute, 23, 24) << 2;
  offset |= bit(state.toExecute, 23) * 0xfc000000;
  state.registers[15] += offset;
}

void execute(void) {
  // Delegate to each execution function depending on decodedType.
  if (cond(state.toExecute)) {
    void (*instructionType[5])(void) = {
        dataProcessing, multiply, singleDataTransfer, branch, termination};
    instructionType[state.decodedType]();
  }
}

int main(int argc, char *argv[]) {
  // Initialise state pointers to null
  state.toDecode = 0xffffffff;
  state.toExecute = 0xffffffff;

  // Read binary file to state memory
  readFile(argc, argv);

  // Process next cycle until termination
  while (state.decodedType != Terminate) {
    // Fetch Stage
    uint32_t newFetched = fetch();
    // Decode Stage
    enum decodeType newDecodedType = 0;
    if (state.toDecode != 0xffffffff) {
      newDecodedType = decode();
    }
    // Execute Stage
    if (state.toExecute != 0xffffffff) {
      execute();
    }

    // Update state values for next cycle and free executed instruction string.
    // Clear fetch decode pipeline if branch instruction is executed.
    if (state.decodedType == Branch && cond(state.toExecute)) {
      state.toDecode = state.toExecute = 0xffffffff;
      state.decodedType = 0;
    } else {
      state.toExecute = state.toDecode;
      state.toDecode = newFetched;
      state.decodedType = newDecodedType;
      // Increment PC by 4 only if not branch.
      state.registers[15] += 4;
    }
  }

  termination();
  return EXIT_SUCCESS;
}
