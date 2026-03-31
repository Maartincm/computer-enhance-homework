#include <cstring>
#include <iostream>

#include "../haversine_types.h"

#include "../haversine_json_parser.cpp"

#define ASSERTM(condition, message)                                             \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__   \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            return 1;                                                          \
        }                                                                      \
    } while (false)

#define ASSERT(condition)                                                      \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__   \
                      << " line " << __LINE__ << std::endl;                    \
            return 1;                                                          \
        }                                                                      \
    } while (false)

typedef int (*TestProtocol)();

int test_parser_1() {
    const char *json_input {"{\"name\": \"Juan\"}"};
    void *json_data = JSON::parse(json_input);
    JSON::Type entire_data_type = JSON::get_type(json_data);
    ASSERT(entire_data_type == JSON::OBJECT);

    // char **keys = JSON::keys(json_data);
    // JSON::Type age_type = JSON::get_value_type(json_data, "age");
    // ASSERT(age_type == JSON::NUMERIC);
    char *name = JSON::get_value_string(json_data, "name");
    ASSERT(!strcmp(name, "Juan"));
    return 0;
}

int test_parser_2() {
    const char *json_input {" \
{ \
    \"name\": \"Juan\", \
    \"age\": 23 \
} \
        "};
    // const char *json_input {"{\"name\": \"Juan\", \"age\": 23}"};
    void *json_data = JSON::parse(json_input);
    JSON::Type entire_data_type = JSON::get_type(json_data);
    ASSERT(entire_data_type == JSON::OBJECT);

    // char **keys = JSON::keys(json_data);
    // JSON::Type age_type = JSON::get_value_type(json_data, "age");
    // ASSERT(age_type == JSON::NUMERIC);
    u64 age = JSON::get_value_u64(json_data, "age");
    ASSERT(age == 23);
    return 0;
}

int test_parser_3() {
    const char *json_input {"{\
    \"name\": \"Juan\",\
    \"nested\": {\
        \"first\": \"string\",\
        \"pairs\": [\
            {\"x0\": 13.84133324, \"x1\": -15.44444444},\
            {\"x0\": 11.33333333, \"x1\": 15.44444444},\
            {\"x0\": 5.44221199, \"x1\": 99.99999999}\
        ]\
    },\
    \"end\": 123\
}"};
    void *json_data = JSON::parse(json_input);
    JSON::Type entire_data_type = JSON::get_type(json_data);
    ASSERT(entire_data_type == JSON::OBJECT);

    // char **keys = JSON::keys(json_data);
    // JSON::Type age_type = JSON::get_value_type(json_data, "age");
    // ASSERT(age_type == JSON::NUMERIC);
    JSON::Header *nested_header = JSON::get_value_header(json_data, "nested");
    JSON::Header *pairs_header = JSON::get_value_header(nested_header, "pairs");
    f64 sum = 0;
    JSON::Header *array_value_header = pairs_header->next_header;
    for (u32 i = 0; i < pairs_header->count; i++) {
        sum += JSON::get_value_f64(array_value_header, "x0");
        sum += JSON::get_value_f64(array_value_header, "x1");
        array_value_header = array_value_header->next_header;
    }
    ASSERT((sum - 130.61687855L) < 0.0001);
    return 0;
}

TestProtocol tests_to_run[] = {
    test_parser_1,
    test_parser_2,
    test_parser_3,
};

int main(int argc, char *argv[]) {
    for (u64 i = 0; i < ArrayCount(tests_to_run); i++) {
        b32 result = tests_to_run[i]();
        if (result) {
            printf("Test failed\n");
        } else {
            printf("Test passed\n");
        }
    }
    return 0;
}

