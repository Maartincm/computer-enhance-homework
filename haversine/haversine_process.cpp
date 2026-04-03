#include <cstdio>

#include "haversine_types.h"

#define PROFILE 1

#include "haversine_json_parser.cpp"
#include "haversine_timing.cpp"
#include "listing_0065_haversine_formula.cpp"

#define EARTH_RADIUS 6372.8

#define DOUBLE_EPSILON 0.00000001


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
  FILE *input_file_pointer = fopen(input_file_name, "r");
  if (!input_file_pointer) {
    printf("Could not open `%s`\n", input_file_name);
    return 1;
  }

  char *answers_file_name = argv[argc - 1];
  FILE *answers_file_pointer = fopen(answers_file_name, "rb");
  if (!input_file_pointer) {
    printf("Could not open `%s`\n", answers_file_name);
    return 1;
  }

  fseek(input_file_pointer, 0, SEEK_END);
  u64 input_file_size = ftell(input_file_pointer);
  fseek(input_file_pointer, 0, SEEK_SET);

  f64 sum = 0;

  fseek(answers_file_pointer, -sizeof(f64), SEEK_END);
  f64 total_expected_result;
  fread(&total_expected_result, sizeof(f64), 1, answers_file_pointer);

  printf("\nExpected haversine average: %.16f\n", total_expected_result);

  printf("\nProcessing points from JSON file...\n");

  char *input_content;
  {
    TIMED_BLOCK("Allocate mem for reading json");
    input_content = (char *)malloc(input_file_size);
  }
  {
    TIMED_BLOCK_BANDWIDTH("Read JSON file", input_file_size);
    fread(input_content, input_file_size, 1, input_file_pointer);
    fclose(input_file_pointer);
  }
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
    TIMED_BLOCK_BANDWIDTH("Haversine compute", pairs_array->count * sizeof(f64) * 4);
    for (u32 point_index = 0; point_index < pairs_array->count; point_index++) {
      f64 f1 = JSON::get_value_f64(point_object, "x0");
      f64 f2 = JSON::get_value_f64(point_object, "y0");
      f64 f3 = JSON::get_value_f64(point_object, "x1");
      f64 f4 = JSON::get_value_f64(point_object, "y1");
      f64 haversine_result = ReferenceHaversine(f1, f2, f3, f4, EARTH_RADIUS);
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

    f64 haversine_average = sum / (f64)pairs_array->count;
    printf("\nAverage of the haversine sum: %.16f\n", haversine_average);

  }

  Timing::print_profiling();

  printf("\n");
  return 0;
}
