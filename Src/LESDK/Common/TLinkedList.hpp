/**
 * @file        LESDK/Common/TLinkedList.hpp
 * @brief       This file contains the template for a basic Unreal TLinkedList
 *
 * @author      Mgamerz
 */

#pragma once

// ==============
// TLinkedList
// ==============
template< class T > struct TLinkedList
{
public:
    // Crude way of counting the number of remaining forward items in the linked list.
    // You should call this at the start of the list, not the end.
    int Count() {
        int count = 1;
        TLinkedList* _nextItem = NextItem;
        while (_nextItem != nullptr) {
            count++;
            _nextItem = _nextItem->NextItem;
        }
        return count;
    }

    T CurrentItem;
    TLinkedList* NextItem;
    TLinkedList** PreviousItem;
};
