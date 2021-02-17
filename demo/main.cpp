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
    void on_event_fired(const dummy_event&) { event_count_++; }
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
    event_bus evt_bus;
    my_callback_object callback_obj;

    const auto reg = evt_bus.register_handler<dummy_event>(&foo);
    const auto third_event_reg = evt_bus.register_handler<third_event>([](const third_event& evt)
    {
            std::cout << "my third event handler: " << evt.value << new_line;
    });

    const auto empty_event_handler = evt_bus.register_handler<third_event>([]() {std::cout << "I just do stuff when a third_event is fired." << new_line; });
    
    dummy_event evt{ "hello from dummy event" };
    evt_bus.fire_event(&evt);
    evt_bus.fire_event(third_event{ 13.0 });
    evt_bus.remove_handler(third_event_reg);
    evt_bus.fire_event(third_event{ 13.0 });
    
    
    const auto other_event_reg = evt_bus.register_handler<other_event>([](const other_event& other_evt) {std::cout << "first other event handler says: " << other_evt.message << std::endl; });
    const auto other_event_second_reg = evt_bus.register_handler<other_event>([](const other_event& other_evt){std::cout << "second other event handler says: " << other_evt.id << " " << other_evt.message << std::endl;});
    const auto dmy_evt_first_reg = evt_bus.register_handler<dummy_event>([](const dummy_event& dmy_evt) {std::cout << "third event handler says: " << dmy_evt.message << std::endl;});
    const auto dmy_evt_pmr_reg = evt_bus.register_handler<dummy_event>(&callback_obj , &my_callback_object::on_event_fired);
    const auto thrid_event_reg_pmr = evt_bus.register_handler<third_event>(&callback_obj, &my_callback_object::on_third_event);

    // the following does not compile
    // third_event_object teo;
    // const auto rg = evt_bus.register_handler<third_event>(&teo, &my_callback_object::on_third_event);
    
    other_event other_evt{ 2, "hello there" };
    dummy_event dmy_event{ "oh boy..." };
    
    evt_bus.fire_event(dmy_event);

    std::cout << "Firing other event\n";
    evt_bus.fire_event(other_evt);
    evt_bus.remove_handler(other_event_reg);
    std::cout << "Firing other event\n";
    evt_bus.fire_event(other_evt);
    evt_bus.fire_event(third_event{});
    
    std::cout << "callback count: " << callback_obj.get_event_count() << std::endl;
    std::cout << "handler count: " << evt_bus.handler_count() << "\n";
    std::cout << "removing handlers..." << "\n";

    evt_bus.remove_handler(reg);
    evt_bus.remove_handler(other_event_second_reg);
    evt_bus.remove_handler(empty_event_handler);
    evt_bus.remove_handler(dmy_evt_first_reg);
    evt_bus.remove_handler(dmy_evt_pmr_reg);
    evt_bus.remove_handler(thrid_event_reg_pmr);

    return 0;
}