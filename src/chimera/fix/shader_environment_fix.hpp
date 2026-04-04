// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_SHADER_ENVIRONMENT_FIX
#define CHIMERA_SHADER_ENVIRONMENT_FIX

namespace Chimera {
    /**
     * Fixes and memes for shader_environment tags.
     * - Fix environment_specular_light texture/sampler mismatch.
     * - Fix environment_specular_lightmap not working properly/at all.
     * - Fix environment_reflection_mirror not setting vector normalization (allows us to use the higher quality ps2.0 shader).
     * - Fix environment_reflection_flat using the ps1.1 shader on retail instead of the ps2.0 version.
     * - Support alternate bump attenuation flag.
     * - Dank memes for legacy maps.
     */
    void set_up_shader_environment_fix() noexcept;
}

#endif
