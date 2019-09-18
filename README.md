# eventbus

`eventbus` is a simple, header only C++17 event bus library that doesn't require you to inherit from any sort of `event` class.

## Introduction

`eventbus` implements the so called "Mediator" pattern. This pattern is useful when you want components to communicate to each other without necessary "knowning" about each other. This can be useful in *some* situations but should be used with caution (there are alternative design patterns to consider). 

## Motivation

There are many event bus implementations; so why another one? All the implmentations that I've seen (in C++) require the consumer of the library to inherit from a base `Event` class. Mean that if you want to use your own custom event class, you have to so something like:

````cpp
class MyEvent : public Event
{
    // class info / properties and what not
};
````

To me, this is undesireable, so I set out to see if it was possible to implement something similar, but remove the requirement of inheriting from a base `event` type. 

After going through some training at CppCon2019, I wanted to apply my newfound knowledge and this project was a perfect oppurtunity.

## Usage

### Getting the Library

To get the library, you can simply copy the source to your project and include it as a sub-directory (or a git submodule). Alternatively, you can use something like [CPM](https://github.com/TheLartians/CPM) which is based on CMakes `Fetch_Content` module.

### Using the Library

The libary provides an `event_bus` class. You can register event handlers like so:

````cpp
event_bus::register_handler<event_type>(&event_callback)
````

Where `event_type` is your "event" type (any type!) and `&event_callback` is a reference to a function. You can also use a lamba or a class member function* (:construction: curently under construction :construction:). 

A more complete example:

````cpp
#include <iostream>
#include <string>

#include <eventbus/event_bus.hpp>

struct my_event
{
    int id;
    std::string message;
};

void function_callback(const my_event& evt)
{
    std::cout << "got event: " << evt.message << std::endl;
}

int main()
{
    event_bus::register_handler<my_event>(&evt);

    event_bus::register_handler<my_event>(
        [](const my_event& evt)
        {
            std::cout << "got event lambda: " << evt.message <<std::endl;
        }
    );

    // fire events
    my_event evt{2, "my_event message"};
    event_bus::fire_event(evt);

    return 0;
}
````

## Limitations

In general, all callback functions **must** return `void` and **must** accept the event type parameter as a const reference (i.e. `const T&`). Currently, `eventbus` only supports single argument functions as callbacks, but this may be improved upon at a later time. 

### Class Member Functions

Under the hood, `event_bus` uses an internal `callable` class that type erases (in a way) the arguments to an `std::function` object. Currently, you can use `std::bind()` to get a valid `std::function` that can be registered with the event bus. 

Example:
````cpp
struct my_event
{
    int id;
    std::string;
};

class my_callback
{
    public:
        my_callback() = default;
        void on_my_event(const my_event& evt)
        {
            std::cout << "got callback in class: " << evt.message
                << std::endl;
        }
};

int main()
{
    my_callback callback;
    std::function<void(const my_event&)> class_callback = 
        std::bind(&my_callback::on_my_event, &callback, std::placeholders::_1);
    event_bus::register_handler<my_event>(callback);

    event_bus::fire_event(my_event{3, "my_event"});
}
````

This is a bit convoluted, but can be improved upon.

### Header Only

The library is header only, but `event_bus` is implemented as a singleton. This will pose problems if you need the same singleton instance across application/`dll` boundaries. 