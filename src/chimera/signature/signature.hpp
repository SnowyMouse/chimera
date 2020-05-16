// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_SIGNATURE_HPP
#define CHIMERA_SIGNATURE_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <cstdint>

namespace Chimera {
    using SigByte = std::int16_t;

    class Signature {
    public:
        /**
         * Get the name of the signature
         * @return name of the signature
         */
        const char *name() const noexcept;

        /**
         * Get the name of the feature for this signature
         * @return name of the feature for the signature
         */
        const char *feature() const noexcept;

        /**
         * Get the original data retrieved when the signature was found or nullptr
         * @return original data
         */
        const std::byte *original_data() const noexcept;

        /**
         * Get the size of the signature given
         * @return signature size
         */
        std::size_t original_data_size() const noexcept;

        /**
         * Get the pointer to the data if the signature was found
         * @return pointer to the signature data or nullptr if not found
         */
        std::byte *data() const noexcept;

        /**
         * Revert data at data() back to data at original_data()
         */
        void rollback() const noexcept;

        /**
         * Constructor for a Signature
         * @param name      name of the signature
         * @param feature   feature of the signature
         * @param signature byte signature
         * @param length    length of the byte signature
         */
        Signature(const char *name, const char *feature, const SigByte *signature, std::size_t length);
    private:
        /** Name of the signature */
        std::string p_name;

        /** Feature of the signature */
        std::string p_feature;

        /** Original bytes of the signature */
        std::vector<std::byte> p_original_data;

        /** Pointer to where the signature is */
        std::byte *p_data = nullptr;
    };

    /**
     * Find all signatures
     * @return vector of all signatures
     */
    std::vector<Signature> find_all_signatures();
}


#endif
