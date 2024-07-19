#pragma once

#include <cassert>
#include <cstdint>

#include <concepts>
#include <utility>

////////////////////////////////////////////////////////////////////////////////

template <class Derived>
class RefCountedBase
{
    // ???
};

////////////////////////////////////////////////////////////////////////////////

template <class T>
class IntrusivePtr
{
    // ???
};

template <class T, class... Args>
IntrusivePtr<T> New(Args&&... args)
{
    return {}; // ???
}
