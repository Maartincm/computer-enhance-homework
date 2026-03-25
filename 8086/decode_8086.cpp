#include <cstdint>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

#define OP_MOV 0b100010

#define OP_MODE_REGISTER_MODE 0b11

union OpCodeByte1 {
  uint8_t byte;
  struct {
    uint8_t width : 1;
    uint8_t direction : 1;
    uint8_t code : 6;
  } fields;
  struct {
    uint8_t dir_and_width : 2;
    uint8_t : 6;
  } merged;
};
union OpCodeByte2 {
  uint8_t byte;
  struct {
    uint8_t reg_mem : 3;
    uint8_t reg : 3;
    uint8_t mode : 2;
  } fields;
};

const char *register_array[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
const char *register_array_wide[] = {"ax", "cx", "dx", "bx",
                                     "sp", "bp", "si", "di"};

void print_bytes(const char *bytes, uint32_t size, uint8_t column_width) {
  printf("Input bytes:\n");
  uint32_t printed_byte_count = 0;
  for (uint32_t i = 0; i < size; i += 8) {
    printf("%.8s", bytes + i);
    printed_byte_count++;
    if (printed_byte_count > column_width) {
      printed_byte_count = 0;
      printf("\n");
    } else {
      printf(" ");
    }
  }
  printf("\n");
}
void print_bytes(const char *bytes, uint32_t size) {
  print_bytes(bytes, size, 8);
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
  const char output_filepath[] = "8086.decoded";
  output_decoded_file_pointer = std::fopen(output_filepath, "w");

  uint8_t buffer[BUFFER_SIZE]{};
  char bin_str[BUFFER_SIZE * 8 + 1]{};
  uint32_t bytes_read = 0;
  printf("Decoded instructions:\n");
  while (1) {
    bytes_read = std::fread(buffer, sizeof(uint8_t), BUFFER_SIZE,
                            input_bytecode_file_pointer);
    for (uint32_t i = 0; i < bytes_read; i += 2) {
      // uint8_t byte_read_1 = buffer[i];
      // uint8_t byte_read_2 = buffer[i + 1];
      //
      // uint8_t op_code = byte_read_1 & 0b11111100;
      // uint8_t op_direction = byte_read_1 & 0b00000010;
      // uint8_t op_width = byte_read_1 & 0b00000001;
      // uint8_t op_mode = byte_read_2 & 0b11000000;
      // uint8_t op_register = (byte_read_2 & 0b00111000) >> 3;
      // uint8_t op_register_mem = byte_read_2 & 0b00000111;

      OpCodeByte1 opcode_byte1{buffer[i]};
      OpCodeByte2 opcode_byte2{buffer[i + 1]};

      char instruction[32]{};
      switch (opcode_byte1.fields.code) {
      case OP_MOV:
        switch (opcode_byte2.fields.mode) {
        case OP_MODE_REGISTER_MODE:

          switch (opcode_byte1.merged.dir_and_width) {
          // const char **reg_lookup =
          //     opcode_byte1.fields.width ? register_array_wide :
          //     register_array;
          // uint8_t source_reg = opcode_byte1.fields.direction
          //                          ? opcode_byte2.fields.reg_mem
          //                          : opcode_byte2.fields.reg;
          // uint8_t target_reg = opcode_byte1.fields.direction
          //                          ? opcode_byte2.fields.reg
          //                          : opcode_byte2.fields.reg_mem;
          // snprintf(instruction, sizeof(instruction), "mov %s, %s",
          //          reg_lookup[target_reg], reg_lookup[source_reg]);
          case 0b00:
            snprintf(instruction, sizeof(instruction), "mov %s, %s",
                     register_array[opcode_byte2.fields.reg_mem],
                     register_array[opcode_byte2.fields.reg]);
            break;
          case 0b01:
            snprintf(instruction, sizeof(instruction), "mov %s, %s",
                     register_array_wide[opcode_byte2.fields.reg_mem],
                     register_array_wide[opcode_byte2.fields.reg]);
            break;
          case 0b10:
            snprintf(instruction, sizeof(instruction), "mov %s, %s",
                     register_array[opcode_byte2.fields.reg],
                     register_array[opcode_byte2.fields.reg_mem]);
            break;
          case 0b11:
            snprintf(instruction, sizeof(instruction), "mov %s, %s",
                     register_array_wide[opcode_byte2.fields.reg],
                     register_array_wide[opcode_byte2.fields.reg_mem]);
            break;
          default:
            break;
          }
          break;

        default:
          break;
        }
        break;
      default:
        break;
      }

      printf("%s\n", instruction);
      fprintf(output_decoded_file_pointer, "%s\n", instruction);

      // Write the ASCII representation of the binary ones and zeroes
      for (int8_t j = 0; j < 8; j++) {
        bin_str[i * 8 + j] = ((opcode_byte1.byte >> j & 0x01) == 1) ? '1' : '0';
        bin_str[(i + 1) * 8 + j] =
            ((opcode_byte2.byte >> j & 0x01) == 1) ? '1' : '0';
      }
    }
    uint32_t decoded_size = bytes_read * 8;
    bin_str[decoded_size] = '\0';
    printf("\n");
    print_bytes(bin_str, decoded_size, 2);
    if (bytes_read < BUFFER_SIZE) {
      break;
    }
  }
  return 0;
}
