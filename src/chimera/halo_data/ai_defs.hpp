// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_AI_DEFS_HPP
#define CHIMERA_AI_DEFS_HPP

#include <cstdint>

namespace Chimera {

    struct AIUnitInformationLookData {
        std::int32_t unit_index;
    };
    static_assert(sizeof(AIUnitInformationLookData) == 0x4);

    struct AIObjectInformationLookData {
        std::int32_t object_index;
    };
    static_assert(sizeof(AIObjectInformationLookData) == 0x4);

    struct AIInformationLookData {
        union {
            AIUnitInformationLookData unit;
            AIObjectInformationLookData object;
        };
    };
    static_assert(sizeof(AIInformationLookData) == 0x4);

    struct AIAallegianceInformationData {
        std::int16_t team1_index;
        std::int16_t team2_index;
        bool broken;
    };
    static_assert(sizeof(AIAallegianceInformationData) == 0x6);

    struct AICombatStimulusInformationData {
        std::uint16_t pad;
    };
    static_assert(sizeof(AICombatStimulusInformationData) == 0x2);

    struct AITargetKnowledgeInformationData {
        std::int32_t prop_index;
    };
    static_assert(sizeof(AITargetKnowledgeInformationData) == 0x4);

    struct AIInformationData {
        union {
            AIAallegianceInformationData allegiance;
            AICombatStimulusInformationData combat_stimulus;
            AITargetKnowledgeInformationData target_knowledge;
        };
    };
    static_assert(sizeof(AIInformationData) == 0x8);

    struct AIInformationPacket {
        std::int32_t target_unit_index;
        std::int16_t communication_type;
        std::int16_t dialogue_type_index;
        std::int16_t damage_category;
        bool updated_dialogue_timers;
        std::int16_t look_priority;
        std::int16_t look_type;
        AIInformationLookData look_data;
        std::int16_t information_type;
        AIInformationData information_data;
    };
    static_assert(sizeof(AIInformationPacket) == 0x20);

}

#endif
