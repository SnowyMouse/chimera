/**
 * Monolith Mod Loader
 *
 * Copyright (c) 2019 Kavawuvi
 *
 * This software is licensed under the GNU General Public License version 3 or
 * later. For more information, see LICENSE.
 */

#define MODS_PATH "mods\\"

#include <w32api.h>
#define _WIN32_WINNT Windows7

#include <windows.h>
#include <winbase.h>
#include <shlwapi.h>

typedef struct {
    HMODULE module;
} LoadedDLL;

static LoadedDLL *loaded_dlls;
static int dll_count = 0;
static int dll_capacity = 16;

static int string_copy(char *out, const char *in) {
    int i=0;
    while(*in) {
        *(out++) = *(in++);
        i++;
    }
    *out = 0;
    return i;
}

static inline char to_lowercase(char a) {
    return a + (a >= 'A' && a <= 'Z') * ('a' - 'A');
}

static inline int string_equal(const char *a, const char *b) {
    while(to_lowercase(*(a++)) == to_lowercase(*(b++)) && *a);
    return *a == *b;
}

// Unload all DLLs
static void unload_dlls() {
    if(!loaded_dlls) {
        return;
    }

    for(int i = 0; i < dll_count; i++) {
        FreeLibrary(loaded_dlls[i].module);
    }

    VirtualFree(loaded_dlls, sizeof(*loaded_dlls) * dll_capacity, MEM_DECOMMIT | MEM_RELEASE);
    loaded_dlls = NULL;
}

// Load DLLs; return true if Chimera could be loaded
static BOOL load_dlls() {
    dll_count = 0;
    loaded_dlls = VirtualAlloc(NULL, sizeof(*loaded_dlls) * dll_capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    // Load Chimera
    extern void instantiate_chimera();
    extern void destroy_chimera();
    extern int find_signatures();
    extern const char *signature_errors();
    extern int halo_type();
    instantiate_chimera();
    if(find_signatures() == 0) {
        MessageBox(NULL, signature_errors(), "Error", MB_OK);
        destroy_chimera();
        return FALSE;
    }

    // Now try to load everything in mods
    WIN32_FIND_DATA find_file_data;
    HMODULE handle = FindFirstFile(MODS_PATH "*.dll", &find_file_data);
    BOOL ok = handle != INVALID_HANDLE_VALUE;
    while(ok) {
        // Find it!
        char path[MAX_PATH * 2];
        int end = string_copy(path, MODS_PATH);
        end += string_copy(path + end, find_file_data.cFileName);
        ok = FindNextFile(handle, &find_file_data);

        // Wine workaround; As of 4.3-staging, *.dll matches anything with an extension that starts with .dll apparently. Also prevent chimera.dll from being loaded here.
        if(!string_equal(path + end - 4, ".dll") || string_equal(path, MODS_PATH "chimera.dll")) {
            continue;
        }

        // Attempt to load it
        loaded_dlls[dll_count].module = LoadLibrary(path);
        if(loaded_dlls[dll_count].module == NULL) {
            continue;
        }

        // Reallocate if needed
        if(++dll_count == dll_capacity) {
            int new_dll_capacity = dll_capacity * 2;
            LoadedDLL *new_loaded_dlls = VirtualAlloc(NULL, sizeof(*loaded_dlls) * new_dll_capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            for(int i = 0; i < dll_count; i++) {
                new_loaded_dlls[i].module = loaded_dlls[i].module;
            }
            VirtualFree(loaded_dlls, sizeof(*loaded_dlls) * dll_capacity, MEM_DECOMMIT | MEM_RELEASE);
            loaded_dlls = new_loaded_dlls;
            dll_capacity = new_dll_capacity;
        }
    }

    return TRUE;
}

// DLL entry point
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    switch(reason) {
        case DLL_PROCESS_ATTACH:
            {
                // Enable DEP
                SetProcessDEPPolicy(PROCESS_DEP_ENABLE);

                // Terminate if we have to
                if(!load_dlls()) {
                    ExitProcess(133);
                }

                // Check if keystone is loaded. If so, bail.
                if(GetModuleHandle("keystone.dll")) {
                    ExitProcess(197);
                }
            }
            break;
        case DLL_PROCESS_DETACH:
            unload_dlls();
            break;
    }
    return TRUE;
}
