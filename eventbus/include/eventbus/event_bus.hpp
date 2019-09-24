#pragma once

#include "detail/function_traits.hpp"

#include <unordered_map>
#include <typeindex>
#include <functional>
#include <any>

namespace dp
{
	class event_bus 
	{
	public:

		[[nodiscard]] static event_bus& instance()
		{
			static event_bus evt_bus;
			return evt_bus;
		}
		
	    template<typename EventType, typename EventHandler>
	    [[nodiscard]] static std::type_index register_handler(EventHandler &&handler)
	    {
	        auto& instance = event_bus::instance();
			using traits = detail::function_traits<EventHandler>;
			const auto type_idx = std::type_index(typeid(EventHandler));
			if constexpr (traits::arity == 0)
			{
				instance.handler_registrations.try_emplace(type_idx, [handler = std::forward<EventHandler>(handler)](auto)
				{
					handler();
				});
			}
			else
			{
				instance.handler_registrations.try_emplace(type_idx, [func = std::forward<EventHandler>(handler)](auto value)
				{
					func(std::any_cast<EventType>(value));
				});
			}
			return type_idx;
	    }

		template<typename T, typename ClassType, typename MemberFunction>
		[[nodiscard]] static std::type_index  register_handler(ClassType* class_instance, MemberFunction&& function) noexcept
	    {
			using traits = detail::function_traits<MemberFunction>;
			static_assert(std::is_same_v<ClassType, std::decay_t<typename traits::owner_type>>, "Member function pointer must match instance type.");
	    	
			auto& instance = event_bus::instance();
			const auto type_idx = std::type_index(typeid(function));
	    	
    		if constexpr (traits::arity == 0)
			{
				instance.handler_registrations.try_emplace(type_idx, [class_instance, function](auto)
					{
						(class_instance->*function)();
					});
			}
			else
			{
				instance.handler_registrations.try_emplace(type_idx, [class_instance, function](auto value)
					{
						(class_instance->*function)(std::any_cast<T>(value));
					});
			}
			return std::type_index(typeid(function));
	    }

		template<typename EventType>
	    static void fire_event(EventType&& evt) noexcept
	    {
	        auto& instance = event_bus::instance();
			auto& func_map = instance.handler_registrations;

			for(auto& callback : func_map)
			{
				try
				{
					callback.second(evt);
				}
				catch(std::bad_any_cast& 
#if(_DEBUG)				
					bad_any_cast // name the variable only in debug to avoid warnings in release
#endif
					)
				{
	#if(_DEBUG)
					std::cerr << "Bad any cast " << bad_any_cast.what() << "\n";
	#endif
				}
			}
	    }

		static bool remove_handler(const std::type_index index) noexcept
	    {
			auto& callbacks = event_bus::instance().handler_registrations;
		    if(callbacks.find(index) != std::end(callbacks))
		    {
				const auto num_erased = callbacks.erase(index);
				return num_erased > 0;
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
		std::unordered_map<std::type_index, std::function<void(std::any)>> handler_registrations;
	};
}
