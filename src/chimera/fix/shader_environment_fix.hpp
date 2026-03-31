// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_SHADER_ENVIRONMENT_FIX
#define CHIMERA_SHADER_ENVIRONMENT_FIX

namespace Chimera {
    /**
     * Fixes and memes for shader_environment tags.
     * - Fix environment_specular_light texture/sampler mismatch.
     * - Fix environment_specular_lightmap not working properly/at all.
     * - Support alternate bump attenuation flag.
     * - Dank memes for legacy maps.
     */
    void set_up_shader_environment_fix() noexcept;
}

#endif
