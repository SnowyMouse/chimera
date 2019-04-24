/**
 * Monolith Mod Loader
 *
 * Copyright (c) 2019 Kavawuvi
 *
 * This software is licensed under the GNU General Public License version 3 or
 * later. For more information, see LICENSE.
 */

#define MODS_PATH "mods\\"
#define CHIMERA_DLL "chimera.dll"

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

    // First load chimera.dll
    if(PathFileExists(CHIMERA_DLL)) {
        loaded_dlls[0].module = LoadLibrary(CHIMERA_DLL);
        dll_count++;

        typedef void (*void_fn)();
        typedef int (*int_fn)();
        typedef const char *(*char_arr_fn)();

        void_fn instantiate_chimera = (void_fn)GetProcAddress(loaded_dlls[0].module, "instantiate_chimera");
        void_fn destroy_chimera = (void_fn)GetProcAddress(loaded_dlls[0].module, "destroy_chimera");
        int_fn find_signatures = (int_fn)GetProcAddress(loaded_dlls[0].module, "find_signatures");
        char_arr_fn signature_errors = (char_arr_fn)GetProcAddress(loaded_dlls[0].module, "signature_errors");
        int_fn halo_type = (int_fn)GetProcAddress(loaded_dlls[0].module, "halo_type");

        // Make sure we have those functions
        if(!instantiate_chimera || !destroy_chimera || !find_signatures || !signature_errors || !halo_type) {
            MessageBox(NULL, "Unrecognizable " CHIMERA_DLL, "Error", MB_OK);
            return FALSE;
        }
        else {
            instantiate_chimera();
            if(find_signatures() == 0) {
                MessageBox(NULL, signature_errors(), "Error", MB_OK);
                destroy_chimera();
                return FALSE;
            }
        }
    }

    // Don't continue if we can't do anything
    else {
        unload_dlls();
        loaded_dlls = NULL;
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
        if(!string_equal(path + end - 4, ".dll") || string_equal(path, MODS_PATH CHIMERA_DLL)) {
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
BOOL WINAPI DllMainCRTStartup(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    switch(reason) {
        case DLL_PROCESS_ATTACH:
            {
                // Enable DEP
                SetProcessDEPPolicy(PROCESS_DEP_ENABLE);

                // Terminate if we have to
                if(!load_dlls()) {
                    char file_name[MAX_PATH];
                    DWORD len = GetModuleFileNameA(NULL, file_name, sizeof(file_name));
                    char haloce[] = "haloce.exe";
                    BOOL terminate = TRUE;
                    if(len >= sizeof(haloce)) {
                        if(string_equal(file_name + len - sizeof(haloce) + 1, haloce)) {
                            terminate = MessageBox(NULL, "Chimera could not be loaded. Continue running Halo?", "Error", MB_YESNO) == IDNO;
                        }
                    }

                    if(terminate) {
                        ExitProcess(133);
                    }
                }
            }
            break;
        case DLL_PROCESS_DETACH:
            unload_dlls();
            break;
    }
    return TRUE;
}
