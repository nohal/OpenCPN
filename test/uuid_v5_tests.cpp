#include <gtest/gtest.h>

#include "model/uuid_v5.h"

TEST(UUIDv5, DNSNamespace) {
  // uuid -v 5 6ba7b810-9dad-11d1-80b4-00c04fd430c8 opencpn.org
  // f8de1cfa-f30a-5484-9456-4cf3e15699c0
  struct TestCase {
    std::string namespace_uuid_str;
    std::string name;
    std::string expected;
  };
  std::vector<TestCase> test_cases = {
      {"6ba7b810-9dad-11d1-80b4-00c04fd430c8", "opencpn.org",
       "f8de1cfa-f30a-5484-9456-4cf3e15699c0"},
  };
  for (const auto& test_case : test_cases) {
    std::string u =
        UUIDv5::uuid_v5(test_case.namespace_uuid_str, test_case.name);
    EXPECT_EQ(u, test_case.expected)
        << "Namespace: " << test_case.namespace_uuid_str
        << " Name: " << test_case.name << " Expected: " << test_case.expected
        << " Got: " << u;
  }
}

TEST(UUIDv5, CustomNamespace) {
  struct TestCase {
    std::string namespace_uuid_str;
    std::string name;
    std::string expected;
  };
  std::vector<TestCase> test_cases = {
      {"f8de1cfa-f30a-5484-9456-4cf3e15699c0", "test/31.2345/-32.34456",
       "39d36552-3608-5ff2-b402-b4ce1b1fceff"},
      {"f8de1cfa-f30a-5484-9456-4cf3e15699c0", "test waypoint name",
       "599444d6-4a95-5750-a241-8a81e9a7899f"},
  };
  for (const auto& test_case : test_cases) {
    std::string u =
        UUIDv5::uuid_v5(test_case.namespace_uuid_str, test_case.name);
    EXPECT_EQ(u, test_case.expected)
        << "Namespace: " << test_case.namespace_uuid_str
        << " Name: " << test_case.name << " Expected: " << test_case.expected
        << " Got: " << u;
  }
}
