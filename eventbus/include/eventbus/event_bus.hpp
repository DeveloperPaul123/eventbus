#pragma once

#include "eventbus/utils/singleton.hpp"
#include "eventbus/utils/callable.hpp"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>

class event_bus : public singleton<event_bus>
{
public:
    using singleton<event_bus>::instance;

    template<typename EventType, typename EventHandler>
    static void register_handler(const EventHandler &handler)
    {
        auto instance = event_bus::instance();
        instance->handler_registrations[typeid(EventType)].emplace_back(handler);
    }

    template<typename EventType>
    static void fire_event(const EventType& evt) 
    {
        const auto instance = event_bus::instance();
		auto& func_map = instance->handler_registrations;
		if (func_map.find(typeid(evt)) != func_map.end())
		{
			auto& callbacks = func_map[typeid(evt)];
			for (callable<void>& callback : callbacks)
			{
				callback(evt);
			}
		}
		else
		{
#if(_DEBUG)
			// print out error in debug
			std::cerr << "No registered callbacks for given event type.\n";
#endif
		}
    }
private:
    event_bus() = default;
	template< typename> friend class ::singleton;
	std::unordered_map<std::type_index, std::vector<callable<void>>> handler_registrations;
};
