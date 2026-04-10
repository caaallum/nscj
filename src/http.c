#include <Windows.h>
#include <winhttp.h>
#include <assert.h>

#include "http.h"

//==========================================================
BOOL
http_fetch(_In_ http_config_t* config, _Out_ TCHAR** response) {
    for (int redirect = 0; redirect < 5; redirect++) {
        URL_COMPONENTS parts = { 0 };
        parts.dwStructSize = sizeof(parts);

        TCHAR host[512];
        TCHAR path[2048];

        parts.lpszHostName = host;
        parts.dwHostNameLength = 512;
        parts.lpszUrlPath = path;
        parts.dwUrlPathLength = 2048;

        if (!WinHttpCrackUrl(config->url, 0, 0, &parts))
            return FALSE;

        BOOL isHttps = (parts.nScheme == INTERNET_SCHEME_HTTPS);

        HINTERNET hSession = WinHttpOpen(L"NSCJ/1.0",
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
            config->method,
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

        if (config->username && config->password) {
            WinHttpSetCredentials(
                hRequest,
                WINHTTP_AUTH_TARGET_SERVER,
                WINHTTP_AUTH_SCHEME_BASIC,
                config->username,
                config->password,
                NULL);
        }

        DWORD body_len = config->body ? lstrlenA(config->body) : 0;

        BOOL sent = WinHttpSendRequest(
            hRequest,
            config->headers ? config->headers : WINHTTP_NO_ADDITIONAL_HEADERS,
            -1,
            config->body ? config->body : WINHTTP_NO_REQUEST_DATA,
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
            WCHAR location[2048];
            DWORD len = sizeof(location);

            if (WinHttpQueryHeaders(
                hRequest,
                WINHTTP_QUERY_LOCATION,
                NULL,
                location,
                &len,
                NULL)) {
                http_config_set_url(config, location);

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
        size_t wlen = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
        *response = (TCHAR*)malloc(wlen * sizeof(TCHAR));
        MultiByteToWideChar(CP_UTF8, 0, buffer, -1, *response, (int)wlen);
        GlobalFree(buffer);
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


//==========================================================
http_config_t*
http_config_new() {
    http_config_t* config = (http_config_t*)malloc(sizeof(http_config_t));
    assert(config);

    config->url = NULL;
    config->method = NULL;
    config->headers = NULL;
    config->body = NULL;
    config->username = NULL;
    config->password = NULL;

    http_config_set_method(config, TEXT("GET"));

    return config;
}

//==========================================================
VOID
http_config_set_url(_In_ http_config_t* this, _In_ LPCTSTR url) {
    assert(this);
    assert(url);

    if (this->url) free(this->url);
#ifdef UNICODE
    int len = lstrlenW(url) + 1;
    this->url = (PWCHAR)malloc(sizeof(WCHAR) * len);
    assert(this->url);
    lstrcpyW(this->url, url);
#else
    int len = MultiByteToWideChar(CP_UTF8, 0, url, -1, NULL, 0);
    this->url = (PWCHAR)malloc(sizeof(WCHAR) * len);  // sizeof(WCHAR) * len, not just len
    assert(this->url);
    MultiByteToWideChar(CP_UTF8, 0, url, -1, this->url, len);
#endif
}

//==========================================================
VOID
http_config_set_method(_In_ http_config_t* this, _In_ LPCTSTR method) {
    assert(this);
    assert(method);

    if (this->method) free(this->method);
#ifdef UNICODE
    int len = lstrlenW(method) + 1;
    this->method = (PWCHAR)malloc(sizeof(WCHAR) * len);
    assert(this->method);
    lstrcpyW(this->method, method);
#else
    int len = MultiByteToWideChar(CP_UTF8, 0, method, -1, NULL, 0);
    this->method = (PWCHAR)malloc(sizeof(WCHAR) * len);  // sizeof(WCHAR) * len, not just len
    assert(this->method);
    MultiByteToWideChar(CP_UTF8, 0, method, -1, this->method, len);
#endif
}

//==========================================================
VOID
http_config_set_headers(_In_ http_config_t* this, _In_ LPCTSTR headers) {
    assert(this);
    assert(headers);

    if (this->headers) free(this->headers);
#ifdef UNICODE
    int len = lstrlenW(headers) + 1;
    this->headers = (PWCHAR)malloc(sizeof(WCHAR) * (len + 3));
    assert(this->headers);
    lstrcpyW(this->headers, headers);
#else
    int len = MultiByteToWideChar(CP_UTF8, 0, headers, -1, NULL, 0);
    this->headers = (PWCHAR)malloc(sizeof(WCHAR) * (len + 3));  // sizeof(WCHAR) * len, not just len
    assert(this->headers);
    MultiByteToWideChar(CP_UTF8, 0, headers, -1, this->headers, len);
#endif

    lstrcatW(this->headers, L"\r\n");
}

//==========================================================
VOID
http_config_add_header(_In_ http_config_t* this, _In_ LPCTSTR header) {
    assert(this);
    assert(header);

#ifdef UNICODE
    int header_len = lstrlenW(header);
    int existing_len = this->headers ? lstrlenW(this->headers) : 0;
    // +3 for \r\n and null terminator
    int new_len = existing_len + header_len + 3;

    PWCHAR new_headers = (PWCHAR)malloc(sizeof(WCHAR) * new_len);
    assert(new_headers);

    if (this->headers) {
        lstrcpyW(new_headers, this->headers);
        free(this->headers);
    }
    else {
        new_headers[0] = L'\0';
    }

    lstrcatW(new_headers, header);
    lstrcatW(new_headers, L"\r\n");
    this->headers = new_headers;
#else
    int header_len = MultiByteToWideChar(CP_UTF8, 0, header, -1, NULL, 0) - 1; // exclude null
    int existing_len = this->headers ? lstrlenW(this->headers) : 0;
    int new_len = existing_len + header_len + 3;

    PWCHAR new_headers = (PWCHAR)malloc(sizeof(WCHAR) * new_len);
    assert(new_headers);

    if (this->headers) {
        lstrcpyW(new_headers, this->headers);
        free(this->headers);
    }
    else {
        new_headers[0] = L'\0';
    }

    // Convert and append at the right offset
    MultiByteToWideChar(CP_UTF8, 0, header, -1, new_headers + existing_len, header_len + 1);
    lstrcatW(new_headers, L"\r\n");
    this->headers = new_headers;
#endif
}

//==========================================================
VOID
http_config_set_body(_In_ http_config_t* this, _In_ LPCTSTR body) {
    assert(this);
    assert(body);

    if (this->body) free(this->body);
#ifdef UNICODE
    int len = WideCharToMultiByte(CP_UTF8, 0, body, -1, NULL, 0, NULL, NULL);
    this->body = (PCHAR)malloc(len);
    assert(this->body);
    WideCharToMultiByte(CP_UTF8, 0, body, -1, this->body, len, NULL, NULL);
#else
    int len = lstrlenA(body) + 1;  // +1 for null terminator
    this->body = (PCHAR)malloc(len);
    assert(this->body);
    lstrcpyA(this->body, body);
#endif
}

//==========================================================
VOID
http_config_set_username(_In_ http_config_t* this, _In_ LPCTSTR username) {
    assert(this);
    assert(username);

    if (this->username) free(this->username);
#ifdef UNICODE
    int len = lstrlenW(username) + 1;
    this->username = (PWCHAR)malloc(sizeof(WCHAR) * len);
    assert(this->username);
    lstrcpyW(this->username, username);
#else
    int len = MultiByteToWideChar(CP_UTF8, 0, username, -1, NULL, 0);
    this->username = (PWCHAR)malloc(sizeof(WCHAR) * len);  // sizeof(WCHAR) * len, not just len
    assert(this->username);
    MultiByteToWideChar(CP_UTF8, 0, username, -1, this->username, len);
#endif
}

//==========================================================
VOID
http_config_set_password(_In_ http_config_t* this, _In_ LPCTSTR password) {
    assert(this);
    assert(password);

    if (this->password) free(this->password);
#ifdef UNICODE
    int len = lstrlenW(password) + 1;
    this->password = (PWCHAR)malloc(sizeof(WCHAR) * len);
    assert(this->password);
    lstrcpyW(this->password, password);
#else
    int len = MultiByteToWideChar(CP_UTF8, 0, password, -1, NULL, 0);
    this->password = (PWCHAR)malloc(sizeof(WCHAR) * len);  // sizeof(WCHAR) * len, not just len
    assert(this->password);
    MultiByteToWideChar(CP_UTF8, 0, password, -1, this->password, len);
#endif
}

//==========================================================
VOID
http_config_free(_In_ http_config_t* config) {
    if (!config)
        return;
    if (config->url) free(config->url);
    if (config->method) free(config->method);
    if (config->headers) free(config->headers);
    if (config->body) free(config->body);
    if (config->username) free(config->username);
    if (config->password) free(config->password);
    free(config);
}