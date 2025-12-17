#include <unity.h>
#include <unity_fixture.h>

TEST_SETUP(exceptions) {}
TEST_TEAR_DOWN(exceptions) {}

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
TEST_GROUP(exceptions);

TEST(exceptions, basic)
{
	try {
		throw std::bad_alloc();
		TEST_ASSERT(false);
	} catch (const std::bad_alloc& e) {
		TEST_ASSERT(true);
	} catch (const std::exception& e) {
		TEST_ASSERT(false);
	} catch (...) {
		TEST_ASSERT(false);
	}
}

TEST(exceptions, rethrow)
{
	auto rethrow = []() {
		try {
			throw std::bad_alloc();
			TEST_ASSERT(false);
		} catch (...) {
			throw;
		}
		TEST_ASSERT(false);
	};

	try {
		rethrow();
		TEST_ASSERT(false);
	} catch (const std::exception& e) {
		TEST_ASSERT(true);
	} catch (...) {
		TEST_ASSERT(false);
	}
}

TEST_GROUP_RUNNER(exceptions)
{
	RUN_TEST_CASE(exceptions, basic);
	RUN_TEST_CASE(exceptions, rethrow);
}
#else
TEST_GROUP_RUNNER(exceptions) {}
#endif