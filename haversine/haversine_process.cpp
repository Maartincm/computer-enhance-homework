#include <cstdio>
#include <stdexcept>

#include "haversine_types.h"

#include "haversine_json_parser.cpp"
#include "haversine_timing.cpp"
#include "listing_0065_haversine_formula.cpp"

#define EARTH_RADIUS 6372.8

#define DOUBLE_EPSILON 0.00000001

int main(int argc, char *argv[]) {
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
  u64 base_checkpoint = 100000;
  u64 next_checkpoint = base_checkpoint;

  fseek(answers_file_pointer, -sizeof(f64), SEEK_END);
  f64 total_expected_result;
  fread(&total_expected_result, sizeof(f64), 1, answers_file_pointer);

  printf("\nExpected haversine average: %.16f\n", total_expected_result);

  printf("\n\nProcessing points from JSON file...\n");
  TimeMeasurement timings{};
  timing_start(&timings);
  TimeMeasurement json_load_timings = timings;
  TimeMeasurement batch_timings;
  TimeMeasurement haversine_compute_timings;

  char *input_content = (char *)malloc(input_file_size);
  fread(input_content, input_file_size, 1, input_file_pointer);
  fclose(input_file_pointer);
  JSON::Header *json_object = JSON::parse(input_content, ((u64)1 << 32));
  JSON::Header *pairs_array = JSON::get_value_header(json_object, "pairs");
  JSON::Header *point_object = JSON::get_array_elem_header(pairs_array, 0);
  timing_end(&json_load_timings);

  timing_start(&batch_timings);
  haversine_compute_timings = batch_timings;
  for (u32 point_index = 0; point_index < pairs_array->count; point_index++) {
    f64 f1 = JSON::get_value_f64(point_object, "x0");
    f64 f2 = JSON::get_value_f64(point_object, "y0");
    f64 f3 = JSON::get_value_f64(point_object, "x1");
    f64 f4 = JSON::get_value_f64(point_object, "y1");
    f64 haversine_result = ReferenceHaversine(f1, f2, f3, f4, EARTH_RADIUS);
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
    sum += haversine_result;
    if (point_index == next_checkpoint) {
      timing_end(&batch_timings);
      printf("\tPoints processed: %lu | This batch took: (CPU: %.4f ms | Wall: "
             "%.4f ms)\n",
             next_checkpoint, batch_timings.elapsed_cpu_ms,
             batch_timings.elapsed_wall_ms);
      next_checkpoint += base_checkpoint;
      timing_start(&batch_timings);
    }
    point_object = point_object->next_header;
  }
  timing_end(&haversine_compute_timings);
  timing_end(&timings);

  f64 haversine_average = sum / (f64)pairs_array->count;
  printf("\nAverage of the haversine sum: %.16f\n", haversine_average);

  printf("\nTotal elapsed time: (CPU: %.4f ms | Wall: %.4f ms)\n",
         timings.elapsed_cpu_ms, timings.elapsed_wall_ms);
  printf("  Seggregated timings:\n");
  printf("    JSON Parse       : (CPU: %.4f ms | Wall: %.4f ms)\n",
         json_load_timings.elapsed_cpu_ms, json_load_timings.elapsed_wall_ms);
  printf("    Haversine compute: (CPU: %.4f ms | Wall: %.4f ms)\n",
         haversine_compute_timings.elapsed_cpu_ms, haversine_compute_timings.elapsed_wall_ms);

  printf("\n");
  return 0;
}
