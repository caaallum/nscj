#include <windows.h>
#include <stdio.h>
#include <nsis/pluginapi.h>
#include <cjson/cJSON.h>

#include "util.h"
#include "http.h"
#include "tree.h"

#define NSISFUNC(_name) void __declspec(dllexport) _name(HWND hWndParent, int string_size, TCHAR* variables, stack_t** stacktop, extra_parameters* extra)

HINSTANCE g_hInstance;
HWND g_hwndParent;

cJSON* cJSON_ParseT(const TCHAR* json) {
    cJSON* root = NULL;
#ifdef UNICODE

    if (!json) return root;

    // Convert wchar_t -> UTF-8
    size_t len = wcstombs(NULL, json, 0);
    if (len == (size_t)-1) return NULL;

    char* utf8 = malloc(len + 1);
    if (!utf8) return NULL;

    wcstombs(utf8, json, len + 1);

    root = cJSON_Parse(utf8);
    free(utf8);
#else
    root = cJSON_Parse(json);
#endif
    return root;
}


NSISFUNC(Set) {
    EXDLL_INIT();

    PTCHAR arg = (PTCHAR)GlobalAlloc(GPTR, sizeof(TCHAR) * string_size);
    TCHAR* json = NULL;

    TCHAR tree_name[64] = TEXT("default");

    http_config_t* http_config = http_config_new();

    if (!arg)
        return;

    while (popstring(arg) == 0) {
        if (lstrcmpi(arg, TEXT("/tree")) == 0) {
            popstring(arg);
            lstrcpy(tree_name, arg);
        }
        else if (lstrcmpi(arg, TEXT("/buffer")) == 0) {
            popstring(arg);
            int len = lstrlen(arg) + 1;
            json = (TCHAR*)GlobalAlloc(GPTR, len);
            lstrcpy(json, arg);
        }
        else if (lstrcmpi(arg, TEXT("/file")) == 0) {
            popstring(arg);

            DWORD outlength = 0;
            if (!read_file(arg, &json, &outlength)) {
                continue;
            }
        }
        else if (lstrcmpi(arg, TEXT("/url")) == 0) {
            popstring(arg);
            http_config_set_url(http_config, arg);
        }
        else if (lstrcmpi(arg, TEXT("/method")) == 0) {
            popstring(arg);
            http_config_set_method(http_config, arg);
        }
        else if (lstrcmpi(arg, TEXT("/body")) == 0) {
            popstring(arg);
            http_config_set_body(http_config, arg);
        }
        else if (lstrcmpi(arg, TEXT("/header")) == 0) {
            popstring(arg);
            http_config_add_header(http_config, arg);
        }
        else if (lstrcmpi(arg, TEXT("/user")) == 0) {
            popstring(arg);
            http_config_set_username(http_config, arg);
        }
        else if (lstrcmpi(arg, TEXT("/pass")) == 0) {
            popstring(arg);
            http_config_set_password(http_config, arg);
        }
    }

    if (http_config->url) {
        if (!http_fetch(http_config, &json)) {
            pushstring(TEXT("0"));
            goto cleanup;
        }
    }

    if (!json) {
        GlobalFree(json);
        pushstring(TEXT("0"));
        return;
    }

    cJSON* root = cJSON_ParseT(json);
    //free(json);

    if (!root) {
        pushstring(TEXT("0"));
        goto cleanup;
    }

    tree_set(tree_name, root);

    pushstring(TEXT("1"));

cleanup:
    http_config_free(http_config);
    GlobalFree(arg);
}


NSISFUNC(Get) {
    EXDLL_INIT();

    TCHAR tree_name[64] = TEXT("default");

    PTCHAR arg = (PTCHAR)GlobalAlloc(GPTR, sizeof(TCHAR) * string_size);
    char* key = (char*)GlobalAlloc(GPTR, string_size);

    if (!arg || !key) {
        if (arg) GlobalFree(arg);
        if (key) GlobalFree(key);
        pushstring(TEXT(""));
        return;
    }

    // First pass: check for /tree
    while (popstring(arg) == 0) {
        if (lstrcmpi(arg, TEXT("/tree")) == 0) {
            popstring(arg);
            lstrcpy(tree_name, arg);
        }
        else {
            pushstring(arg);
            break;
        }
    }

    cJSON* node = tree_get_root(tree_name);

    if (!node) {
        pushstring(TEXT(""));
        goto cleanup;
    }

    while (popstring(arg) == 0) {
        if (is_number(arg)) {
            int index = _ttoi(arg);

            if (!cJSON_IsArray(node)) {
                node = NULL;
                break;
            }

            node = cJSON_GetArrayItem(node, index);
        }
        else {
            tchar_to_utf8(arg, key, string_size);

            if (!cJSON_IsObject(node)) {
                node = NULL;
                break;
            }

            node = cJSON_GetObjectItem(node, key);
        }

        if (!node)
            break;
    }

    if (!node) {
        pushstring(TEXT(""));
    }
    else if (cJSON_IsString(node)) {
        PTCHAR out = (PTCHAR)GlobalAlloc(GPTR, sizeof(TCHAR) * string_size);
        if (out) {
            utf8_to_tchar(node->valuestring, out, string_size);
            pushstring(out);
            GlobalFree(out);
        }
        else {
            pushstring(TEXT(""));
        }
    }
    //else if (cJSON_IsNumber(node)) {
    //    TCHAR out[64];
    //    _sntprintf(out, 64, TEXT("%g"), node->valuedouble);
    //    pushstring(out);
    //}
    else if (cJSON_IsBool(node)) {
        pushstring(cJSON_IsTrue(node) ? TEXT("1") : TEXT("0"));
    }
    else {
        pushstring(TEXT(""));
    }

    // cleanup
cleanup:
    GlobalFree(arg);
    GlobalFree(key);
}

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved) {
  g_hInstance = hInst;
  return TRUE;
}

void __declspec(dllexport) Unload(HWND a, int b, TCHAR* c, stack_t** d, extra_parameters* e) {
    // Free your linked list here
    tree_clear();
}