# eventbus
`eventbus` is a simple, header only C++17 event bus library that doesn't require you to inherit from any sort of `event` class.

- [Design Goals](#design-goals)
- [Integration](#integration)
    - [CMake](#cmake)
    - [vcpkg](#vcpkg)
  - [Usage](#usage)
    - [Define An Event Object](#define-an-event-object)
    - [Registering Handlers](#registering-handlers)
    - [Firing Events](#firing-events)
- [Work In Progress Items](#work-in-progress-items)
- [Limitations](#limitations)
  - [Header Only](#header-only)
  - [Multithreading](#multithreading)
- [Contributing](#contributing)

## Design Goals

`eventbus` implements the "Mediator" pattern. This pattern is useful when you want components to communicate to each other without necessary "knowning" about each other. This can be useful in *some* situations but should be used with caution (there are alternative design patterns to consider). 

* **Do not require event object inheritance** I wanted to implement an event bus system that doesn't require users to inherit from some base `Event` class in order to use the event class. 
* **Many Callback Types** It's important that the library supports many different types of callbacks including:
  * Lambdas
  * Class member functions
  * Free functions
* **Flexible Callbacks** Callbacks should be able to take no input parameters, the event type by `const&` or by value. 

## Integration

`eventbus` is a header only library. All the files you need are in the `eventbus/include` folder. To use the library just include `eventbus/event_bus.hpp`. 

#### CMake

`eventbus` defines three CMake `INTERFACE` targets that can be used in your project:
* `eventbus`
* `eventbus::eventbus`
* `dp::eventbus`

````cmake
find_package(dp::eventbus REQUIRED)

````

Alternatively, you can use something like [CPM](https://github.com/TheLartians/CPM) which is based on CMakes `Fetch_Content` module.

````cmake
CPMAddPackage(
    NAME eventbus
    GITHUB_REPOSITORY DeveloperPaul123/eventbus
    GIT_TAG #053902d63de5529ee65d965f8b1fb0851eceed24 change this to latest commit
)
````

#### vcpkg 

:construction: This library will be on `vcpkg` soon. :construction:

### Usage

The basic premise of the `event_bus` is that with it, you can:
* Register handlers
* Fire events that call the corresponding handlers

#### Define An Event Object

The "event" object can really be any class or structure you want. This is where the flexibility of this library shines. 

#### Registering Handlers

**Free function**
````cpp
void event_callback(event_type evt)
{
    // event callback logic...
}

dp::event_bus::register_handler<event_type>(&event_callback)
````

**Lambda**
````cpp
dp::event_bus::register_handler<event_type>([](const event_type& evt)
{
    // logic code...
});
````

**Class Member Function**

````cpp
class event_handler
{
    public:
        void on_event(event_type type)
        {
            // handler logic...
        }
};

// other code

event_handler handler;
dp::event_bus::register_handler<event_type>(&handler, &event_handler::on_event);
````

**Note:** You can't mix a class instance of type `T` with the member function of another class (i.e. `&U::function_name`). 

#### Firing Events

````cpp
event_type evt{
    // data and info..
};
dp::event_bus::fire_event(evt); // all connect handler for the given event type will be fired.
````

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
    auto handler_registration = dp::event_bus::register_handler<my_event>(&evt);

    auto lambda_registration = dp::event_bus::register_handler<my_event>(
        [](const my_event& evt)
        {
            std::cout << "got event lambda: " << evt.message <<std::endl;
        }
    );

    // fire events
    my_event evt{2, "my_event message"};
    dp::event_bus::fire_event(evt);

    return 0;
}
````

More example code can be seen in the [demo](https://github.com/DeveloperPaul123/eventbus/tree/develop/demo) project. 

## Work In Progress Items

* Thread safety for all operations. 

## Limitations

In general, all callback functions **must** return `void`. Currently, `eventbus` only supports single argument functions as callbacks, but this may be improved upon at a later time. 

### Header Only

The library is header only, but `event_bus` is implemented as a singleton. This may pose problems if you need the same singleton instance across application/`dll` boundaries. 

### Multithreading

Currently this library is not fully thread safe. The `dp::event_bus` class usese a Meyer's singlton which make the instantiation of the event bus thread safe, but other functionality is not yet thread safe. 

## Contributing

If you find an issue with this library please file an [issue](https://github.com/DeveloperPaul123/eventbus/issues). Pull requests are also welcome! Contributing guide soon to come. 
