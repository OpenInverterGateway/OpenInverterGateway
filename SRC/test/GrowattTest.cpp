#include <Arduino.h>
#include <unity.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "Growatt.h"

void test_camelCaseToSnakeCase() {
  // Test with a single word
  char output[20] = "hello";
  Growatt::camelCaseToSnakeCase("Hello", output);
  TEST_ASSERT_EQUAL_STRING("hello", output);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_camelCaseToSnakeCase);
  UNITY_END();
}