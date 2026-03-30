#include <Windows.h>
#include <winhttp.h>

#include "http.h"

//==========================================================
BOOL
http_fetch(_In_ TCHAR* url, _In_ TCHAR* method, _In_ TCHAR* headers, _In_ TCHAR* body, _In_ TCHAR* username, _In_ TCHAR* password, _Out_ TCHAR** response) {
    for (int redirect = 0; redirect < 5; redirect++) {
        URL_COMPONENTS parts = { 0 };
        parts.dwStructSize = sizeof(parts);

        TCHAR host[512];
        TCHAR path[2048];

        parts.lpszHostName = host;
        parts.dwHostNameLength = 512;
        parts.lpszUrlPath = path;
        parts.dwUrlPathLength = 2048;

        if (!WinHttpCrackUrl(url, 0, 0, &parts))
            return FALSE;

        BOOL isHttps = (parts.nScheme == INTERNET_SCHEME_HTTPS);

        HINTERNET hSession = WinHttpOpen(TEXT("NSCJ/1.0"),
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0);

        if (!hSession) return FALSE;

        HINTERNET hConnect = WinHttpConnect(hSession,
            parts.lpszHostName,
            parts.nPort, 0);

        if (!hConnect) {
            WinHttpCloseHandle(hSession);
            return FALSE;
        }

        HINTERNET hRequest = WinHttpOpenRequest(
            hConnect,
            method,
            parts.lpszUrlPath,
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            isHttps ? WINHTTP_FLAG_SECURE : 0);

        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return FALSE;
        }

        if (username && username[0]) {
            WinHttpSetCredentials(
                hRequest,
                WINHTTP_AUTH_TARGET_SERVER,
                WINHTTP_AUTH_SCHEME_BASIC,
                username,
                password,
                NULL);
        }

        char* body_utf8 = NULL;
        DWORD body_len = 0;

#ifdef UNICODE
        if (body && body[0]) {
            int len = WideCharToMultiByte(CP_UTF8, 0, body, -1, NULL, 0, NULL, NULL);
            body_utf8 = (char*)malloc(len);
            WideCharToMultiByte(CP_UTF8, 0, body, -1, body_utf8, len, NULL, NULL);
            body_len = len - 1; // exclude null terminator
        }
#else
        body_utf8 = body;
        body_len = body ? (DWORD)strlen(body) : 0;
#endif

        BOOL sent = WinHttpSendRequest(
            hRequest,
            headers && headers[0] ? headers : WINHTTP_NO_ADDITIONAL_HEADERS,
            -1,
            body_utf8 ? body_utf8 : WINHTTP_NO_REQUEST_DATA,
            body_len,
            body_len,
            0);

        if (!sent || !WinHttpReceiveResponse(hRequest, NULL)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return FALSE;
        }

        DWORD status = 0;
        DWORD size = sizeof(status);

        WinHttpQueryHeaders(
            hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            NULL,
            &status,
            &size,
            NULL);

        if (status == 301 || status == 302 || status == 307 || status == 308) {
            TCHAR location[2048];
            DWORD len = sizeof(location);

            if (WinHttpQueryHeaders(
                hRequest,
                WINHTTP_QUERY_LOCATION,
                NULL,
                location,
                &len,
                NULL)) {
                lstrcpy(url, location); // follow redirect

                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                continue;
            }
        }

        // Read response
        char* buffer = GlobalAlloc(GPTR, sizeof(char));
        DWORD capacity = 0;
        DWORD total = 0;

        do {
            DWORD avail = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &avail))
                break;

            if (avail == 0)
                break;

            if (total + avail + 1 > capacity) {
                DWORD newCap = (capacity == 0) ? 8192 : capacity;
                while (newCap < total + avail + 1) newCap *= 2;

                char* newBuf = (char*)GlobalReAlloc(buffer, newCap, GMEM_MOVEABLE | GMEM_ZEROINIT);
                if (!newBuf) {
                    if (buffer) GlobalFree(buffer);
                    return FALSE;
                }

                buffer = newBuf;
                capacity = newCap;
            }

            DWORD read = 0;
            if (!WinHttpReadData(hRequest, buffer + total, avail, &read))
                break;

            total += read;

        } while (1);

        if (!buffer)
            return FALSE;

        buffer[total] = 0;

#ifdef UNICODE
        // Convert char buffer -> TCHAR
        size_t wlen = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
        *response = (TCHAR*)malloc(wlen * sizeof(TCHAR));
        MultiByteToWideChar(CP_UTF8, 0, buffer, -1, *response, (int)wlen);
        GlobalFree(buffer);
        if (body_utf8) free(body_utf8);
#else
        *response = (TCHAR*)buffer;
#endif

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return TRUE;
    }

    return FALSE;
}