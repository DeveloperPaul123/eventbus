#include <iostream>
#include <typeindex>

#include <eventbus/event_bus.hpp>

const char new_line = '\n';

struct dummy_event
{
    std::string message;
};

struct other_event
{
	int id;
	std::string message;
};

struct third_event
{
	double value{ 234.00 };
};

void foo(const dummy_event& evt)
{
	std::cout << "event: " << evt.message << std::endl;
}

class my_callback_object
{
public:
	my_callback_object() = default;
	void on_event_fired(const dummy_event& evt) { event_count_++; }
	void on_third_event() {};
	[[nodiscard]] int get_event_count() const { return event_count_; }
private:
	int event_count_{ 0 };
};

class third_event_object
{
public:
	void on_third_event() {};
};

int main()
{
	using namespace dp;
	my_callback_object callback_obj;

	auto fo = &foo;
	const auto reg = event_bus::register_handler<dummy_event>(&foo);
	const auto third_event_reg = event_bus::register_handler<third_event>([](const third_event& evt)
	{
			std::cout << "my third event handler: " << evt.value << new_line;
	});

	const auto empty_event_handler = event_bus::register_handler<third_event>([]() {std::cout << "I just do stuff when a third_event is fired." << new_line; });
	
	dummy_event evt{ "hello from dummy event" };
	event_bus::fire_event(&evt);
	event_bus::fire_event(third_event{ 13.0 });
	event_bus::remove_handler(third_event_reg);
	event_bus::fire_event(third_event{ 13.0 });
	
	const auto other_event_reg = event_bus::register_handler<other_event>([](const other_event& evt) {std::cout << "first other event handler says: " << evt.message << std::endl; });
	const auto other_event_second_reg = event_bus::register_handler<other_event>([](const other_event& evt){std::cout << "second other event handler says: " << evt.id << " " << evt.message << std::endl;});
	const auto dmy_evt_first_reg = event_bus::register_handler<dummy_event>([](const dummy_event& evt) {std::cout << "wow it works!" << std::endl;});
	const auto dmy_evt_pmr_reg = event_bus::register_handler<dummy_event>(&callback_obj , &my_callback_object::on_event_fired);
	const auto thrid_event_reg_pmr = event_bus::register_handler<third_event>(&callback_obj, &my_callback_object::on_third_event);

	// the following does not compile
	// third_event_object teo;
	// const auto rg = event_bus::register_handler<third_event>(&teo, &my_callback_object::on_third_event);
	
	other_event other_evt{ 2, "hello there" };
	dummy_event dmy_event{ "oh boy..." };
	
	event_bus::fire_event(dmy_event);

	std::cout << "Firing other event\n";
	event_bus::fire_event(other_evt);
	event_bus::remove_handler(other_event_reg);
	std::cout << "Firing other event\n";
	event_bus::fire_event(other_evt);
	event_bus::fire_event(third_event{});
	
	std::cout << "callback count: " << callback_obj.get_event_count() << std::endl;
    return 0;
}