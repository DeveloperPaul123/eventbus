#pragma once

#include <memory>

template< typename T>
class singleton
{
 public:
   using singletonType = singleton<T>;

 protected:
	explicit singleton()
	{
		static_assert(std::is_base_of<singletonType, T>::value, "Invalid inheritance relationship detected");
	}

   singleton( const singleton& ) = default;
   singleton& operator=( const singleton& ) = default;
   ~singleton() = default;

 public:
   static std::shared_ptr<T> instance()
   {
      static std::shared_ptr<T> object( new T() );
      return object;
   }
};
