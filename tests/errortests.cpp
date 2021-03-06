#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <set>
#include <string_view>

#include "simdjson.h"

using namespace simdjson;
using namespace std;

#ifndef SIMDJSON_BENCHMARK_DATA_DIR
#define SIMDJSON_BENCHMARK_DATA_DIR "jsonexamples/"
#endif
const char *TWITTER_JSON = SIMDJSON_BENCHMARK_DATA_DIR "twitter.json";

#define TEST_START() { cout << "Running " << __func__ << " ..." << endl; }
#define ASSERT_ERROR(ACTUAL, EXPECTED) if ((ACTUAL) != (EXPECTED)) { cerr << "FAIL: Unexpected error \"" << (ACTUAL) << "\" (expected \"" << (EXPECTED) << "\")" << endl; return false; }
#define TEST_FAIL(MESSAGE) { cerr << "FAIL: " << (MESSAGE) << endl; return false; }
#define TEST_SUCCEED() { return true; }
namespace parser_load {
  const char * NONEXISTENT_FILE = "this_file_does_not_exist.json";
  bool parser_load_capacity() {
    TEST_START();
    dom::parser parser(1); // 1 byte max capacity
    auto error = parser.load(TWITTER_JSON).error();
    ASSERT_ERROR(error, CAPACITY);
    TEST_SUCCEED();
  }
  bool parser_load_many_capacity() {
    TEST_START();
    dom::parser parser(1); // 1 byte max capacity
    for (auto doc : parser.load_many(TWITTER_JSON)) {
      ASSERT_ERROR(doc.error(), CAPACITY);
      TEST_SUCCEED();
    }
    TEST_FAIL("No documents returned");
  }

  bool parser_parse_many_documents_error_in_the_middle() {
    TEST_START();
    const padded_string DOC = "1 2 [} 3"_padded;
    size_t count = 0;
    dom::parser parser;
    for (auto doc : parser.parse_many(DOC)) {
      count++;
      auto [val, error] = doc.get<uint64_t>();
      if (count == 3) {
        ASSERT_ERROR(error, TAPE_ERROR);
      } else {
        if (error) { TEST_FAIL(error); }
        if (val != count) { cerr << "FAIL: expected " << count << ", got " << val << endl; return false; }
      }
    }
    if (count != 3) { cerr << "FAIL: expected 2 documents and 1 error, got " << count << " total things" << endl; return false; }
    TEST_SUCCEED();
  }

  bool parser_parse_many_documents_partial() {
    TEST_START();
    const padded_string DOC = "["_padded;
    size_t count = 0;
    dom::parser parser;
    for (auto doc : parser.parse_many(DOC)) {
      count++;
      ASSERT_ERROR(doc.error(), TAPE_ERROR);
    }
    if (count != 1) { cerr << "FAIL: expected no documents and 1 error, got " << count << " total things" << endl; return false; }
    TEST_SUCCEED();
  }

  bool parser_parse_many_documents_partial_at_the_end() {
    TEST_START();
    const padded_string DOC = "1 2 ["_padded;
    size_t count = 0;
    dom::parser parser;
    for (auto doc : parser.parse_many(DOC)) {
      count++;
      auto [val, error] = doc.get<uint64_t>();
      if (count == 3) {
        ASSERT_ERROR(error, TAPE_ERROR);
      } else {
        if (error) { TEST_FAIL(error); }
        if (val != count) { cerr << "FAIL: expected " << count << ", got " << val << endl; return false; }
      }
    }
    if (count != 3) { cerr << "FAIL: expected 2 documents and 1 error, got " << count << " total things" << endl; return false; }
    TEST_SUCCEED();
  }

  bool parser_load_nonexistent() {
    TEST_START();
    dom::parser parser;
    auto error = parser.load(NONEXISTENT_FILE).error();
    ASSERT_ERROR(error, IO_ERROR);
    TEST_SUCCEED();
  }
  bool parser_load_many_nonexistent() {
    TEST_START();
    dom::parser parser;
    for (auto doc : parser.load_many(NONEXISTENT_FILE)) {
      ASSERT_ERROR(doc.error(), IO_ERROR);
      TEST_SUCCEED();
    }
    TEST_FAIL("No documents returned");
  }
  bool padded_string_load_nonexistent() {
    TEST_START();
    auto error = padded_string::load(NONEXISTENT_FILE).error();
    ASSERT_ERROR(error, IO_ERROR);
    TEST_SUCCEED();
  }

  bool parser_load_chain() {
    TEST_START();
    dom::parser parser;
    auto error = parser.load(NONEXISTENT_FILE)["foo"].get<uint64_t>().error();
    ASSERT_ERROR(error, IO_ERROR);
    TEST_SUCCEED();
  }
  bool parser_load_many_chain() {
    TEST_START();
    dom::parser parser;
    for (auto doc : parser.load_many(NONEXISTENT_FILE)) {
      auto error = doc["foo"].get<uint64_t>().error();
      ASSERT_ERROR(error, IO_ERROR);
      TEST_SUCCEED();
    }
    TEST_FAIL("No documents returned");
  }
  bool run() {
    return true
        && parser_load_capacity()
        && parser_load_many_capacity()
        && parser_load_nonexistent()
        && parser_load_many_nonexistent()
        && padded_string_load_nonexistent()
        && parser_load_chain()
        && parser_load_many_chain()
        && parser_parse_many_documents_error_in_the_middle()
        && parser_parse_many_documents_partial()
        && parser_parse_many_documents_partial_at_the_end()
    ;
  }
}

int main() {
  // this is put here deliberately to check that the documentation is correct (README),
  // should this fail to compile, you should update the documentation:
  if (simdjson::active_implementation->name() == "unsupported") { 
    printf("unsupported CPU\n"); 
  }
  std::cout << "Running error tests." << std::endl;
  if (!parser_load::run()) {
    return EXIT_FAILURE;
  }
  std::cout << "Error tests are ok." << std::endl;
  return EXIT_SUCCESS;
}
