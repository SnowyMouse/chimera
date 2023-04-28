#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h>

#include "get_file_name_from_handle.h"

// Stolen from https://docs.microsoft.com/en-us/windows/win32/memory/obtaining-a-file-name-from-a-file-handle?redirectedfrom=MSDN
#define BUFSIZE 512

DWORD WINAPI GetFileNameFromHandle(HANDLE hFile, LPSTR lpszFilePath, DWORD cchFilePath, DWORD dwFlags)
{
  BOOL bSuccess = FALSE;
  TCHAR pszFilename[MAX_PATH+1];
  HANDLE hFileMap;

  // Get the file size.
  DWORD dwFileSizeHi = 0;
  DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

  if( dwFileSizeLo == 0 && dwFileSizeHi == 0 )
  {
     _tprintf(TEXT("Cannot map a file with a length of zero.\n"));
     return FALSE;
  }

  // Create a file mapping object.
  hFileMap = CreateFileMapping(hFile,
                    NULL,
                    PAGE_READONLY,
                    0,
                    1,
                    NULL);

  if (hFileMap)
  {
    // Create a file mapping to get the file name.
    void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

    if (pMem)
    {
      if (GetMappedFileName (GetCurrentProcess(),
                             pMem,
                             pszFilename,
                             MAX_PATH))
      {

        // Translate path with device name to drive letters.
        TCHAR szTemp[BUFSIZE];
        szTemp[0] = '\0';

        if (GetLogicalDriveStrings(BUFSIZE-1, szTemp))
        {
          TCHAR szName[MAX_PATH];
          TCHAR szDrive[3] = TEXT(" :");
          BOOL bFound = FALSE;
          TCHAR* p = szTemp;

          do
          {
            // Copy the drive letter to the template string
            *szDrive = *p;

            // Look up each device name
            if (QueryDosDevice(szDrive, szName, MAX_PATH))
            {
              size_t uNameLen = _tcslen(szName);

              if (uNameLen < MAX_PATH)
              {
                bFound = _tcsnicmp(pszFilename, szName, uNameLen) == 0
                         && *(pszFilename + uNameLen) == _T('\\');

                if (bFound)
                {
                  // Reconstruct pszFilename using szTempFile
                  // Replace device path with DOS path
                  TCHAR szTempFile[MAX_PATH];
                  StringCchPrintf(szTempFile,
                            MAX_PATH,
                            TEXT("%s%s"),
                            szDrive,
                            pszFilename+uNameLen);
                  StringCchCopyN(pszFilename, MAX_PATH+1, szTempFile, _tcslen(szTempFile));
                }
              }
            }

            // Go to the next NULL character.
            while (*p++);
          } while (!bFound && *p); // end of string
        }
      }
      bSuccess = TRUE;
      UnmapViewOfFile(pMem);
    }

    CloseHandle(hFileMap);
  }
 //_tprintf(TEXT("File name is %s\n"), pszFilename);

  if(bSuccess) {
    LPSTR copy_start = pszFilename;
    if(dwFlags & VOLUME_NAME_NONE) {
      copy_start += 6;
    }
    StringCchCopyN(lpszFilePath, cchFilePath, copy_start, _tcslen(copy_start));
  }
  return(bSuccess);
}
