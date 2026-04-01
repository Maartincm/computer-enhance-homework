#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "haversine_types.h"

#include "haversine_stack.cpp"

namespace JSON {
typedef u32 Type;
enum Types : u64 {
  ERROR = 0,
  OBJECT,
  ARRAY,
  KEY,
  NUMERIC,
  INTEGER,
  FLOAT,
  STRING,
  BOOLEAN,
};

enum Token : u32 {
  TOKEN_NONE,
  CURLY_BRACE_OPEN,
  SQUARE_BRACKET_OPEN,
  DOUBLE_QUOTE_OPEN,
};

enum ParseState : u32 {
  STATE_NONE,
  STATE_START,
  STATE_OBJECT,
  STATE_ARRAY,
  STATE_OBJECT_KEY,
  STATE_OBJECT_KEY_DONE,
  STATE_OBJECT_EXPECTING_VALUE,
  STATE_OBJECT_VALUE_STRING,
  STATE_OBJECT_VALUE_NUMERIC,
  STATE_OBJECT_VALUE_DONE,
};

struct Header {
  u32 size;
  u32 count;
  Types type;
  Header *next_header;
  Header *next_inner_header;
};

struct Proc {
  Token token;
  ParseState state;
  Header *header_loc;
};

Header *parse(const char *input, u64 size) {
  Header *base_data = (Header *)malloc(size);
  Header *header_loc = base_data;
  Stack::Stack stack = Stack::create();
  ParseState state = STATE_START;
  u8 *data_p;
  u64 seen_count = 0;
  char ch;
  do {
    ch = *input++;
    switch (state) {
    case STATE_START: {
      switch (ch) {
      case ' ':
      case '\n':
        break;

      case '{': {
        *header_loc = {0, 0, Types::OBJECT, 0, header_loc + 1};
        Proc p = {CURLY_BRACE_OPEN, state, header_loc};
        state = STATE_OBJECT;
        Stack::push(&stack, &p, sizeof(p));
        header_loc++;
        break;
      }
      case '[': {
        *header_loc = {0, 0, Types::ARRAY, 0, header_loc + 1};
        Proc p = {SQUARE_BRACKET_OPEN, state, header_loc};
        state = STATE_ARRAY;
        Stack::push(&stack, &p, sizeof(p));
        header_loc++;
        break;
      }
      }
      break;
    }
    case STATE_OBJECT: {
      switch (ch) {
      case ' ':
      case '\n':
        break;
      case '"': {
        *header_loc = {0, 0, Types::KEY, 0, 0};
        Proc p = {DOUBLE_QUOTE_OPEN, state, header_loc};
        state = STATE_OBJECT_KEY;
        Stack::push(&stack, &p, sizeof(p));
        data_p = (u8 *)(header_loc + 1);
        break;
      }
      case '}': {
        state = STATE_OBJECT_VALUE_DONE;
        seen_count--;
        input--;
        break;
      }
      default: {
        printf("Invalid JSON char at %lu while processing object expecting key "
               "start. Char was `%c`\n",
               seen_count, ch);
        throw std::runtime_error("Invalid JSON");
      }
      }
      break;
    }
    case STATE_OBJECT_KEY: {
      switch (ch) {
      case '"': {
        // TODO: Here we might want to add a \0 at the end of the string?
        // TODO: Maybe we want to align the next header to 8 byte boundaries? We
        // can use the string size for that
        // TODO: Stack peek to get object proc -> header and increase pair count
        *data_p++ = '\0';
        Proc *p = (Proc *)Stack::pop(&stack, sizeof(Proc));
        if (p->token != DOUBLE_QUOTE_OPEN) {
          printf("Invalid JSON char at %lu while processing object. Closing "
                 "double quote doesnt match.\n",
                 seen_count);
          throw std::runtime_error("Invalid JSON");
        }
        header_loc = (Header *)data_p;
        p->header_loc->next_inner_header = header_loc;

        state = STATE_OBJECT_KEY_DONE;
        Proc *object_p = (Proc *)Stack::peek(&stack, sizeof(Proc));
        object_p->header_loc->count++;
        object_p->header_loc->size += p->header_loc->size;

        Stack::push(&stack, p, sizeof(Proc));
        break;
      }
      default: {
        *data_p++ = ch;
        header_loc->count++;
        header_loc->size++;
        break;
      }
      }
      break;
    }
    case STATE_OBJECT_KEY_DONE: {
      switch (ch) {
      case ' ':
      case '\n':
        break;
      case ':': {
        state = STATE_OBJECT_EXPECTING_VALUE;
        break;
      }
      default: {
        printf("Invalid JSON char at %lu while processing object expecting "
               "colon between key and value\n",
               seen_count);
        throw std::runtime_error("Invalid JSON");
      }
      }
      break;
    }
    case STATE_ARRAY:
    case STATE_OBJECT_EXPECTING_VALUE: {
      switch (ch) {
      case ' ':
      case '\n':
        break;
      case '{': {
        *header_loc = {0, 0, Types::OBJECT, 0, header_loc + 1};
        Proc p = {CURLY_BRACE_OPEN, state, header_loc};
        state = STATE_OBJECT;
        Stack::push(&stack, &p, sizeof(p));
        data_p = (u8 *)(header_loc + 1);
        header_loc++;
        break;
      }
      case '[': {
        *header_loc = {0, 0, Types::ARRAY, 0, header_loc + 1};
        Proc p = {SQUARE_BRACKET_OPEN, state, header_loc};
        state = STATE_ARRAY;
        Stack::push(&stack, &p, sizeof(p));
        data_p = (u8 *)(header_loc + 1);
        header_loc++;
        break;
      }
      case '"': {
        *header_loc = {0, 0, Types::STRING, 0, 0};
        Proc p = {DOUBLE_QUOTE_OPEN, state, header_loc};
        state = STATE_OBJECT_VALUE_STRING;
        Stack::push(&stack, &p, sizeof(p));
        data_p = (u8 *)(header_loc + 1);
        break;
      }
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '-': {
        *header_loc = {0, 0, Types::NUMERIC, 0, 0};
        Proc p = {TOKEN_NONE, state, header_loc};
        state = STATE_OBJECT_VALUE_NUMERIC;
        Stack::push(&stack, &p, sizeof(p));
        data_p = (u8 *)(header_loc + 1);
        *data_p++ = ch;
        break;
      }
      case ',': {
        if (state == STATE_ARRAY) {
          state = STATE_ARRAY;
        } else {
          printf("Invalid JSON char at %lu while processing object expecting "
                 "value to start. Instead got `%c`.\n",
                 seen_count, ch);
          throw std::runtime_error("Invalid JSON");
        }
        break;
      }
      case ']': {
        if (state == STATE_ARRAY) {
          Proc *p = (Proc *)Stack::pop(&stack, sizeof(Proc));
          if (p->token != SQUARE_BRACKET_OPEN) {
            printf("Invalid JSON char at %lu while processing object. Closing "
                   "square bracket doesnt match.\n",
                   seen_count);
            throw std::runtime_error("Invalid JSON");
          }
          state = p->state;
          if (state == STATE_START) {
            return base_data;
          } else if (state == STATE_OBJECT_EXPECTING_VALUE) {
            Proc *p_key = (Proc *)Stack::pop(&stack, sizeof(Proc));
            p_key->header_loc->next_inner_header = p->header_loc;
            p_key->header_loc->next_header = header_loc;
            state = STATE_OBJECT_VALUE_DONE;
          } else if (state == STATE_ARRAY) {
            state = STATE_ARRAY;
          }
          p->header_loc->next_header = header_loc;
          // p->header_loc->next_inner_header = header_loc;
          break;
        } else {
          printf("Invalid JSON char at %lu while processing object expecting "
                 "value to start. Instead got `%c`\n",
                 seen_count, ch);
          throw std::runtime_error("Invalid JSON");
        }
      }
      // case '}': {
      //   if (state == STATE_ARRAY) {
      //     Proc *p_key = (Proc *)Stack::pop(&stack, sizeof(Proc));
      //     p_key->header_loc->next_header = header_loc;
      //
      //     state = STATE_OBJECT_VALUE_DONE;
      //     input--;
      //     seen_count--;
      //   } else {
      //     printf("Invalid JSON char at %lu while processing object expecting
      //     "
      //            "value to start. Instead got `%c`\n",
      //            seen_count, ch);
      //     throw std::runtime_error("Invalid JSON");
      //   }
      //   break;
      // }
      default: {
        printf("Invalid JSON char at %lu while processing object expecting "
               "value to start\n",
               seen_count);
        throw std::runtime_error("Invalid JSON");
      }
      }
      break;
    }
    case STATE_OBJECT_VALUE_STRING: {
      switch (ch) {
      case '"': {
        *data_p++ = '\0';
        header_loc = (Header *)data_p;
        // TODO: Here we might want to add a \0 at the end of the string?
        // TODO: Maybe we want to align the next header to 8 byte boundaries? We
        // can use the string size for that
        // TODO: Stack peek to get object proc -> header and increase pair count
        Proc *p_value = (Proc *)Stack::pop(&stack, sizeof(Proc));
        p_value->header_loc->next_inner_header = header_loc;
        p_value->header_loc->next_header = header_loc;

        Proc *p_parent = (Proc *)Stack::pop(&stack, sizeof(Proc));
        if (p_parent->header_loc->type == JSON::ARRAY) {
          if (ch == '}') {
            throw std::runtime_error("Invalid JSON");
          }
          p_parent->header_loc->count++;
          p_parent->header_loc->size += p_value->header_loc->size;
          Stack::push(&stack, p_parent, sizeof(Proc));
          state = STATE_ARRAY;
        } else if (p_parent->header_loc->type == JSON::KEY) {
          if (ch == ']') {
            throw std::runtime_error("Invalid JSON");
          }
          p_parent->header_loc->next_header = header_loc;
          state = STATE_OBJECT_VALUE_DONE;
        } else {
          throw std::runtime_error("Invalid JSON");
        }
        break;
      }
      default: {
        *data_p++ = ch;
        header_loc->count++;
        header_loc->size++;
        break;
      }
      }
      break;
    }
    case STATE_OBJECT_VALUE_NUMERIC: {
      switch (ch) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '.': {
        *data_p++ = ch;
        break;
      }
      case ' ':
      case '\n':
      case '}':
      case ']':
      case ',': {
        *data_p++ = '\0';
        Proc *p_value = (Proc *)Stack::pop(&stack, sizeof(Proc));
        char *num_str = (char *)(p_value->header_loc + 1);
        bool is_float = false;
        for (char *num_ch = num_str; *num_ch != '\0'; num_ch++) {
          if (*num_ch == '.') {
            is_float = true;
            break;
          }
        }
        u64 *num_ptr = (u64 *)num_str;
        if (is_float) {
          f64 *fnum_ptr = (f64 *)num_ptr;
          *fnum_ptr = atof(num_str);
          p_value->header_loc->type = FLOAT;
        } else {
          *num_ptr = std::atoi(num_str);
          p_value->header_loc->type = INTEGER;
        }
        p_value->header_loc->count = 1;
        p_value->header_loc->size = sizeof(s64);
        header_loc = (Header *)(num_ptr + 1);
        // TODO: Here we might want to add a \0 at the end of the string?
        // TODO: Maybe we want to align the next header to 8 byte boundaries? We
        // can use the string size for that
        // TODO: Stack peek to get object proc -> header and increase pair count
        p_value->header_loc->next_header = header_loc;

        Proc *p_parent = (Proc *)Stack::pop(&stack, sizeof(Proc));
        if (p_parent->header_loc->type == JSON::ARRAY) {
          if (ch == '}') {
            throw std::runtime_error("Invalid JSON");
          }
          p_parent->header_loc->count++;
          p_parent->header_loc->size += p_value->header_loc->size;
          Stack::push(&stack, p_parent, sizeof(Proc));
          state = STATE_ARRAY;
        } else if (p_parent->header_loc->type == JSON::KEY) {
          if (ch == ']') {
            throw std::runtime_error("Invalid JSON");
          }
          p_parent->header_loc->next_header = header_loc;
          state = STATE_OBJECT_VALUE_DONE;
        } else {
          throw std::runtime_error("Invalid JSON");
        }

        input--;
        seen_count--;
        break;
      }
      default: {
        printf("Invalid JSON char at %lu while processing numeric value. Char "
               "was `%c`\n",
               seen_count, ch);
        throw std::runtime_error("Invalid JSON");
      }
      }
      break;
    }
    case STATE_OBJECT_VALUE_DONE: {
      switch (ch) {
      case ' ':
      case '\n':
        break;
      case '}': {
        Proc *p = (Proc *)Stack::pop(&stack, sizeof(Proc));
        if (p->token != CURLY_BRACE_OPEN) {
          printf("Invalid JSON char at %lu while processing object. Closing "
                 "curly brace doesnt match.\n",
                 seen_count);
          throw std::runtime_error("Invalid JSON");
        }
        state = p->state;
        if (state == STATE_START) {
          return base_data;
        }
        if (state == STATE_OBJECT_EXPECTING_VALUE) {
          state = STATE_OBJECT_VALUE_DONE;
          Proc *p_parent = (Proc *)Stack::pop(&stack, sizeof(Proc));
          p_parent->header_loc->next_inner_header = p->header_loc;
          p_parent->header_loc->next_header = header_loc;
        } else if (state == STATE_ARRAY) {
          Proc *p_parent = (Proc *)Stack::peek(&stack, sizeof(Proc));
          p_parent->header_loc->count++;
          p_parent->header_loc->size += p->header_loc->size;
          state = STATE_ARRAY;
        }
        else {
          throw std::runtime_error("Invalid JSON");
        }
        p->header_loc->next_header = header_loc;
        // p->header_loc->next_inner_header = header_loc;
        break;
      }
      case ',': {
        Proc *object_p = (Proc *)Stack::peek(&stack, sizeof(Proc));
        state =
            object_p->header_loc->type == OBJECT ? STATE_OBJECT : STATE_ARRAY;
        break;
      }
      default: {
        printf("Invalid JSON char at %lu while processing object after value "
               "was done\n",
               seen_count);
        throw std::runtime_error("Invalid JSON");
      }
      }
      break;
    }
    case STATE_NONE: {
      throw std::runtime_error("Invalid JSON");
      break;
    }
    }
    seen_count++;
  } while (ch != '\0');
  return base_data;
}

Header *parse(const char *input) {
  return parse(input, 1<<20);
}

Type get_type(Header *data) {
  Header *head = (Header *)data;
  return head->type;
}

Header *get_value_header(Header *object_header, const char *lookup_key) {
  Types data_type{get_type(object_header)};
  if (data_type != Types::OBJECT) {
    printf("Trying to get value type for non object json data (type was "
           "`%ld`)\n",
           data_type);
    throw std::runtime_error("Invalid JSON");
  }
  Header *key_header = object_header->next_inner_header;
  Header *value_header = 0;
  for (u32 i = 0; i < object_header->count; i++) {
    char *key_data = (char *)(key_header + 1);
    if (strcmp(key_data, lookup_key)) {
      key_header = key_header->next_header;
    } else {
      value_header = key_header->next_inner_header;
      break;
    }
  }
  if (value_header == 0) {
    printf("Could not find key `%s` in JSON object.\n", lookup_key);
    throw std::runtime_error("Could not find key in JSON object.");
  }
  return value_header;
}

Header *get_array_elem_header(Header *array_header, u64 index) {
  Types data_type{get_type(array_header)};
  if (data_type != Types::ARRAY) {
    printf("Trying to get element from non array json data (type was `%ld`)\n",
           data_type);
    throw std::runtime_error("Invalid JSON");
  }
  if (index < 0 || index > array_header->count) {
    printf("Array out of index in JSON object. Index was `%lu`\n", index);
    throw std::runtime_error("Array out of index in JSON object.");
  }
  Header *value_header = array_header->next_inner_header;
  for (u32 i = 0; i < index; i++) {
    value_header = value_header->next_header;
  }
  return value_header;
}

char *get_value_string(Header *data, const char *lookup_key) {
  Header *value_header = get_value_header(data, lookup_key);
  return (char *)(value_header + 1);
}

f64 get_value_f64(Header *data, const char *lookup_key) {
  Header *value_header = get_value_header(data, lookup_key);
  return *(f64 *)(value_header + 1);
}

u64 get_value_u64(Header *data, const char *lookup_key) {
  Header *value_header = get_value_header(data, lookup_key);
  return *(u64 *)(value_header + 1);
}

char *get_array_elem_string(Header *data, u64 index) {
  Header *value_header = get_array_elem_header(data, index);
  return (char *)(value_header + 1);
}

f64 get_array_elem_f64(Header *data, u64 index) {
  Header *value_header = get_array_elem_header(data, index);
  return *(f64 *)(value_header + 1);
}

u64 get_array_elem_u64(Header *data, u64 index) {
  Header *value_header = get_array_elem_header(data, index);
  return *(u64 *)(value_header + 1);
}

} // namespace JSON
