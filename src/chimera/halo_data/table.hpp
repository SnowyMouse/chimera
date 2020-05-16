// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_TABLE_HPP
#define CHIMERA_TABLE_HPP

#include <cstdint>
#include "pad.hpp"

namespace Chimera {
    /**
     * This is a table type that is used throughout Halo for many things.
     */
    template<typename T> struct GenericTable {
        /** Name of the table */
        char name[0x20];

        /** Maximum number of elements */
        std::uint16_t max_elements;

        /** Size of an element */
        std::uint16_t element_size;

        PAD(0x8);
        PAD(0x2);

        /** Current size of the table, even including gaps */
        std::uint16_t current_size;

        /** Number of valid elements in the table */
        std::uint16_t count;

        /** ID of the next element to be added */
        std::uint16_t next_id;

        /** Pointer to the first element */
        T *first_element;

        /**
         * Get the element from an index
         * @param  index index of the element
         * @return       pointer to the element or nullptr if out of bounds
         */
        T *get_element(std::size_t index) {
            if(index >= this->current_size) {
                return nullptr;
            }
            else {
                return this->first_element + index;
            }
        }
    };
    static_assert(sizeof(GenericTable<int>) == 0x38);
}

#endif
