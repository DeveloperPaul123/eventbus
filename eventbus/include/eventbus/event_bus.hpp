#pragma once

#include "utils/singleton.hpp"
#include "detail/function_traits.hpp"

#include <unordered_map>
#include <typeindex>
#include <functional>
#include <any>

namespace dp
{
	class event_bus : public singleton<event_bus>
	{
	public:
	    using singleton<event_bus>::instance;
		
	    template<typename EventType, typename EventHandler>
	    [[nodiscard]] static std::type_index register_handler(EventHandler &&handler)
	    {
	        auto instance = event_bus::instance();
			using traits = detail::function_traits<EventHandler>;
			const auto type_idx = std::type_index(typeid(EventHandler));
			if constexpr (traits::arity == 0)
			{
				instance->handler_registrations.try_emplace(type_idx, [handler = std::forward<EventHandler>(handler)](auto)
				{
					handler();
				});
			}
			else
			{
				instance->handler_registrations.try_emplace(type_idx, [func = std::forward<EventHandler>(handler)](auto value)
				{
					func(std::any_cast<EventType>(value));
				});
			}
			return type_idx;
	    }

		template<typename T, typename ClassType, typename MemberFunction>
		[[nodiscard]] static std::type_index  register_handler(ClassType* class_instance, MemberFunction&& function) noexcept
	    {
			using traits = detail::member_function_traits<MemberFunction>;
    		
			static_assert(std::is_same_v<ClassType, std::decay_t<typename traits::class_type>>, "Member function pointer must match instance type.");
			auto instance = event_bus::instance();
			const auto type_idx = std::type_index(typeid(function));
	    	
    		if constexpr (std::is_same_v<T, void>)
			{
				instance->handler_registrations.try_emplace(type_idx, [class_instance, function](auto)
					{
						(class_instance->*function)();
					});
			}
			else
			{
				instance->handler_registrations.try_emplace(type_idx, [class_instance, function](auto value)
					{
						(class_instance->*function)(std::any_cast<T>(value));
					});
			}
			return std::type_index(typeid(function));
	    }

		template<typename EventType>
	    static void fire_event(EventType&& evt) noexcept
	    {
	        const auto instance = event_bus::instance();
			auto& func_map = instance->handler_registrations;

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
			auto& callbacks = event_bus::instance()->handler_registrations;
		    if(callbacks.find(index) != std::end(callbacks))
		    {
				const auto num_erased = callbacks.erase(index);
				return num_erased > 0;
		    }

			return false;
	    }
		
	private:
	    event_bus() = default;
		template< typename> friend class dp::singleton;
		std::unordered_map<std::type_index, std::function<void(std::any)>> handler_registrations;
	};
}
