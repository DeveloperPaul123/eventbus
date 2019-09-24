#include <eventbus/event_bus.hpp>
#include <atomic>

#include <gtest/gtest.h>

struct test_event_type
{
	int id{ -1 };
	std::string event_message{ "" };
	double data_value{ 1.0 };
};

inline std::ostream& operator<<(std::ostream& out, const test_event_type &evt)
{
	out << "id: " << evt.id << " msg: " << evt.event_message << " data: " << evt.data_value;
	return out;
}

class event_handler_counter
{
	std::atomic<unsigned int> event_count_ = 0;
public:
	event_handler_counter() = default;
	[[nodiscard]] unsigned int get_count() const { return event_count_.load(); };
	void on_test_event()
	{
		++event_count_;
	}
};

class EventBusTestFixture : public ::testing::Test
{
protected:
	event_handler_counter counter;
	std::type_index counter_event_registration_{ typeid(&event_handler_counter::on_test_event) };
	
	void SetUp() override
	{
		counter_event_registration_ = dp::event_bus::register_handler<test_event_type>(&counter, &event_handler_counter::on_test_event);
	}

	void TearDown() override
	{
		dp::event_bus::remove_handler(counter_event_registration_);
	}
};

void free_function_callback(const test_event_type &type_event)
{
	std::cout << "Free function callback : " << type_event << "\n";
}

TEST_F(EventBusTestFixture, LambdaRegistrationAndDeregistration)
{
	test_event_type test_event{ 1, "event message", 32.56 };
	const auto lambda_one_reg = dp::event_bus::register_handler<test_event_type>([]() {std::cout << "Lambda 1\n"; });
	const auto lambda_two_reg = dp::event_bus::register_handler<test_event_type>([&test_event](const test_event_type& evt)
		{
			EXPECT_EQ(evt.id, test_event.id);
			EXPECT_EQ(evt.event_message, test_event.event_message);
			EXPECT_EQ(evt.data_value, test_event.data_value);
		});

	const auto lambda_three_reg = dp::event_bus::register_handler<test_event_type>([](test_event_type)
		{
			std::cout << "Lambda 3 take by copy.\n";
		});

	// should be 4 because we register a handler in the test fixture SetUp
	ASSERT_EQ(dp::event_bus::handler_count(), 4);
	dp::event_bus::fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 1);
	dp::event_bus::fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 2);

	dp::event_bus::remove_handler(lambda_one_reg);

	dp::event_bus::fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 3);
	EXPECT_EQ(dp::event_bus::handler_count(), 3);

	dp::event_bus::remove_handler(lambda_two_reg);

	dp::event_bus::fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 4);
	EXPECT_EQ(dp::event_bus::handler_count(), 2);

	dp::event_bus::remove_handler(lambda_three_reg);

	dp::event_bus::fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 5);
	EXPECT_EQ(dp::event_bus::handler_count(), 1);
}