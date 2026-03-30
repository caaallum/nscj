#include <Windows.h>

#include "util.h"

//==========================================================
VOID
tchar_to_utf8(_In_ const TCHAR* src, _Out_ LPSTR dst, _In_ INT max) {
#ifdef UNICODE
    WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, max, NULL, NULL);
#else
    (void)lstrcpyn(dst, src, max);
#endif
}

//==========================================================
VOID
utf8_to_tchar(_In_ LPCSTR src, _Out_ TCHAR* dst, _In_ INT max) {
#ifdef UNICODE
    MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, max);
#else
    (void)lstrcpyn(dst, src, max);
#endif
}

//==========================================================
VOID
tchar_to_wchar(_In_ LPCTSTR src, _Out_ LPWSTR dst, _In_ INT max) {
#ifdef UNICODE
    (void)lstrcpyn(dst, src, max);
#else
    MultiByteToWideChar(CP_ACP, 0, src, -1, dst, max);
#endif
}

//==========================================================
BOOL
is_number(_In_ LPCTSTR str) {
    if (!str || !*str) return 0;

    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return FALSE;
        }
    }
    return TRUE;
}

//==========================================================
BOOL 
read_file(_In_ LPCTSTR filename, _Out_ PTCHAR* buffer, _Out_ LPDWORD outlength) {
    HANDLE hFile = CreateFile(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return FALSE;
    }

    if (fileSize.QuadPart > (MAXDWORD - 1)) {
        CloseHandle(hFile);
        return FALSE;
    }

    DWORD bytesToRead = (DWORD)fileSize.QuadPart;

    // Read raw bytes into a narrow buffer first
    LPSTR rawBuf = (LPSTR)GlobalAlloc(GPTR, bytesToRead + 1);
    if (!rawBuf) {
        CloseHandle(hFile);
        return FALSE;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(hFile, rawBuf, bytesToRead, &bytesRead, NULL) || bytesRead != bytesToRead) {
        GlobalFree(rawBuf);
        CloseHandle(hFile);
        return FALSE;
    }
    CloseHandle(hFile);
    rawBuf[bytesRead] = '\0';

#ifdef UNICODE
    // Query the required wide-char buffer size
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, rawBuf, (int)bytesRead, NULL, 0);
    if (wideLen == 0) {
        GlobalFree(rawBuf);
        return FALSE;
    }

    LPWSTR wideBuf = (LPWSTR)GlobalAlloc(GPTR, ((SIZE_T)wideLen + 1) * sizeof(WCHAR));
    if (!wideBuf) {
        GlobalFree(rawBuf);
        return FALSE;
    }

    MultiByteToWideChar(CP_UTF8, 0, rawBuf, (int)bytesRead, wideBuf, wideLen);
    wideBuf[wideLen] = L'\0';

    GlobalFree(rawBuf);

    *buffer = wideBuf;
    *outlength = (DWORD)(wideLen * sizeof(WCHAR));
#else
    // ANSI build — raw bytes are already the right type
    * buffer = rawBuf;
    *outlength = bytesRead;
#endif

    return TRUE;
}