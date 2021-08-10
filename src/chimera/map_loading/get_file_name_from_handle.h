// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA__GET_FILE_NAME_FROM_HANDLE_H
#define CHIMERA__GET_FILE_NAME_FROM_HANDLE_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

DWORD WINAPI GetFileNameFromHandle(HANDLE hFile, LPSTR lpszFilePath, DWORD cchFilePath, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif
