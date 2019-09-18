#include <eventbus/utils/callable.hpp>

#include <gtest/gtest.h>

#include <string>

struct test_obj
{
	int id;
	std::string message;
};

void free_func(const test_obj& obj, int call_id)
{
	std::cout << "free_func: " << obj.id << " : " << obj.message << " call id: " << call_id << std::endl;
}

bool free_func_bool(int id)
{
	return id == 1;
}

TEST(callable_tests, use_free_functions)
{
	callable<void> free_func_test(&free_func);

	EXPECT_ANY_THROW(free_func_test(3.2f));
	EXPECT_ANY_THROW(free_func_test(3, test_obj{}));

	test_obj obj{};
	int id = 4;
	EXPECT_NO_THROW(free_func_test(obj, id));

	callable<bool> free_func_bool(&free_func_bool);
	auto result = free_func_bool(4);
	EXPECT_FALSE(result);
}

TEST(callable_tests, use_lambdas)
{
	callable<void> lambda([](test_obj value) {std::cout 
		<< "lambda: " << value.message << std::endl; });

	test_obj obj{};
	EXPECT_NO_THROW(lambda(test_obj{}));
	// EXPECT_NO_THROW(lambda(obj));
}