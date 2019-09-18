#include <iostream>
#include <functional>
#include <any>
#include <map>
#include <typeinfo>
#include <typeindex>

#include <eventbus/event_bus.hpp>

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
	int get_event_count() { return event_count_; }
private:
	int event_count_{ 0 };
};

int main()
{
	my_callback_object callback_obj;

	event_bus::register_handler<dummy_event>(&foo);
	event_bus::register_handler<other_event>([](const other_event& evt) {std::cout << "event_bus: " << evt.message << std::endl; });
	event_bus::register_handler<other_event>([](const other_event& evt){std::cout << "event bus other event: " << evt.id << " " << evt.message << std::endl;});
	event_bus::register_handler<dummy_event>([](const dummy_event& evt) {std::cout << "wow it works!" << std::endl;});
	event_bus::register_handler<dummy_event>(
		(std::function<void(const dummy_event&)>)std::bind(&my_callback_object::on_event_fired, &callback_obj, std::placeholders::_1)
	);
	
	other_event other_evt{ 2, "hello there" };
	dummy_event dmy_event{ "oh boy..." };

	event_bus::fire_event(dmy_event);
	event_bus::fire_event(dmy_event);
	event_bus::fire_event(dmy_event);
	event_bus::fire_event(other_evt);
	event_bus::fire_event(third_event{});

	std::cout << "callback count: " << callback_obj.get_event_count() << std::endl;
    return 0;
}