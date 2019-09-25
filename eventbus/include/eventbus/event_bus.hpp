#pragma once

#include "detail/function_traits.hpp"

#include <unordered_map>
#include <typeindex>
#include <functional>
#include <any>

namespace dp
{
	struct handler_registration
	{
		const void* handle{ nullptr };
	};
	
	class event_bus 
	{
	public:

		[[nodiscard]] static event_bus& instance()
		{
			static event_bus evt_bus;
			return evt_bus;
		}
		
	    template<typename EventType, typename EventHandler>
	    [[nodiscard]] static handler_registration register_handler(EventHandler &&handler)
	    {
	        auto& instance = event_bus::instance();
			using traits = detail::function_traits<EventHandler>;
			const auto type_idx = std::type_index(typeid(EventType));
			handler_registration registration;
			if constexpr (traits::arity == 0)
			{
				auto it = instance.handler_registrations.emplace(type_idx, [handler = std::forward<EventHandler>(handler)](auto)
				{
					handler();
				});

				registration.handle = static_cast<const void*>(&(it->second));
			}
			else
			{
				auto it = instance.handler_registrations.emplace(type_idx, [func = std::forward<EventHandler>(handler)](auto value)
				{
					func(std::any_cast<EventType>(value));
				});

				registration.handle = static_cast<const void*>(&(it->second));
			}
			return registration;
	    }

		template<typename EventType, typename ClassType, typename MemberFunction>
		[[nodiscard]] static handler_registration register_handler(ClassType* class_instance, MemberFunction&& function) noexcept
	    {
			using traits = detail::function_traits<MemberFunction>;
			static_assert(std::is_same_v<ClassType, std::decay_t<typename traits::owner_type>>, "Member function pointer must match instance type.");
	    	
			auto& instance = event_bus::instance();
			const auto type_idx = std::type_index(typeid(EventType));
			handler_registration registration;

    		if constexpr (traits::arity == 0)
			{
				auto it = instance.handler_registrations.emplace(type_idx, [class_instance, function](auto)
					{
						(class_instance->*function)();
					});

				registration.handle = static_cast<const void*>(&(it->second));
			}
			else
			{
				auto it = instance.handler_registrations.emplace(type_idx, [class_instance, function](auto value)
					{
						(class_instance->*function)(std::any_cast<EventType>(value));
					});

				registration.handle = static_cast<const void*>(&(it->second));

			}
			return registration;
	    }

		template<typename EventType>
	    static void fire_event(const EventType& evt) noexcept
	    {
	        auto& instance = event_bus::instance();
			auto& func_map = instance.handler_registrations;
			// only call the functions we need to
			auto [begin_evt_id, end_evt_id] = func_map.equal_range(std::type_index(typeid(EventType)));
			for(; begin_evt_id != end_evt_id; ++begin_evt_id)
			{
				try
				{
					begin_evt_id->second(std::any_cast<EventType>(evt));
				}
				catch(std::bad_any_cast&){} // Ignore for now
			}
	    }

		static bool remove_handler(const handler_registration &registration) noexcept
	    {
			if (!registration.handle) { return false; }
			
			auto& callbacks = event_bus::instance().handler_registrations;
			for(auto it = callbacks.begin(); it != callbacks.end(); ++it)
			{
				if(static_cast<const void*>(&(it->second)) == registration.handle)
				{
					callbacks.erase(it);
					return true;
				}
			}

			return false;
	    }

		static void remove_handlers() noexcept
	    {
			event_bus::instance().handler_registrations.clear();
	    }

		[[nodiscard]] static std::size_t handler_count() noexcept
	    {
			return event_bus::instance().handler_registrations.size();
	    }
		
	private:
	    event_bus() = default;
		std::unordered_multimap<std::type_index, std::function<void(std::any)>> handler_registrations;
	};
}
