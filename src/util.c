#include <Windows.h>

#include "util.h"

//==========================================================
void
tchar_to_utf8(const TCHAR* src, char* dst, int max) {
#ifdef UNICODE
    WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, max, NULL, NULL);
#else
    (void)lstrcpyn(dst, src, max);
#endif
}

//==========================================================
void
utf8_to_tchar(const char* src, TCHAR* dst, int max) {
#ifdef UNICODE
    MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, max);
#else
    (void)lstrcpyn(dst, src, max);
#endif
}

//==========================================================
void
tchar_to_wchar(const TCHAR* src, wchar_t* dst, int max) {
#ifdef UNICODE
    (void)lstrcpyn(dst, src, max);
#else
    MultiByteToWideChar(CP_ACP, 0, src, -1, dst, max);
#endif
}

//==========================================================
BOOL
is_number(const TCHAR* str) {
    if (!str || !*str) return 0;

    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return FALSE;
        }
    }
    return TRUE;
}