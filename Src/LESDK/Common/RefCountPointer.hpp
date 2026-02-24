/**
 * @file        LESDK/Common/TLinkedList.hpp
 * @brief       This file contains the template for a basic Unreal RefCountPointer struct.
 *
 * @author      Mgamerz
 */

#pragma once

// ===================
// Ref Count Pointer
// ===================
// Wrapper for types that support Ref counting
// In compiled code it is simply a pointer to the object.
template<typename T>
struct RefCountPointer {
    T* Object; // Pointer to the object that is referenced

    RefCountPointer() {}

    RefCountPointer(T* InObject)
        : Object(InObject) {
    }

    RefCountPointer& operator=(T* other)
    {
        // There's some reference counting going on here...
        // Not sure we should mess with doing it
        // If stuff really doesn't work, we can do that I guess.
        Object = other;
        return *this;
    }

    RefCountPointer& operator=(const RefCountPointer& other)
    {
        return *this = other.Object;
    }
};
