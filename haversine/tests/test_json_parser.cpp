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
    JSON::Header *json_data = JSON::parse(json_input);
    JSON::Type entire_data_type = JSON::get_type(json_data);
    ASSERT(entire_data_type == JSON::OBJECT);

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
    JSON::Header *json_data = JSON::parse(json_input);
    JSON::Type entire_data_type = JSON::get_type(json_data);
    ASSERT(entire_data_type == JSON::OBJECT);

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
    JSON::Header *json_data = JSON::parse(json_input);
    JSON::Type entire_data_type = JSON::get_type(json_data);
    ASSERT(entire_data_type == JSON::OBJECT);

    // char **keys = JSON::keys(json_data);
    // JSON::Type age_type = JSON::get_value_type(json_data, "age");
    // ASSERT(age_type == JSON::NUMERIC);
    JSON::Header *nested_header = JSON::get_value_header(json_data, "nested");
    JSON::Header *pairs_header = JSON::get_value_header(nested_header, "pairs");
    f64 sum = 0;
    JSON::Header *array_value_header = pairs_header->next_inner_header;
    for (u32 i = 0; i < pairs_header->count; i++) {
        sum += JSON::get_value_f64(array_value_header, "x0");
        sum += JSON::get_value_f64(array_value_header, "x1");
        array_value_header = array_value_header->next_header;
    }
    ASSERT((sum - 130.61687855L) < 0.0001);
    return 0;
}

int test_parser_4() {
    const char *json_input {"{\"array\": [1, 2, 3, 44, 5]}"};
    JSON::Header *json_data = JSON::parse(json_input);
    JSON::Header *array_object = JSON::get_value_header(json_data, "array");
    ASSERT(array_object->type == JSON::ARRAY);

    u64 array_elem = JSON::get_array_elem_u64(array_object, 3);
    ASSERT(array_elem == 44);
    return 0;
}

int test_parser_5() {
    const char *json_input {"{\"1\": {}}"};
    JSON::Header *json_data = JSON::parse(json_input);
    JSON::Header *empty_obj = JSON::get_value_header(json_data, "1");
    ASSERT(empty_obj->size == 0);
    ASSERT(empty_obj->count == 0);
    return 0;
}

int test_parser_6() {
    const char *json_input {"[]"};
    JSON::Header *json_data = JSON::parse(json_input);
    ASSERT(json_data->size == 0);
    ASSERT(json_data->count == 0);
    return 0;
}

int test_parser_7() {
    const char *json_input {"[1, 2, {\"first\": \"key\", \"second\": [{\"wow\": -3.47}, \"test\", 42], \"another\": \"key after the array\"}, 4, 5]"};
    JSON::Header *json_data = JSON::parse(json_input);
    JSON::Header *inner_obj = JSON::get_array_elem_header(json_data, 2);
    JSON::Header *inner_array = JSON::get_value_header(inner_obj, "second");

    JSON::Header *double_nested_object = JSON::get_array_elem_header(inner_array, 0);
    ASSERT(JSON::get_value_f64(double_nested_object, "wow") - (-3.47L) < 0.00001);
    ASSERT(!(strcmp(JSON::get_array_elem_string(inner_array, 1), "test")));
    ASSERT(JSON::get_array_elem_u64(inner_array, 2) == 42);
    ASSERT(!(strcmp(JSON::get_value_string(inner_obj, "another"), "key after the array")));
    ASSERT(JSON::get_array_elem_u64(json_data, 3) == 4);
    ASSERT(JSON::get_array_elem_u64(json_data, 4) == 5);

    return 0;
}

TestProtocol tests_to_run[] = {
    test_parser_1,
    test_parser_2,
    test_parser_3,
    test_parser_4,
    test_parser_5,
    test_parser_6,
    test_parser_7,
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

