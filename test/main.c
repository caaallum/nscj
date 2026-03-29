#include <windows.h>
#include <stdio.h>
#include "../nsis/pluginapi.h"

#define TEST_STRING_SIZE 1024

static TCHAR g_vars_storage[__INST_LAST * TEST_STRING_SIZE];

// Implement the three globals that pluginapi.c references via extern
unsigned int  g_stringsize = TEST_STRING_SIZE;
stack_t* g_stacktop_storage = NULL;
stack_t** g_stacktop = &g_stacktop_storage;
TCHAR* g_variables = g_vars_storage;

static void 
dump_stack(void) {
    stack_t* p = *g_stacktop;
    int i = 0;
    _tprintf(_T("  -- stack (top first) --\n"));
    while (p)
    {
        _tprintf(_T("  [%d] \"%s\"\n"), i++, p->text);
        p = p->next;
    }
    if (i == 0)
        _tprintf(_T("  (empty)\n"));
}

static void 
dump_vars(void) {
    static const TCHAR* names[] = {
        _T("$0"),  _T("$1"),  _T("$2"),  _T("$3"),  _T("$4"),
        _T("$5"),  _T("$6"),  _T("$7"),  _T("$8"),  _T("$9"),
        _T("$R0"), _T("$R1"), _T("$R2"), _T("$R3"), _T("$R4"),
        _T("$R5"), _T("$R6"), _T("$R7"), _T("$R8"), _T("$R9"),
        _T("$CMDLINE"), _T("$INSTDIR"), _T("$OUTDIR"),
        _T("$EXEDIR"), _T("$LANGUAGE")
    };
    _tprintf(_T("  -- variables --\n"));
    for (int i = 0; i < __INST_LAST; i++) {
        const TCHAR* val = getuservariable(i);
        if (val && val[0] != _T('\0'))
            _tprintf(_T("  %-12s = \"%s\"\n"), names[i], val);
    }
}

static void 
clear_stack(void) {
    while (*g_stacktop) {
        stack_t* top = *g_stacktop;
        *g_stacktop = top->next;
        GlobalFree((HGLOBAL)top);
    }
}

/** Zero all NSIS variables. */
static void 
clear_vars(void) {
    memset(g_vars_storage, 0, sizeof(g_vars_storage));
}

/** Reset everything between tests. */
static void 
reset(void) {
    clear_stack();
    clear_vars();
}

// -----------------------------------------------------------------------
// Forward-declare the plugin functions you want to test.
// -----------------------------------------------------------------------

// Example – replace with the real names from nscj.c
extern void __cdecl Get(HWND hwndParent, int string_size,
    TCHAR* variables, stack_t** stacktop, void* extra);
extern void __cdecl Set(HWND hwndParent, int string_size,
    TCHAR* variables, stack_t** stacktop, void* extra);

// -----------------------------------------------------------------------
// Convenience macro – calls a plugin function using the harness globals
// -----------------------------------------------------------------------

#define CALL_PLUGIN(fn) \
    fn(NULL, TEST_STRING_SIZE, g_variables, g_stacktop, NULL)

// -----------------------------------------------------------------------
// Individual tests
// -----------------------------------------------------------------------

static void 
test_set_and_get(void) {
    TCHAR result[TEST_STRING_SIZE];

    _tprintf(_T("\n=== test_set_and_get ===\n"));
    reset();

    // Simulate:  nscj::Set "myKey" "myValue"
    pushstring(_T("https://jsonplaceholder.typicode.com/users/1"));
    pushstring(_T("/url"));
    pushstring(_T("user1"));
    pushstring(_T("/tree"));

    _tprintf(_T("Before Set:\n"));
    dump_stack();

    CALL_PLUGIN(Set);
    popstring(result);
    _tprintf(_T("After Set:\n"));
    dump_stack();
    dump_vars();

    // Simulate:  nscj::Get "myKey"
    pushstring(_T("name"));
    pushstring(_T("user1"));
    pushstring(_T("/tree"));

    _tprintf(_T("Before Get:\n"));
    dump_stack();

    CALL_PLUGIN(Get);

    _tprintf(_T("After Get:\n"));
    dump_stack();

    // The plugin should have pushed the result
    if (popstring(result) == 0)
        _tprintf(_T("  Result = \"%s\"\n"), result);
    else
        _tprintf(_T("  ERROR: stack was empty after Get\n"));
}
//
//static void test_get_missing_key(void)
//{
//    TCHAR result[TEST_STRING_SIZE];
//
//    _tprintf(_T("\n=== test_get_missing_key ===\n"));
//    reset();
//
//    push(_T("nonExistentKey"));
//    CALL_PLUGIN(Get);
//
//    if (pop(result) == 0)
//        _tprintf(_T("  Result = \"%s\" (expected empty or error token)\n"), result);
//    else
//        _tprintf(_T("  Stack empty after Get on missing key\n"));
//}
//
//static void test_variable_roundtrip(void)
//{
//    _tprintf(_T("\n=== test_variable_roundtrip ===\n"));
//    reset();
//
//    // Write directly to $R0 and read it back
//    setuservariable(INST_R0, _T("hello from $R0"));
//    _tprintf(_T("  $R0 = \"%s\"\n"), getuservariable(INST_R0));
//
//    // Overwrite via the plugin path – push a new value and ask the
//    // plugin to store it, then read $R0 again
//    push(_T("updated value"));
//    push(_T("R0_key"));
//    CALL_PLUGIN(Set);
//    dump_vars();
//}

// -----------------------------------------------------------------------
// Entry point
// -----------------------------------------------------------------------

int 
main(void) {
    _tprintf(_T("NSIS plugin test harness\n"));
    _tprintf(_T("string_size = %u  |  __INST_LAST = %d\n"),
        TEST_STRING_SIZE, __INST_LAST);

    test_set_and_get();
    //test_get_missing_key();
    //test_variable_roundtrip();

    _tprintf(_T("\nDone.\n"));
    return 0;
}