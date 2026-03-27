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

#define ARRAY_SIZE(input) (sizeof(input) / sizeof((input)[0]))

namespace Flags {
enum : uint16_t {
  CARRY = 1 << 0,
  PARITY = 1 << 2,
  AUX_CARRY = 1 << 4,
  ZERO = 1 << 6,
  SIGN = 1 << 7,
  TRAP = 1 << 8,
  INTERRUPT = 1 << 9,
  DIRECTION = 1 << 10,
  OVERFLOW = 1 << 11,
};
}
const char *flag_names_array[] = {"C", "?", "P", "?", "A", "?", "Z", "S",
                                  "T", "I", "D", "O", "?", "?", "?", "?"};

uint16_t flags_register{};
uint16_t previuos_flags_register{};

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
typedef int16_t(OpCodeHandler)(char *, OpCodeByte, uint8_t *, const char *);
struct OpCodeHandlerMapping {
  OpCodeHandler *handler;
  const char *instruction_name;
};
OpCodeHandlerMapping opcode_to_handler_mapping[256];

const char *segment_register_array[] = {"es", "cs", "ss", "ds"};
const char *register_array[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
const char *register_array_wide[] = {"ax", "cx", "dx", "bx",
                                     "sp", "bp", "si", "di"};
uint8_t sorted_reg_index[] = {0, 3, 1, 2, 4, 5, 6, 7, 0, 1, 2, 3};
const char *address_calculation_array[] = {
    "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};
const char *icode_reg[] = {"add", "or",  "adc", "sbb",
                           "and", "sub", "unk", "cmp"};

uint16_t registers[12]{};
uint16_t registers_previous_state[12]{};

void snapshot_registers_state() {
  for (uint8_t i = 0; i < ARRAY_SIZE(registers); i++) {
    registers_previous_state[i] = registers[i];
  }
}

void snapshot_flags_state() { previuos_flags_register = flags_register; }

void print_flags_change() {
  bool changed = false;
  for (uint8_t i = 0; i < 16; i++) {
    uint8_t cur_val = (flags_register >> i) & 0x01;
    uint8_t prev_val = (previuos_flags_register >> i) & 0x01;
    if (cur_val != prev_val) {
      changed = true;
    }
  }
  if (changed) {
    for (uint8_t i = 0; i < 16; i++) {
      uint8_t prev_val = (previuos_flags_register >> i) & 0x01;
      if (prev_val) {
        printf("%s", flag_names_array[i]);
      }
    }
    printf(" -> ");
    for (uint8_t i = 0; i < 16; i++) {
      uint8_t cur_val = (flags_register >> i) & 0x01;
      if (cur_val) {
        printf("%s", flag_names_array[i]);
      }
    }
  }
}

void print_register_change() {
  const char *delim = "";
  bool changed = false;
  for (uint8_t i = 0; i < ARRAY_SIZE(registers); i++) {
    if (registers_previous_state[i] != registers[i]) {
      if (i < 8) {
        printf("%s%s: 0x%04x -> 0x%04x", delim, register_array_wide[i],
               registers_previous_state[i], registers[i]);
      } else {
        printf("%s%s: 0x%04x -> 0x%04x", delim,
               segment_register_array[sorted_reg_index[i]],
               registers_previous_state[i], registers[i]);
      }
      delim = " | ";
      changed = true;
    }
  }
  if (!changed) {
    printf("no changes");
  }
}

void print_registers(uint32_t ip_final_position) {
  printf("==============================================\n");
  printf("Registers state\n");
  for (uint8_t i = 0; i < ARRAY_SIZE(registers); i++) {
    uint8_t index = sorted_reg_index[i];
    if (i < 8) {
      printf("\t%s: 0x%04x\n", register_array_wide[index], registers[index]);
    } else {
      printf("\t%s: 0x%04x\n", segment_register_array[index], registers[i]);
    }
  }
  printf("\tip: 0x%04x\n", ip_final_position);
  printf("==============================================\n");
}

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

int16_t unknown_opcode_handler(char *instruction_dest, OpCodeByte opcode,
                               uint8_t *bytecode_stream, const char *icode) {
  printf("ERROR: Uknown opcode: ");
  print_bytes(&opcode.byte, 1);
  printf("\n");
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
             address_calculation_array[opcode_byte2.f2.reg_mem], sign_str,
             displacement < 0 ? -displacement : displacement);
  } else {
    snprintf(mem_block_dest, MEM_BLOCK_SIZE, "[%s]",
             address_calculation_array[opcode_byte2.f2.reg_mem]);
  }
  return bytes_decoded_count;
}

void simulate(OpCodeByte opcode, uint8_t target_index, uint8_t source_index,
              bool is_wide, bool is_immediate, uint32_t provided_data,
              const char *icode) {
  if (opcode.byte >> 2 == 0b100011) {
    // NOTE: We are dealing with segment registers
    if (opcode.f1.direction) {
      target_index += 8;
    } else {
      source_index += 8;
    }
  }

  uint32_t data;
  if (is_immediate) {
    data = provided_data;
  } else {
    uint8_t shift_amount = 0;
    if (!is_wide && source_index >= 4 && source_index < 8) {
      source_index = source_index % 4;
      shift_amount = 8;
    }
    data = registers[source_index] >> shift_amount;
  }

  uint32_t input_data;
  if (is_wide) {
    input_data = data;
  } else {
    // NOTE: Handle only updating low or high byte of the register without
    // overwriting everything.
    uint16_t low_or_high_mask;
    uint8_t shift_amount = 0;
    if (target_index < 4) {
      low_or_high_mask = 0x00ff;
      shift_amount = 0;
    } else {
      low_or_high_mask = 0xff00;
      shift_amount = 8;
    }
    if (target_index < 8) {
      target_index = target_index % 4;
    }
    if (!is_immediate && !is_wide && source_index >= 4) {
      data >>= 8;
    }
    input_data = (registers[target_index] & ~low_or_high_mask) |
                 ((data << shift_amount) & low_or_high_mask);
  }
  uint32_t old_reg_val = registers[target_index];
  if ((int16_t)input_data == -90) {
    printf("test");
  }
  if (icode[0] == 'm') {
    registers[target_index] = input_data;
  } else {
    uint32_t res;
    switch (icode[0]) {
    case 'a': {
      res = registers[target_index] + input_data;
      registers[target_index] = res;
      break;
    }
    case 's': {
      res = registers[target_index] - input_data;
      registers[target_index] = res;
      break;
    }
    case 'c': {
      res = registers[target_index] - input_data;
      break;
    }
    }
    flags_register = 0;
    uint8_t ones_count = 0;
    for (uint8_t i = 0; i < 8; i++) {
      if ((res >> i) & 0x01)
        ones_count++;
    }
    if (ones_count % 2 == 0)
      flags_register |= Flags::PARITY;
    if (res & 0x8000) {
      flags_register |= Flags::SIGN;
    }
    if (icode[0] == 'a') {
      if (((old_reg_val & 0xf) + (input_data & 0xf)) & 0x10)
        flags_register |= Flags::AUX_CARRY;
      if (((old_reg_val & 0xffff) + (input_data & 0xffff)) & 0x10000) {
        flags_register |= Flags::CARRY;
      }
      // if (is_signed != has_carry && (old_reg_val & 0x8000) == (input_data &
      // 0x8000))
      if (!(old_reg_val & 0x8000) && !(input_data & 0x8000) && (res & 0x8000))
        flags_register |= Flags::OVERFLOW;
      if ((old_reg_val & 0x8000) && (input_data & 0x8000) && !(res & 0x8000))
        flags_register |= Flags::OVERFLOW;
    }
    if (icode[0] == 's' || icode[0] == 'c') {
      if ((uint8_t)(old_reg_val & 0xf) < (uint8_t)(input_data & 0xf))
        flags_register |= Flags::AUX_CARRY;
      if (old_reg_val < input_data) {
        flags_register |= Flags::CARRY;
      }
      // if (is_signed != has_carry && (old_reg_val & 0x8000) == (input_data &
      // 0x8000))
      if (!(old_reg_val & 0x8000) && (input_data & 0x8000) && (res & 0x8000))
        flags_register |= Flags::OVERFLOW;
      if ((old_reg_val & 0x8000) && !(input_data & 0x8000) && !(res & 0x8000))
        flags_register |= Flags::OVERFLOW;
    }
    if (!(res & 0xff))
      flags_register |= Flags::ZERO;
  }
  // NOTE: SIMULATION Part ends.
}

int16_t mov_reg_mem_to_reg_mem_handler(char *instruction_dest,
                                       OpCodeByte opcode,
                                       uint8_t *bytecode_stream,
                                       const char *icode) {
  OpCodeByte opcode_byte2{*bytecode_stream++};
  uint8_t bytes_decoded_count = 1;
  char mem_block[MEM_BLOCK_SIZE]{};
  bytes_decoded_count +=
      get_mem_block(mem_block, opcode, opcode_byte2, bytecode_stream);
  bytecode_stream += bytes_decoded_count - 1;
  bool is_wide = opcode.f1.width;
  if (opcode.byte >> 2 == 0b100011) {
    // NOTE: We are dealing with segment registers
    is_wide = true;
  }
  const char **source_lookup_array =
      is_wide ? register_array_wide : register_array;
  const char **target_lookup_array = source_lookup_array;
  const char *source;
  const char *target;
  uint8_t source_index;
  uint8_t target_index;
  if (opcode.byte >> 2 == 0b100011) {
    // NOTE: We are dealing with segment registers
    if (opcode.f1.direction) {
      target_lookup_array = segment_register_array;
    } else {
      source_lookup_array = segment_register_array;
    }
  }

  if (opcode_byte2.f2.mode == 0b11) {
    source_index =
        opcode.f1.direction ? opcode_byte2.f2.reg_mem : opcode_byte2.f2.reg;
    target_index =
        opcode.f1.direction ? opcode_byte2.f2.reg : opcode_byte2.f2.reg_mem;
    source = source_lookup_array[source_index];
    target = target_lookup_array[target_index];
  } else {
    source_index = opcode.f1.direction ? -1 : opcode_byte2.f2.reg;
    target_index = opcode.f1.direction ? opcode_byte2.f2.reg : -1;
    source = opcode.f1.direction ? mem_block
                                 : source_lookup_array[opcode_byte2.f2.reg];
    target = opcode.f1.direction ? target_lookup_array[opcode_byte2.f2.reg]
                                 : mem_block;
  }

  simulate(opcode, target_index, source_index, is_wide, false, 0, icode);

  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "%s %s, %s", icode, target,
           source);
  return bytes_decoded_count;
}

int16_t mov_immediate_to_reg_mem_handler(char *instruction_dest,
                                         OpCodeByte opcode,
                                         uint8_t *bytecode_stream,
                                         const char *icode) {
  OpCodeByte opcode_byte2{*bytecode_stream++};
  uint8_t bytes_decoded_count = 1;
  char mem_block[MEM_BLOCK_SIZE]{};
  bytes_decoded_count +=
      get_mem_block(mem_block, opcode, opcode_byte2, bytecode_stream);
  bytecode_stream += bytes_decoded_count - 1;

  uint16_t data = *bytecode_stream++;
  bytes_decoded_count++;
  char immediate[16]{};
  bool is_wide = opcode.f1.width;
  bool sign_extended = false;
  if (is_wide &&
      (!opcode.f1.direction || (icode[0] == 'm' && icode[1] == 'o'))) {
    data += *bytecode_stream++ << 8;
    bytes_decoded_count++;
  } else if (opcode.f1.direction && !(icode[0] == 'm') && data & 0x80) {
    data = (0xff << 8) | data;
    sign_extended = true;
  }
  if (is_wide) {
    if (sign_extended)
      snprintf(immediate, sizeof(immediate), "word %d", int16_t(data));
    else
      snprintf(immediate, sizeof(immediate), "word %u", data);
  } else {
    if (sign_extended)
      snprintf(immediate, sizeof(immediate), "byte %d", int16_t(data));
    else
      snprintf(immediate, sizeof(immediate), "byte %u", data);
  }
  const char *source = immediate;
  const char *target;
  uint8_t target_index = -1;
  if (opcode_byte2.f2.mode == 0b11) {
    target = is_wide ? register_array_wide[opcode_byte2.f2.reg_mem]
                     : register_array[opcode_byte2.f2.reg_mem];
    target_index = opcode_byte2.f2.reg_mem;
  } else {
    target = mem_block;
  }
  const char *actual_icode = (icode[0] == 'm' && icode[1] == 'o')
                                 ? icode
                                 : icode_reg[opcode_byte2.f2.reg];

  simulate(opcode, target_index, 0, is_wide, true, data, actual_icode);

  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "%s %s, %s", actual_icode,
           target, source);
  return bytes_decoded_count;
}

int16_t mov_immediate_to_reg_handler(char *instruction_dest, OpCodeByte opcode,
                                     uint8_t *bytecode_stream,
                                     const char *icode) {
  int bytes_decoded_count = 1;
  uint32_t data = *bytecode_stream++;
  const char **lookup_array = register_array;
  bool is_wide = opcode.byte & 0b00001000;
  if (is_wide) {
    lookup_array = register_array_wide;
    data += *bytecode_stream++ << 8;
    bytes_decoded_count++;
  }

  simulate(opcode, opcode.f2.reg_mem, 0, is_wide, true, data, icode);

  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "%s %s, %u", icode,
           lookup_array[opcode.f2.reg_mem], data);
  return bytes_decoded_count;
}

int16_t mov_mem_to_from_accum_handler(char *instruction_dest, OpCodeByte opcode,
                                      uint8_t *bytecode_stream,
                                      const char *icode) {
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
  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "%s %s, %s", icode, target,
           source);
  return bytes_decoded_count;
}

int16_t add_reg_mem_with_reg_handler(char *instruction_dest, OpCodeByte opcode,
                                     uint8_t *bytecode_stream,
                                     const char *icode) {
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
  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "add %s, %s", target,
           source);
  return bytes_decoded_count;
}
int16_t add_immediate_to_reg_mem_handler(char *instruction_dest,
                                         OpCodeByte opcode,
                                         uint8_t *bytecode_stream,
                                         const char *icode) {
  return 0;
}
int16_t add_immediate_to_accum_handler(char *instruction_dest,
                                       OpCodeByte opcode,
                                       uint8_t *bytecode_stream,
                                       const char *icode) {
  int bytes_decoded_count = 1;

  int16_t data = *bytecode_stream++;
  const char *reg = opcode.f1.width ? "ax" : "al";
  if (opcode.f1.width) {
    data += *bytecode_stream++ << 8;
    bytes_decoded_count++;
  } else if (data & 0x80) {
    data = (0xff << 8) | data;
  }
  char mem_block[32]{};
  snprintf(mem_block, sizeof(mem_block), "%d", data);
  const char *source = opcode.f1.direction ? reg : mem_block;
  const char *target = opcode.f1.direction ? mem_block : reg;
  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "%s %s, %s", icode, target,
           source);
  return bytes_decoded_count;
}

int16_t jump_handler(char *instruction_dest, OpCodeByte opcode,
                     uint8_t *bytecode_stream, const char *icode) {
  uint16_t bytes_decoded = 1;
  int8_t offset = *bytecode_stream++;
  offset += 2;
  const char sign_str = offset < 0 ? '-' : '+';
  snprintf(instruction_dest, INSTRUCTION_DEST_SIZE, "%s $%c%d", icode, sign_str,
           offset < 0 ? -offset : offset);
  bool should_jump = false;
  should_jump |=
      (std::strncmp(icode, "jnz", 3) == 0 && !(flags_register & Flags::ZERO));
  should_jump |=
      (std::strncmp(icode, "je", 2) == 0 && (flags_register & Flags::ZERO));
  should_jump |=
      (std::strncmp(icode, "jp", 2) == 0 && (flags_register & Flags::PARITY));
  should_jump |=
      (std::strncmp(icode, "jb", 2) == 0 && (flags_register & Flags::CARRY));
  if (!std::strncmp(icode, "loopnz", 6)) {
    registers[1] -= 1;
    should_jump |= registers[1] != 0;
  }
  if (should_jump) {
    return offset - 2 + bytes_decoded;
  }
  return bytes_decoded;
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

  // NOTE: Initialize all handlers to a non implemented catchall.
  for (uint8_t i = 0; i != 255; i++) {
    opcode_to_handler_mapping[i] = {unknown_opcode_handler, "unk"};
  }

  // NOTE: Here we set each handler for their set of opcodes
  for (uint8_t i = 0; i < 4; i++)
    opcode_to_handler_mapping[(0b100010 << 2) | i] = {
        mov_reg_mem_to_reg_mem_handler, "mov"};
  for (uint8_t i = 0; i < 4; i++)
    opcode_to_handler_mapping[(0b101000 << 2) | i] = {
        mov_mem_to_from_accum_handler, "mov"};
  for (uint8_t i = 0; i < 16; i++)
    opcode_to_handler_mapping[(0b1011 << 4) | i] = {
        mov_immediate_to_reg_handler, "mov"};
  opcode_to_handler_mapping[0b11000110] = {mov_immediate_to_reg_mem_handler,
                                           "mov"};
  opcode_to_handler_mapping[0b11000111] = {mov_immediate_to_reg_mem_handler,
                                           "mov"};
  // NOTE: Segment registers moves
  opcode_to_handler_mapping[0b10001110] = {mov_reg_mem_to_reg_mem_handler,
                                           "mov"};
  opcode_to_handler_mapping[0b10001100] = {mov_reg_mem_to_reg_mem_handler,
                                           "mov"};

  // NOTE: Arithmetic
  for (uint8_t i = 0; i < 4; i++)
    opcode_to_handler_mapping[(0b000000 << 2) | i] = {
        mov_reg_mem_to_reg_mem_handler, "add"};
  for (uint8_t i = 0; i < 4; i++)
    opcode_to_handler_mapping[(0b100000 << 2) | i] = {
        mov_immediate_to_reg_mem_handler, "add"};
  opcode_to_handler_mapping[0b00000100] = {add_immediate_to_accum_handler,
                                           "add"};
  opcode_to_handler_mapping[0b00000101] = {add_immediate_to_accum_handler,
                                           "add"};

  for (uint8_t i = 0; i < 4; i++)
    opcode_to_handler_mapping[(0b001010 << 2) | i] = {
        mov_reg_mem_to_reg_mem_handler, "sub"};
  // for (uint8_t i = 0; i < 4; i++)
  //   opcode_to_handler_mapping[(0b100000 << 2) | i] = {
  //       mov_immediate_to_reg_mem_handler, "sub"};
  opcode_to_handler_mapping[0b00101100] = {add_immediate_to_accum_handler,
                                           "sub"};
  opcode_to_handler_mapping[0b00101101] = {add_immediate_to_accum_handler,
                                           "sub"};

  for (uint8_t i = 0; i < 4; i++)
    opcode_to_handler_mapping[(0b001110 << 2) | i] = {
        mov_reg_mem_to_reg_mem_handler, "cmp"};
  // for (uint8_t i = 0; i < 4; i++)
  //   opcode_to_handler_mapping[(0b100000 << 2) | i] = {
  //       mov_immediate_to_reg_mem_handler, "sub"};
  opcode_to_handler_mapping[0b00111100] = {add_immediate_to_accum_handler,
                                           "cmp"};
  opcode_to_handler_mapping[0b00111101] = {add_immediate_to_accum_handler,
                                           "cmp"};

  // NOTE: Jumps!
  opcode_to_handler_mapping[0b01110101] = {jump_handler, "jnz"};
  opcode_to_handler_mapping[0b01110100] = {jump_handler, "je"};
  opcode_to_handler_mapping[0b01111100] = {jump_handler, "jl"};
  opcode_to_handler_mapping[0b01111110] = {jump_handler, "jle"};
  opcode_to_handler_mapping[0b01110010] = {jump_handler, "jb"};
  opcode_to_handler_mapping[0b01110110] = {jump_handler, "jbe"};
  opcode_to_handler_mapping[0b01111010] = {jump_handler, "jp"};
  opcode_to_handler_mapping[0b01110000] = {jump_handler, "jo"};
  opcode_to_handler_mapping[0b01111000] = {jump_handler, "js"};
  opcode_to_handler_mapping[0b01111101] = {jump_handler, "jnl"};
  opcode_to_handler_mapping[0b01111111] = {jump_handler, "jg"};
  opcode_to_handler_mapping[0b01110011] = {jump_handler, "jnb"};
  opcode_to_handler_mapping[0b01110111] = {jump_handler, "ja"};
  opcode_to_handler_mapping[0b01111011] = {jump_handler, "jnp"};
  opcode_to_handler_mapping[0b01110001] = {jump_handler, "jno"};
  opcode_to_handler_mapping[0b01111001] = {jump_handler, "jns"};
  opcode_to_handler_mapping[0b11100010] = {jump_handler, "loop"};
  opcode_to_handler_mapping[0b11100001] = {jump_handler, "loopz"};
  opcode_to_handler_mapping[0b11100000] = {jump_handler, "loopnz"};
  opcode_to_handler_mapping[0b11100011] = {jump_handler, "jcxz"};

  while (1) {
    uint32_t bytes_read = std::fread(buffer + BUFFER_PADDING, sizeof(uint8_t),
                                     BUFFER_SIZE, input_bytecode_file_pointer);
    printf("\n==============================================\n");
    printf("Read block of %u bytes:\n", bytes_read);

    // printf("\nBuffer padding status:\n");
    // print_bytes(buffer, BUFFER_PADDING, 2);
    // printf("\nInput bytes:\n");
    // print_bytes(buffer + BUFFER_PADDING, bytes_read, 2);

    uint8_t *buffer_break_point =
        std::min(buffer + BUFFER_PADDING + bytes_read, buffer + BUFFER_SIZE);

    printf("\nDecoded instructions:\n");
    while (buf_p < buffer_break_point) {
      uint8_t *instruction_first_byte_pointer = buf_p;
      char instruction[INSTRUCTION_DEST_SIZE]{};
      OpCodeByte opcode_byte1{*buf_p++};

      OpCodeHandlerMapping handler_map =
          opcode_to_handler_mapping[opcode_byte1.byte];

      // if (opcode_byte1.byte == 0x81) {
      //   printf("test");
      // }
      int16_t decoded_byte_count = handler_map.handler(
          instruction, opcode_byte1, buf_p, handler_map.instruction_name);
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
      if (1) {
        printf("\t; ");
        print_register_change();
      }
      if (1) {
        printf("\t| ");
        printf("ip: 0x%04x -> 0x%04x", bytes_decoded_count,
               bytes_decoded_count + decoded_byte_count);
      }
      if (1) {
        printf("\t| ");
        print_flags_change();
      }
      if (1) {
        printf("\t| ");
        for (uint8_t i = 0; i < decoded_byte_count; i++) {
          byte_c_str(instruction_first_byte_pointer[i],
                     transient_byte_char_array);
          printf("%s ", transient_byte_char_array);
        }
      }
      printf("\n");
      fprintf(output_decoded_file_pointer, "%s\n", instruction);
      bytes_decoded_count += decoded_byte_count;

      snapshot_flags_state();
      snapshot_registers_state();
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

  print_registers(bytes_decoded_count);

  return 0;
}
