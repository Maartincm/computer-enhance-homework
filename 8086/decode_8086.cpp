#include <algorithm>
#include <bitset>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define BUFFER_PADDING 8

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
      char instruction[128]{};
      OpCodeByte opcode_byte1{*buf_p++};
      const char **lookup_array =
          opcode_byte1.f1.width ? register_array_wide : register_array;

      switch (opcode_byte1.byte & 0b11111110) {
      case OP_MOV_IMMEDIATE_TO_REG_MEM: {
        OpCodeByte opcode_byte2{*buf_p++};
        uint32_t displacement = 0;
        char mem_block[32]{};
        switch (opcode_byte2.f2.mode) {
        case OP_MODE_MEMORY_MODE_NO_DISP: {
          if (opcode_byte2.f2.reg_mem == 0b110) {
            // NOTE: DIRECT ADDRESS case
            uint8_t data_low = *buf_p++;
            displacement = data_low + (*buf_p++ << 8);
          }
          break;
        }
        case OP_MODE_MEMORY_MODE_BYTE_DISP: {
          displacement = *buf_p++;
          if (displacement & 0x80) {
            displacement += (0xff << 8);
          }
          break;
        }
        case OP_MODE_MEMORY_MODE_WORD_DISP: {
          uint8_t data_low = *buf_p++;
          displacement = data_low + (*buf_p++ << 8);
          break;
        }
        default:
          break;
        }
        uint32_t data = *buf_p++;
        if (opcode_byte1.f1.width) {
          data += *buf_p++ << 8;
        }
        if (displacement && opcode_byte2.f2.reg_mem == 0b110) {
          snprintf(mem_block, sizeof(mem_block), "[%u]", displacement);
        } else if (displacement) {
          snprintf(mem_block, sizeof(mem_block), "[%s + %u]",
                   address_calculation_array[opcode_byte2.f2.reg_mem],
                   displacement);
        } else {
          snprintf(mem_block, sizeof(mem_block), "[%s]",
                   address_calculation_array[opcode_byte2.f2.reg_mem]);
        }
        char immediate[16]{};
        if (opcode_byte1.f1.width) {
          snprintf(immediate, sizeof(immediate), "word %u", data);
        } else {
          snprintf(immediate, sizeof(immediate), "byte %u", data);
        }
        const char *source = immediate;
        const char *target = mem_block;
        snprintf(instruction, sizeof(instruction), "mov %s, %s", target,
                 source);
        break;
      }
      default:
        break;
      }
      switch (opcode_byte1.byte & 0b11111100) {
      case OP_MOV_REG_MEM: {
        OpCodeByte opcode_byte2{*buf_p++};
        switch (opcode_byte2.f2.mode) {
        case OP_MODE_REGISTER_MODE: {
          uint8_t source = opcode_byte1.f1.direction ? opcode_byte2.f2.reg_mem
                                                     : opcode_byte2.f2.reg;
          uint8_t target = opcode_byte1.f1.direction ? opcode_byte2.f2.reg
                                                     : opcode_byte2.f2.reg_mem;
          snprintf(instruction, sizeof(instruction), "mov %s, %s",
                   lookup_array[target], lookup_array[source]);
          break;
        }
        // NOTE: Handling REG TO MEM or MEM TO REG cases
        case OP_MODE_MEMORY_MODE_NO_DISP: {
          char mem_block[32]{};
          if (opcode_byte2.f2.reg_mem == 0b110) {
            // NOTE: DIRECT ADDRESS case
            uint8_t data_low = *buf_p++;
            uint32_t displacement = data_low + (*buf_p++ << 8);
            snprintf(mem_block, sizeof(mem_block), "[%u]", displacement);
          } else {
            snprintf(mem_block, sizeof(mem_block), "[%s]",
                     address_calculation_array[opcode_byte2.f2.reg_mem]);
          }
          const char *source = opcode_byte1.f1.direction
                                   ? mem_block
                                   : lookup_array[opcode_byte2.f2.reg];
          const char *target = opcode_byte1.f1.direction
                                   ? lookup_array[opcode_byte2.f2.reg]
                                   : mem_block;
          snprintf(instruction, sizeof(instruction), "mov %s, %s", target,
                   source);
          break;
        }
        case OP_MODE_MEMORY_MODE_BYTE_DISP: {
          char mem_block[32]{};
          uint32_t displacement = *buf_p++;
          if (displacement & 0x80) {
            displacement += (0xff << 8);
          }
          snprintf(mem_block, sizeof(mem_block), "[%s + %u]",
                   address_calculation_array[opcode_byte2.f2.reg_mem],
                   displacement);
          const char *source = opcode_byte1.f1.direction
                                   ? mem_block
                                   : lookup_array[opcode_byte2.f2.reg];
          const char *target = opcode_byte1.f1.direction
                                   ? lookup_array[opcode_byte2.f2.reg]
                                   : mem_block;
          snprintf(instruction, sizeof(instruction), "mov %s, %s", target,
                   source);
          break;
        }
        case OP_MODE_MEMORY_MODE_WORD_DISP: {
          char mem_block[32]{};
          uint8_t data_low = *buf_p++;
          uint32_t displacement = data_low + (*buf_p++ << 8);
          snprintf(mem_block, sizeof(mem_block), "[%s + %u]",
                   address_calculation_array[opcode_byte2.f2.reg_mem],
                   displacement);
          const char *source = opcode_byte1.f1.direction
                                   ? mem_block
                                   : lookup_array[opcode_byte2.f2.reg];
          const char *target = opcode_byte1.f1.direction
                                   ? lookup_array[opcode_byte2.f2.reg]
                                   : mem_block;
          snprintf(instruction, sizeof(instruction), "mov %s, %s", target,
                   source);
          break;
        }
        default:
          break;
        }
        break;
      }
      case OP_MOV_MEMORY_ACCUMULATOR: {
        uint32_t address = *buf_p++;
        if (opcode_byte1.f1.width) {
          address += *buf_p++ << 8;
        }
        char mem_block[32]{};
        snprintf(mem_block, sizeof(mem_block), "[%u]", address);
        const char *source = opcode_byte1.f1.direction ? "ax" : mem_block;
        const char *target = opcode_byte1.f1.direction ? mem_block : "ax";
        snprintf(instruction, sizeof(instruction), "mov %s, %s", target,
                 source);
        break;
      }
      default:
        break;
      }
      // NOTE: Handle op codes that encode the REG as lowest 3 bits of the first
      // byte
      switch (opcode_byte1.byte & 0b11110000) {
      case OP_MOV_IMMEDIATE_TO_REG: {
        uint32_t data = *buf_p++;
        const char **lookup_array = register_array;
        if (opcode_byte1.byte & 0b00001000) {
          lookup_array = register_array_wide;
          data += *buf_p++ << 8;
        }
        snprintf(instruction, sizeof(instruction), "mov %s, %u",
                 lookup_array[opcode_byte1.f2.reg_mem], data);
        break;
      }
      default:
        break;
      }

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

      uint8_t decoded_byte_count = buf_p - instruction_first_byte_pointer;
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
