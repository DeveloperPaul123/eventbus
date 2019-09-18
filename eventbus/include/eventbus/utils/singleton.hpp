#pragma once

#include <memory>

#define DETECT_CYCLIC_LIFETIME_DEPENDENCY(T) \
   static_assert( ( !HasCyclicDependency<T,NullType>::value ), "Cyclic dependency detected" )

#define CHECK_INHERITANCE_RELATIONSHIP(B,D) \
   static_assert( std::is_base_of<B,D>::value, "Invalid inheritance relationship detected" )


#define BEFRIEND_SINGLETON \
   template< typename> friend class ::singleton;

template< typename T>
class singleton
{
 public:
   using singletonType = singleton<T>;

 protected:
   explicit singleton()
   {
      CHECK_INHERITANCE_RELATIONSHIP( singletonType, T );
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
