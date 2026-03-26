#include <algorithm>
#include <bitset>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define BUFFER_PADDING 8
#define INSTRUCTION_DEST_SIZE 128
#define MEM_BLOCK_SIZE 32

#define OP_MOV_REG_MEM 0b10001000
#define OP_MOV_IMMEDIATE_TO_REG_MEM 0b11000110
#define OP_MOV_IMMEDIATE_TO_REG 0b10110000
#define OP_MOV_MEMORY_ACCUMULATOR 0b10100000

#define OP_MODE_MEMORY_MODE_NO_DISP 0b00
#define OP_MODE_MEMORY_MODE_BYTE_DISP 0b01
#define OP_MODE_MEMORY_MODE_WORD_DISP 0b10
#define OP_MODE_REGISTER_MODE 0b11

union OpCodeByte {
  uint8_t byte;
  struct {
    uint8_t width : 1;
    uint8_t direction : 1;
    uint8_t code : 6;
  } f1;
  struct {
    uint8_t reg_mem : 3;
    uint8_t reg : 3;
    uint8_t mode : 2;
  } f2;
};
typedef uint8_t(OpCodeHandler)(char *, OpCodeByte, uint8_t *);
OpCodeHandler *opcode_to_handler_mapping[256];

const char *register_array[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
const char *register_array_wide[] = {"ax", "cx", "dx", "bx",
                                     "sp", "bp", "si", "di"};
const char *address_calculation_array[] = {
    "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};

inline void byte_c_str(uint8_t val, char *target) {
  strcpy(target, std::bitset<8>(val).to_string().c_str());
}

void print_bytes(uint8_t *bytes, uint32_t size, uint8_t column_width) {
  for (uint32_t i = 0; i < size; i++) {
    printf("%s", std::bitset<8>(bytes[i]).to_string().c_str());
    if (!((i + 1) % column_width)) {
      printf("\n");
    } else {
      printf(" ");
    }
  }
  if (size % column_width) {
    printf("\n");
  }
}
void print_bytes(uint8_t *bytes, uint32_t size) { print_bytes(bytes, size, 8); }

uint8_t unknown_opcode_handler(char *instruction_dest, OpCodeByte opcode,
                               uint8_t *bytecode_stream) {
  printf("Uknown opcode: %b\n", opcode.byte);
  return 0;
}

inline uint8_t get_mem_block(char *mem_block_dest, OpCodeByte opcode,
                             OpCodeByte opcode_byte2,
                             uint8_t *bytecode_stream) {
  uint8_t bytes_decoded_count = 0;
  int16_t displacement = 0;
  bool has_displacement =
      opcode_byte2.f2.mode == 0b01 || opcode_byte2.f2.mode == 0b10;
  bool is_direct_access =
      opcode_byte2.f2.mode == 0 && opcode_byte2.f2.reg_mem == 0b110;
  if (has_displacement || is_direct_access) {
    displacement = *bytecode_stream++;
    bytes_decoded_count++;
    // if (opcode.f1.width) {
    if (opcode_byte2.f2.mode == 0b10 || (is_direct_access && opcode.f1.width)) {
      displacement = (*bytecode_stream++ << 8) | displacement;
      bytes_decoded_count++;
    } else if (displacement & 0x80) {
      displacement = (0xff << 8) | displacement;
    }
  }
  if (is_direct_access) {
    snprintf(mem_block_dest, MEM_BLOCK_SIZE, "[%d]", displacement);
  } else if (has_displacement && displacement) {
    const char *sign_str = displacement < 0 ? "-" : "+";
    snprintf(mem_block_dest, MEM_BLOCK_SIZE, "[%s %s %d]",
             address_calculation_array[opcode_byte2.f2.reg_mem], sign_str, displacement < 0 ? -displacement : displacement);
  } else {
    snprintf(mem_block_dest, MEM_BLOCK_SIZE, "[%s]",
             address_calculation_array[opcode_byte2.f2.reg_mem]);
  }
  return bytes_decoded_count;
}

uint8_t mov_reg_mem_to_reg_mem_handler(char *instruction_dest,
                                       OpCodeByte opcode,
                                       uint8_t *bytecode_stream) {
  OpCodeByte opcode_byte2{*bytecode_stream++};
  uint8_t bytes_decoded_count = 1;
  char mem_block[MEM_BLOCK_SIZE]{};
  bytes_decoded_count +=
      get_mem_block(mem_block, opcode, opcode_byte2, bytecode_stream);
  bytecode_stream += bytes_decoded_count - 1;
  const char **lookup_array =
      opcode.f1.width ? register_array_wide : register_array;
  const char *source;
  const char *target;
  if (opcode_byte2.f2.mode == 0b11) {
    source = opcode.f1.direction ? lookup_array[opcode_byte2.f2.reg_mem]
                                 : lookup_array[opcode_byte2.f2.reg];
    target = opcode.f1.direction ? lookup_array[opcode_byte2.f2.reg]
                                 : lookup_array[opcode_byte2.f2.reg_mem];
  } else {
    source =
        opcode.f1.direction ? mem_block : lookup_array[opcode_byte2.f2.reg];
    target =
        opcode.f1.direction ? lookup_array[opcode_byte2.f2.reg] : mem_block;
  }
  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "mov %s, %s", target,
           source);
  return bytes_decoded_count;
}

uint8_t mov_immediate_to_reg_mem_handler(char *instruction_dest,
                                         OpCodeByte opcode,
                                         uint8_t *bytecode_stream) {
  OpCodeByte opcode_byte2{*bytecode_stream++};
  uint8_t bytes_decoded_count = 1;
  char mem_block[MEM_BLOCK_SIZE]{};
  bytes_decoded_count +=
      get_mem_block(mem_block, opcode, opcode_byte2, bytecode_stream);
  bytecode_stream += bytes_decoded_count - 1;

  uint32_t data = *bytecode_stream++;
  bytes_decoded_count++;
  if (opcode.f1.width) {
    data += *bytecode_stream++ << 8;
    bytes_decoded_count++;
  }

  char immediate[16]{};
  if (opcode.f1.width) {
    snprintf(immediate, sizeof(immediate), "word %u", data);
  } else {
    snprintf(immediate, sizeof(immediate), "byte %u", data);
  }
  const char *source = immediate;
  const char *target = mem_block;
  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "mov %s, %s", target,
           source);
  return bytes_decoded_count;
}

uint8_t mov_immediate_to_reg_handler(char *instruction_dest, OpCodeByte opcode,
                                     uint8_t *bytecode_stream) {
  int bytes_decoded_count = 1;
  uint32_t data = *bytecode_stream++;
  const char **lookup_array = register_array;
  if (opcode.byte & 0b00001000) {
    lookup_array = register_array_wide;
    data += *bytecode_stream++ << 8;
    bytes_decoded_count++;
  }
  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "mov %s, %u",
           lookup_array[opcode.f2.reg_mem], data);
  return bytes_decoded_count;
}

uint8_t mov_mem_to_from_accum_handler(char *instruction_dest, OpCodeByte opcode,
                                      uint8_t *bytecode_stream) {
  int bytes_decoded_count = 1;
  uint32_t address = *bytecode_stream++;
  if (opcode.f1.width) {
    address += *bytecode_stream++ << 8;
    bytes_decoded_count++;
  }
  char mem_block[32]{};
  snprintf(mem_block, sizeof(mem_block), "[%u]", address);
  const char *source = opcode.f1.direction ? "ax" : mem_block;
  const char *target = opcode.f1.direction ? mem_block : "ax";
  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "mov %s, %s", target,
           source);
  return bytes_decoded_count;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: decode_8086 <bytecode_filepath>");
    return 1;
  }
  const char *bytecode_filepath = argv[1];
  FILE *input_bytecode_file_pointer;
  input_bytecode_file_pointer = std::fopen(bytecode_filepath, "rb");
  if (!input_bytecode_file_pointer) {
    printf("Could not open file for reading: `%s`", bytecode_filepath);
    return 1;
  }
  FILE *output_decoded_file_pointer;
  const char output_filepath[] = "decoded.asm";
  output_decoded_file_pointer = std::fopen(output_filepath, "w");

  uint8_t buffer[BUFFER_SIZE + BUFFER_PADDING]{};
  uint8_t *buf_p = buffer + BUFFER_PADDING;
  uint32_t bytes_decoded_count = 0;
  char transient_byte_char_array[9] = {};
  for (uint8_t i = 0; i != 255; i++) {
    opcode_to_handler_mapping[i] = unknown_opcode_handler;
  }

  for (uint8_t i = 0; i < 4; i++)
    opcode_to_handler_mapping[(0b100010 << 2) | i] = mov_reg_mem_to_reg_mem_handler;
  for (uint8_t i = 0; i < 4; i++)
    opcode_to_handler_mapping[(0b101000 << 2) | i] = mov_mem_to_from_accum_handler;
  for (uint8_t i = 0; i < 16; i++)
    opcode_to_handler_mapping[(0b1011 << 4) | i] = mov_immediate_to_reg_handler;
  opcode_to_handler_mapping[0b11000110] = mov_immediate_to_reg_mem_handler;
  opcode_to_handler_mapping[0b11000111] = mov_immediate_to_reg_mem_handler;
  while (1) {
    uint32_t bytes_read = std::fread(buffer + BUFFER_PADDING, sizeof(uint8_t),
                                     BUFFER_SIZE, input_bytecode_file_pointer);
    printf("\n==============================================\n");
    printf("Read block of %u bytes:\n", bytes_read);

    printf("\nBuffer padding status:\n");
    print_bytes(buffer, BUFFER_PADDING, 2);
    printf("\nInput bytes:\n");
    print_bytes(buffer + BUFFER_PADDING, bytes_read, 2);

    uint8_t *buffer_break_point =
        std::min(buffer + BUFFER_PADDING + bytes_read, buffer + BUFFER_SIZE);

    printf("\nDecoded instructions:\n");
    while (buf_p < buffer_break_point) {
      uint8_t *instruction_first_byte_pointer = buf_p;
      char instruction[INSTRUCTION_DEST_SIZE]{};
      OpCodeByte opcode_byte1{*buf_p++};

      OpCodeHandler *handler = opcode_to_handler_mapping[opcode_byte1.byte];
      uint8_t decoded_byte_count = handler(instruction, opcode_byte1, buf_p);
      buf_p += decoded_byte_count;
      decoded_byte_count++;

      if (!instruction[0]) {
        printf("Error decoding bytecode at location %u\n", bytes_decoded_count);
        printf("Next 4 bytes were ");
        for (uint8_t i = 0; i < 4; i++) {
          byte_c_str(instruction_first_byte_pointer[i],
                     transient_byte_char_array);
          printf("%s ", transient_byte_char_array);
        }
        printf("\n");
        return 1;
      }

      printf("%s", instruction);
      printf("\t\t | ");
      for (uint8_t i = 0; i < decoded_byte_count; i++) {
        byte_c_str(instruction_first_byte_pointer[i],
                   transient_byte_char_array);
        printf("%s ", transient_byte_char_array);
      }
      printf("\n");
      fprintf(output_decoded_file_pointer, "%s\n", instruction);
      bytes_decoded_count += decoded_byte_count;
    }
    if (bytes_read < BUFFER_SIZE) {
      break;
    }
    uint8_t bytes_remaining = buffer + BUFFER_PADDING + BUFFER_SIZE - buf_p;
    for (uint8_t i = 0; i < BUFFER_PADDING - bytes_remaining; i++) {
      buffer[i] = 0;
    }
    for (uint8_t i = bytes_remaining; i > 0; i--) {
      buffer[BUFFER_PADDING - i] = *buf_p++;
    }
    buf_p = buffer + BUFFER_PADDING - bytes_remaining;
    printf("Finished block. Copying %u bytes into padding.\n", bytes_remaining);
    printf("==============================================\n");
  }
  return 0;
}
