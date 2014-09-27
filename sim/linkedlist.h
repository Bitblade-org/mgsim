// -*- c++ -*-
#ifndef SIM_LINKEDLIST_H
#define SIM_LINKEDLIST_H

#include "sim/storage.h"

namespace Simulator
{
    // A linked list of indices to a table structure.
    template <
        typename          T, ///< The index type
        typename          L, ///< The lookup table type
        T L::value_type::*N  ///< The next field in the table's element type
        >
    class LinkedList : public SensitiveStorage
    {
        L&   m_table;     ///< The table to dereference to form the linked list
        bool m_empty;     ///< Whether this list is empty
        bool m_popped;    ///< Has a Pop() been done?
        bool m_pushed;    ///< Has a Push() been done?
        T    m_head;      ///< First item on the list (when !m_empty)
        T    m_tail;      ///< Last item on the list (when !m_empty)
        T    m_next;      ///< The next field to use (when m_popped)
        T    m_first;     ///< First item of the list being pushed (when m_pushed)
        T    m_last;      ///< Last item of the list being pushed (when m_pushed)

        void Update() override;

    public:
        /// Is the list empty?
        bool Empty() const;

        /// Does the list contain only one item?
        bool Singular() const;

        /// Returns the front index on the list
        const T& Front() const;

        /// Pushes the item on the back of the list
        void Push(const T& item);

        /// Appends the passed list to this list
        void Append(const T& first, const T& last);

        /// Removes the front item from this list
        void Pop();

        /// Construct an empty list with a sensitive component
        LinkedList(const std::string& name, Object& parent, Clock& clock, L& table);

        // A forward iterator (for debugging the contents)
        struct const_iterator
        {
            const L& m_table;
            const T& m_tail;
            T        m_index;
            bool     m_end;

        public:
            bool operator != (const const_iterator& rhs) const;
            bool operator == (const const_iterator& rhs) const;
            const_iterator& operator++();
            const_iterator operator++(int);
            const T& operator*() const;
            const_iterator(const L& table, const T& tail, const T& index);
            const_iterator(const L& table, const T& tail);
        };

        const_iterator begin() const { return const_iterator(m_table, m_tail, m_head);  }
        const_iterator end()   const { return const_iterator(m_table, m_tail); }
    };

}

#include "sim/linkedlist.hpp"

#endif
