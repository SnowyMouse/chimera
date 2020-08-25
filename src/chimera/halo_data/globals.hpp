// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_GLOBALS_HPP
#define CHIMERA_GLOBALS_HPP

namespace Chimera {
    enum ScriptingGlobalType {
        SCRIPTING_GLOBAL_NOT_FOUND = 0,
        SCRIPTING_GLOBAL_BOOLEAN,
        SCRIPTING_GLOBAL_REAL,
        SCRIPTING_GLOBAL_SHORT,
        SCRIPTING_GLOBAL_LONG,
        SCRIPTING_GLOBAL_UNIMPLEMENTED
    };

    union ScriptingGlobalValue {
        bool boolean;
        float real;
        short short_int;
        long long_int;
    };

    struct ScriptingGlobal {
        ScriptingGlobalType type = SCRIPTING_GLOBAL_NOT_FOUND;
        ScriptingGlobalValue value;
    };

    ScriptingGlobal read_global(const char *global_name) noexcept;

    bool set_global(const char *global_name, ScriptingGlobalValue value) noexcept;
}

#endif
