#include <chrono>
#include <cstdio>
#include <ctime>
#include <random>
#include <ratio>

#include "haversine_types.h"

#include "listing_0065_haversine_formula.cpp"

#define EARTH_RADIUS 6372.8

struct TimeMeasurement {
    std::chrono::time_point<std::chrono::steady_clock> start_wall;
    std::chrono::time_point<std::chrono::steady_clock> end_wall;
    std::clock_t start_cpu;
    std::clock_t end_cpu;
    f64 elapsed_wall_ms;
    f64 elapsed_cpu_ms;
};

std::mt19937_64 engine;
std::uniform_real_distribution<f64> global_dist(-180L, 180L);
std::uniform_real_distribution<f64> dist(-180L, 180L);

void timing_start(TimeMeasurement *timings) {
    timings->start_cpu = std::clock();
    timings->start_wall = std::chrono::steady_clock::now();
}

void timing_end(TimeMeasurement *timings) {
    timings->end_cpu = std::clock();
    timings->end_wall = std::chrono::steady_clock::now();
    timings->elapsed_wall_ms = std::chrono::duration<f64, std::milli>(timings->end_wall - timings->start_wall).count();
    timings->elapsed_cpu_ms = 1000.0 * (timings->end_cpu - timings->start_cpu) / CLOCKS_PER_SEC;
}


void setup_rand(u32 seed) {
    engine.seed(seed);
}

void relocate_distribution() {
    f64 f1 = global_dist(engine);
    f64 f2 = global_dist(engine);
    if (f1 < f2) {
        dist = std::uniform_real_distribution<f64>(f1, f2);
    } else {
        dist = std::uniform_real_distribution<f64>(f2, f1);
    }
}

f64 get_random_f64() {
    return dist(engine);
}

int main(int argc, char *argv[]) {
    const int CLI_EXPECTED_ARG_COUNT = 2;
    if (argc < CLI_EXPECTED_ARG_COUNT + 1) {
        printf("Usage: haversine_json_generator <points_quantity> <seed>\n");
        return 1;
    }
    for (s32 i = 1; i < argc - CLI_EXPECTED_ARG_COUNT; i++) {
        // NOTE: Space for flag parsing
    }

    s64 seed = atoi(argv[argc - 1]);
    if (!seed) {
        printf("Invalid seed\n");
        return 1;
    }
    s64 points_to_generate = atoi(argv[argc - 2]);
    if (points_to_generate < 1) {
        printf("Invalid points_quantity\n");
        return 1;
    }

    setup_rand(seed);
    char json_output_filename[128];
    sprintf(json_output_filename, "haversine_%ld_points.json", points_to_generate);
    char f64_output_filename[128];
    sprintf(f64_output_filename, "haversine_%ld_points.f64", points_to_generate);

    FILE *json_fp = fopen(json_output_filename, "wb");
    FILE *f64_fp = fopen(f64_output_filename, "wb");

    fprintf(json_fp, "{\"pairs\":[\n");

    f64 sum = 0;
    u64 base_checkpoint = points_to_generate / 10;
    u64 base_percentage_done = 10;
    u64 next_checkpoint = base_checkpoint;
    u64 next_percentage_done = base_percentage_done;

    printf("\n\nProcessing %lu points...\n", points_to_generate);
    TimeMeasurement timings {};
    timing_start(&timings);
    for (u32 point_index = 0; point_index < points_to_generate; point_index++) {
        f64 f1 = get_random_f64();
        f64 f2 = get_random_f64();
        f64 f3 = get_random_f64();
        f64 f4 = get_random_f64();
        f64 haversine_result = ReferenceHaversine(f1, f2, f3, f4, EARTH_RADIUS);
        sum += haversine_result;
        fwrite(&haversine_result, sizeof(haversine_result), 1, f64_fp);
        fprintf(json_fp, "  {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f},\n", f1, f2, f3, f4);
        if (point_index == next_checkpoint) {
            relocate_distribution();
            printf("\t%3lu%% done (%lu/%lu)\n", next_percentage_done, next_checkpoint, points_to_generate);
            next_checkpoint += base_checkpoint;
            next_percentage_done += base_percentage_done;
        }
    }
    fseek(json_fp, -2, SEEK_CUR);
    fprintf(json_fp, "\n]}\n");

    fwrite(&sum, sizeof(sum), 1, f64_fp);
    timing_end(&timings);

    f64 haversine_average = sum / (f64)points_to_generate;
    printf("\nAverage of the haversine sum: %.16f\n", haversine_average);

    printf("Total elapsed time: (CPU: %.4f ms | Wall: %.4f ms)\n", timings.elapsed_cpu_ms, timings.elapsed_wall_ms);

    printf("\n");
    return 0;
}
