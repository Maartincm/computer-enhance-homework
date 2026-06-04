#include <cstdio>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <x86intrin.h>

#include "haversine_types.h"

#define PROFILE 1

#include "haversine_json_parser.cpp"
#include "haversine_timing.cpp"

#define HAVERSINE_REPLACEMENT 1
#if HAVERSINE_REPLACEMENT
#include "haversine_replacement.cpp"
#else
#include "listing_0065_haversine_formula.cpp"
#endif

#define EARTH_RADIUS 6372.8

#define DOUBLE_EPSILON 0.00000001

char *read_json_file(const char *input_file_path) {
  FILE *input_file_pointer = fopen(input_file_path, "r");
  if (!input_file_pointer) {
    printf("Could not open `%s`\n", input_file_path);
    return NULL;
  }
  fseek(input_file_pointer, 0, SEEK_END);
  u64 input_file_size = ftell(input_file_pointer);
  fseek(input_file_pointer, 0, SEEK_SET);

  char *input_content;
  {
    TIMED_BLOCK("Allocate mem for reading json");
#if USE_HUGETLB
    input_content = (char *)mmap(0, input_file_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
#else
    input_content = (char *)mmap(0, input_file_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

#define PRETOUCH_FILE_BUFFER 0
#if PRETOUCH_FILE_BUFFER
    u64 page_size = 4 * 1024;
    for (u64 i = 0; i < input_file_size; i += page_size) {
      input_content[i] = 0;
    }
#endif

  }
  {
    TIMED_BLOCK_BANDWIDTH("Read JSON file", input_file_size);
#if 0
    fread(input_content, input_file_size, 1, input_file_pointer);
    fclose(input_file_pointer);
#else
    u64 input_buffer_size = 2 * 1024 * 1024;
    u64 bytes_remaining = input_file_size;
    char *buffer_pointer = input_content;
    while (bytes_remaining > input_buffer_size) {
      fread(buffer_pointer, input_buffer_size, 1, input_file_pointer);
      bytes_remaining -= input_buffer_size;
      buffer_pointer += input_buffer_size;
    }
    fread(buffer_pointer, bytes_remaining, 1, input_file_pointer);
    fclose(input_file_pointer);
#endif
  }
  return input_content;
}

char *read_json_file_2mb_chunks(const char *input_file_path) {
  FILE *input_file_pointer = fopen(input_file_path, "r");
  if (!input_file_pointer) {
    printf("Could not open `%s`\n", input_file_path);
    return NULL;
  }
  fseek(input_file_pointer, 0, SEEK_END);
  u64 input_file_size = ftell(input_file_pointer);
  fseek(input_file_pointer, 0, SEEK_SET);

  u64 input_buffer_size = 2 * 1024 * 1024;
  char *input_content;
  {
    TIMED_BLOCK("Allocate mem for reading json");
    input_content =
        (char *)mmap(0, input_buffer_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (input_content == MAP_FAILED) {
      printf("mmap failed with: '%s'\n", strerror(errno));
      printf("Falling back to small pages\n");
      input_content = (char *)mmap(0, input_buffer_size, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (input_content == MAP_FAILED) {
        printf("mmap failed with: '%s'\n", strerror(errno));
      }
    }
  }
  {
    TIMED_BLOCK_BANDWIDTH("Read JSON file", input_file_size);
    u64 bytes_remaining = input_file_size;
    while (bytes_remaining > input_buffer_size) {
      fread(input_content, input_buffer_size, 1, input_file_pointer);
      bytes_remaining -= input_buffer_size;
    }
    fread(input_content, bytes_remaining, 1, input_file_pointer);
    fclose(input_file_pointer);
  }
  return input_content;
}

int main(int argc, char *argv[]) {
  Timing::begin_profiler();

  const int CLI_EXPECTED_ARG_COUNT = 2;
  if (argc < CLI_EXPECTED_ARG_COUNT + 1) {
    printf("Usage: haversine_process <json_file> <answers_file>\n");
    return 1;
  }
  for (s32 i = 1; i < argc - CLI_EXPECTED_ARG_COUNT; i++) {
    // NOTE: Space for flag parsing
  }

  char *input_file_name = argv[argc - 2];

  char *answers_file_name = argv[argc - 1];
  FILE *answers_file_pointer = fopen(answers_file_name, "rb");
  if (!answers_file_pointer) {
    printf("Could not open `%s`\n", answers_file_name);
    return 1;
  }

  f64 sum = 0;

  fseek(answers_file_pointer, -sizeof(f64), SEEK_END);
  f64 total_expected_result;
  fread(&total_expected_result, sizeof(f64), 1, answers_file_pointer);

  printf("\nExpected haversine average: %.16f\n", total_expected_result);

  printf("\nProcessing points from JSON file...\n");

#define ONLY_READ_FILE_IN_CHUNKS 0
#if ONLY_READ_FILE_IN_CHUNKS
  char *input_content = read_json_file_2mb_chunks(input_file_name);
#else
  char *input_content = read_json_file(input_file_name);
  JSON::Header *json_object;
  JSON::Header *pairs_array;
  JSON::Header *point_object;
  {
    TIMED_BLOCK("JSON Parse");
    json_object = JSON::parse(input_content, ((u64)1 << 32));
  }
  {
    TIMED_BLOCK("Access JSON members");
    pairs_array = JSON::get_value_header(json_object, "pairs");
    point_object = JSON::get_array_elem_header(pairs_array, 0);
  }

  if (point_object) {
    TIMED_BLOCK_BANDWIDTH("Haversine compute",
                          pairs_array->count * sizeof(f64) * 4);
    for (u32 point_index = 0; point_index < pairs_array->count; point_index++) {
      f64 f1 = JSON::get_value_f64(point_object, "x0");
      f64 f2 = JSON::get_value_f64(point_object, "y0");
      f64 f3 = JSON::get_value_f64(point_object, "x1");
      f64 f4 = JSON::get_value_f64(point_object, "y1");
      f64 haversine_result = ReferenceHaversineR2(f1, f2, f3, f4, EARTH_RADIUS);
      sum += haversine_result;
      point_object = point_object->next_header;

#define USE_ANSWERS_FILE 0
#if USE_ANSWERS_FILE
      fseek(answers_file_pointer, point_index * sizeof(f64), SEEK_SET);
      f64 expected_result;
      fread(&expected_result, sizeof(f64), 1, answers_file_pointer);
      if (haversine_result - expected_result > DOUBLE_EPSILON) {
        throw std::runtime_error(
            "Haversine result doesn not match answer at point index `" +
            std::to_string(point_index) + "`. Answer was `" +
            std::to_string(expected_result) + "`. Actual result was `" +
            std::to_string(haversine_result) + "`");
      }
#endif
    }
  }
  {
    TIMED_BLOCK("Reporting");

    f64 haversine_average = sum * 2 * EARTH_RADIUS / (f64)pairs_array->count;
    printf("\nAverage of the haversine sum: %.16f\n", haversine_average);
  }

#endif
  Timing::print_profiling();

  printf("\n");
  return 0;
}
