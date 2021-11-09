#pragma once

#include "detail/function_traits.hpp"

#include <unordered_map>
#include <typeindex>
#include <functional>
#include <any>
#include <shared_mutex>
#include <atomic>
#include <thread>
#include <utility>
#include <mutex>

namespace dp
{
	class event_bus;

	class handler_registration
	{
		const void* handle_{ nullptr };
		dp::event_bus* event_bus_{ nullptr };
	public:
		handler_registration(const handler_registration& other) = delete;
		handler_registration(handler_registration&& other) noexcept;
		handler_registration& operator=(const handler_registration& other) = delete;
		handler_registration& operator=(handler_registration&& other) noexcept;
		~handler_registration();

		[[nodiscard]] const void* handle() const;
		void unregister() const noexcept;
	protected:
		handler_registration(const void* handle, dp::event_bus* bus);
		friend class event_bus;
	};

	class event_bus
	{
	public:

		event_bus() = default;
		template<typename EventType, typename EventHandler>
		[[nodiscard]] handler_registration register_handler(EventHandler&& handler)
		{
			using traits = detail::function_traits<EventHandler>;
			const auto type_idx = std::type_index(typeid(EventType));
			const void* handle;
			if constexpr (traits::arity == 0)
			{
				safe_unique_registrations_access([&]() {
					auto it = handler_registrations_.emplace(type_idx, [handler = std::forward<EventHandler>(handler)](auto) {
						handler();
					});

					handle = static_cast<const void*>(&(it->second));
					});
			}
			else
			{
				safe_unique_registrations_access([&]() {
					auto it = handler_registrations_.emplace(type_idx, [func = std::forward<EventHandler>(handler)](auto value) {
						func(std::any_cast<EventType>(value));
					});

					handle = static_cast<const void*>(&(it->second));
					});
			}
			return { handle, this };
		}

		template<typename EventType, typename ClassType, typename MemberFunction>
		[[nodiscard]] handler_registration register_handler(ClassType* class_instance, MemberFunction&& function) noexcept
		{
			using traits = detail::function_traits<MemberFunction>;
			static_assert(std::is_same_v<ClassType, std::decay_t<typename traits::owner_type>>, "Member function pointer must match instance type.");

			const auto type_idx = std::type_index(typeid(EventType));
			const void* handle;

			if constexpr (traits::arity == 0)
			{
				safe_unique_registrations_access([&]() {
					auto it = handler_registrations_.emplace(type_idx, [class_instance, function](auto) {
						(class_instance->*function)();
						});

					handle = static_cast<const void*>(&(it->second));
					});
			}
			else
			{
				safe_unique_registrations_access([&]() {
					auto it = handler_registrations_.emplace(type_idx, [class_instance, function](auto value) {
						(class_instance->*function)(std::any_cast<EventType>(value));
						});

					handle = static_cast<const void*>(&(it->second));
					});
			}
			return { handle, this };
		}

		template<typename EventType, typename = std::enable_if_t<!std::is_pointer_v<EventType>>>
		void fire_event(EventType&& evt) noexcept
		{
			safe_shared_registrations_access([this, local_event = std::forward<EventType>(evt)]() {
				// only call the functions we need to
				auto [begin_evt_id, end_evt_id] = handler_registrations_.equal_range(std::type_index(typeid(EventType)));
				for (; begin_evt_id != end_evt_id; ++begin_evt_id)
				{
					begin_evt_id->second(local_event);
				}
				});
		}

		bool remove_handler(const handler_registration& registration) noexcept
		{
			if (!registration.handle()) { return false; }

			auto result = false;
			safe_unique_registrations_access([this, &result, &registration]()
				{
					for (auto it = handler_registrations_.begin(); it != handler_registrations_.end(); ++it)
					{
						if (static_cast<const void*>(&(it->second)) == registration.handle())
						{
							handler_registrations_.erase(it);
							result = true;
							break;
						}
					}
				});
			return result;
		}

		void remove_handlers() noexcept
		{
			safe_unique_registrations_access([this]()
				{
					handler_registrations_.clear();
				});
		}

		[[nodiscard]] std::size_t handler_count() noexcept
		{
			std::shared_lock<mutex_type> lock(registration_mutex_);
			std::size_t count{};
			safe_shared_registrations_access([this, &count]()
				{
					count = handler_registrations_.size();
				});
			return count;
		}

	private:
		class mutex : public std::mutex
		{
		public:
			void lock()
			{
				std::mutex::lock();
				lock_holder_ = std::this_thread::get_id();
			}

			void unlock()
			{
				lock_holder_ = std::thread::id();
				std::mutex::unlock();
			}

			[[nodiscard]] bool locked_by_caller() const
			{
				return lock_holder_ == std::this_thread::get_id();
			}

		private:
			std::atomic<std::thread::id> lock_holder_{};
		};

		using mutex_type = std::shared_mutex;
		mutable mutex_type registration_mutex_;
		std::unordered_multimap<std::type_index, std::function<void(std::any)>> handler_registrations_;

		template<typename Callable>
		void safe_shared_registrations_access(Callable&& callable)
		{
			try
			{
				std::shared_lock<mutex_type> lock(registration_mutex_);
				callable();
			}
			catch (std::system_error&)
			{

			}
		}
		template<typename Callable>
		void safe_unique_registrations_access(Callable&& callable)
		{
			try
			{
				// if(registration_mutex_.locked_by_caller()) return;
				// if this fails, an exception may be thrown.
				std::unique_lock<mutex_type> lock(registration_mutex_);
				callable();
			}
			catch (std::system_error&)
			{
				// do nothing
			}
		}
	};


	inline const void* handler_registration::handle() const
	{
		return handle_;
	}

	inline void handler_registration::unregister() const noexcept
	{
		if (event_bus_ && handle_)
		{
			event_bus_->remove_handler(*this);
		}
	}

	inline handler_registration::handler_registration(const void* handle, dp::event_bus* bus)
		: handle_(handle), event_bus_(bus)
	{
	}

	inline handler_registration::handler_registration(handler_registration&& other) noexcept
		: handle_(std::exchange(other.handle_, nullptr)), event_bus_(std::exchange(other.event_bus_, nullptr))
	{
	}

	inline handler_registration& handler_registration::operator=(handler_registration&& other) noexcept
	{
		handle_ = std::exchange(other.handle_, nullptr);
		event_bus_ = std::exchange(other.event_bus_, nullptr);
		return *this;
	}

	inline handler_registration::~handler_registration()
	{
		if (event_bus_ && handle_)
		{
			event_bus_->remove_handler(*this);
		}
	}
}
