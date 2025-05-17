#ifndef ARDUINO_UNIT_TESTS_H
#define ARDUINO_UNIT_TESTS_H
#define unittest_setup() void unittest_setup()
#define unittest_teardown() void unittest_teardown()
#define unittest(name) void name()
#define unittest_main() int main(){unittest_setup(); test_mock(); unittest_teardown(); return 0;}
#endif // ARDUINO_UNIT_TESTS_H
