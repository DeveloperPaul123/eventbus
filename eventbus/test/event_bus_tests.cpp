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
	dp::handler_registration counter_event_registration_;
	dp::event_bus evt_bus;
	void SetUp() override
	{
		counter_event_registration_ = evt_bus.register_handler<test_event_type>(&counter, &event_handler_counter::on_test_event);
	}

	void TearDown() override
	{
		evt_bus.remove_handler(counter_event_registration_);
	}
};

void free_function_callback(const test_event_type &type_event)
{
	std::cout << "Free function callback : " << type_event << "\n";
}

TEST_F(EventBusTestFixture, LambdaRegistrationAndDeregistration)
{
	test_event_type test_event{ 1, "event message", 32.56 };
	const auto lambda_one_reg = evt_bus.register_handler<test_event_type>([]() {std::cout << "Lambda 1\n"; });
	const auto lambda_two_reg = evt_bus.register_handler<test_event_type>([&test_event](const test_event_type& evt)
		{
			EXPECT_EQ(evt.id, test_event.id);
			EXPECT_EQ(evt.event_message, test_event.event_message);
			EXPECT_EQ(evt.data_value, test_event.data_value);
		});

	const auto lambda_three_reg = evt_bus.register_handler<test_event_type>([](test_event_type)
		{
			std::cout << "Lambda 3 take by copy.\n";
		});

	// should be 4 because we register a handler in the test fixture SetUp
	ASSERT_EQ(evt_bus.handler_count(), 4);
	evt_bus.fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 1);
	evt_bus.fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 2);

	evt_bus.remove_handler(lambda_one_reg);

	evt_bus.fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 3);
	EXPECT_EQ(evt_bus.handler_count(), 3);

	evt_bus.remove_handler(lambda_two_reg);

	evt_bus.fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 4);
	EXPECT_EQ(evt_bus.handler_count(), 2);

	evt_bus.remove_handler(lambda_three_reg);

	evt_bus.fire_event(test_event);
	EXPECT_EQ(counter.get_count(), 5);
	EXPECT_EQ(evt_bus.handler_count(), 1);
}

TEST_F(EventBusTestFixture, RegisterWhileDispatching) 
{
	struct nefarious_event_listener 
	{
		dp::event_bus* evt_bus;
		void on_event(test_event_type evt) 
		{
			nefarious_event_listener listener;
			listener.evt_bus = evt_bus;
			if(evt_bus) 
			{
				evt_bus->register_handler<test_event_type>(&listener, &nefarious_event_listener::on_event);
			}
		}
	};

	dp::handler_registration registration;
	{
		nefarious_event_listener listener;
		listener.evt_bus = &evt_bus;
		registration = evt_bus.register_handler<test_event_type>(&listener, &nefarious_event_listener::on_event);
	}

	for(auto i = 0; i < 40; ++i) {
		evt_bus.fire_event(test_event_type{2, "test event", 1.3});
		std::cout << "Handler count: " << evt_bus.handler_count() << "\n";
		// count should be 2 because we registered the first nefarious object and the 
		// test fixture class is registered as well (the counter).
		ASSERT_EQ(evt_bus.handler_count(), 2);
	}

	ASSERT_TRUE(evt_bus.remove_handler(registration));
}

TEST_F(EventBusTestFixture, DeregisterWhileDispatching)
{

	struct deregister_while_dispatch_listener {
		dp::event_bus *evt_bus{nullptr};
		std::vector<dp::handler_registration> *registrations{nullptr};
		void on_event(test_event_type)
		{
			if(evt_bus && registrations) {
				std::for_each(registrations->begin(), registrations->end(), [&](auto reg) {
					evt_bus->remove_handler(reg);
				});
			}
		}
	};

	std::vector<dp::handler_registration> registrations;
	std::vector<deregister_while_dispatch_listener> listeners;
	for(auto i = 0; i < 20; ++i) {
		deregister_while_dispatch_listener listener;
		auto reg = evt_bus.register_handler<test_event_type>(&listener, &deregister_while_dispatch_listener::on_event);
		listeners.emplace_back(listener);
		registrations.emplace_back(reg);
	}
	
	listeners[0].evt_bus = &evt_bus;
	listeners[0].registrations = &registrations;

	for(auto i = 0; i < 40; ++i) {
		evt_bus.fire_event(test_event_type{3, "test event", 3.4});
		std::cout << "Handler count: " << evt_bus.handler_count() << "\n";
		// add 1 because of the test fixture.
		EXPECT_EQ(evt_bus.handler_count(), listeners.size() + 1);
	}

	// remove all the registrations
	for(auto reg: registrations) {
		EXPECT_TRUE(evt_bus.remove_handler(reg));
	}

	EXPECT_EQ(evt_bus.handler_count(), 1);
}

}