#include <stddef.h>   // for size_t
#include <stdarg.h>   // for va_list
#include <setjmp.h>   // for setjmp/longjmp
#define CMOCKA_STATIC 1
#include <cmocka.h>

#include <windows.h>
#include <stdio.h>
#include "../nsis/pluginapi.h"
#include "../src/util.h"

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
    while (p) {
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
// Forward-declare nscj functions
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
// String comparison wrapper for cmocka
// -----------------------------------------------------------------------
#define assert_tchar_equal(expected, actual)           \
    do {                                              \
        CHAR exp_utf8[TEST_STRING_SIZE];                \
        CHAR act_utf8[TEST_STRING_SIZE];                \
        tchar_to_utf8(expected, exp_utf8, TEST_STRING_SIZE); \
        tchar_to_utf8(actual, act_utf8, TEST_STRING_SIZE);   \
        assert_string_equal(exp_utf8, act_utf8);     \
    } while(0)

// -----------------------------------------------------------------------
// Individual tests
// -----------------------------------------------------------------------

static void
test_url_set_get_default(void** state) {
    TCHAR result[TEST_STRING_SIZE];
    reset();

    // Set json 
    pushstring(_T("https://jsonplaceholder.typicode.com/users/1"));
    pushstring(_T("/url"));

    CALL_PLUGIN(Set);
    popstring(result);

    assert_tchar_equal(result, _T("1"));

    // Get name
    pushstring(_T("name"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("Leanne Graham"));

    // Get email
    pushstring(_T("email"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("Sincere@april.biz"));
}

static void
test_url_set_get_single_tree(void** state) {
    TCHAR result[TEST_STRING_SIZE];
    reset();

    // Set json 
    pushstring(_T("https://jsonplaceholder.typicode.com/users/1"));
    pushstring(_T("/url"));
    pushstring(_T("tree1"));
    pushstring(_T("/tree"));

    CALL_PLUGIN(Set);
    popstring(result);

    assert_tchar_equal(result, _T("1"));

    // Get name
    pushstring(_T("name"));
    pushstring(_T("tree1"));
    pushstring(_T("/tree"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("Leanne Graham"));

    // Get email
    pushstring(_T("email"));
    pushstring(_T("tree1"));
    pushstring(_T("/tree"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("Sincere@april.biz"));
}

static void
test_url_set_get_multi_tree(void** state) {
    TCHAR result[TEST_STRING_SIZE];
    reset();

    // Set user1 json 
    pushstring(_T("https://jsonplaceholder.typicode.com/users/1"));
    pushstring(_T("/url"));
    pushstring(_T("user1"));
    pushstring(_T("/tree"));
    CALL_PLUGIN(Set);
    popstring(result);
    assert_tchar_equal(result, _T("1"));

    // Set user2 json
    pushstring(_T("https://jsonplaceholder.typicode.com/users/2"));
    pushstring(_T("/url"));
    pushstring(_T("user2"));
    pushstring(_T("/tree"));
    CALL_PLUGIN(Set);
    popstring(result);
    assert_tchar_equal(result, _T("1"));

    // Get user1 name
    pushstring(_T("name"));
    pushstring(_T("user1"));
    pushstring(_T("/tree"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("Leanne Graham"));

    // Get user1 email
    pushstring(_T("email"));
    pushstring(_T("user1"));
    pushstring(_T("/tree"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("Sincere@april.biz"));

    // Get user2 name
    pushstring(_T("name"));
    pushstring(_T("user2"));
    pushstring(_T("/tree"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("Ervin Howell"));

    // Get user2 email
    pushstring(_T("email"));
    pushstring(_T("user2"));
    pushstring(_T("/tree"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("Shanna@melissa.tv"));
}

static void
test_buffer_set_get_default(void** state) {
    TCHAR result[TEST_STRING_SIZE];
    reset();

    // Set json
    pushstring(_T("{\"type\": \"buffer\"}"));
    pushstring(_T("/buffer"));

    CALL_PLUGIN(Set);
    popstring(result);

    assert_tchar_equal(result, _T("1"));

    pushstring(_T("type"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("buffer"));
}

static void
test_file_set_get_default(void** state) {
    TCHAR result[TEST_STRING_SIZE];
    reset();

    FILE* f = _tfopen(_T("test.json"), _T("w"));
    _fputts(_T("{\"type\": \"file\"}"), f);
    fclose(f);

    pushstring(_T("test.json"));
    pushstring(_T("/file"));

    CALL_PLUGIN(Set);
    popstring(result);

    assert_tchar_equal(result, _T("1"));

    pushstring(_T("type"));
    CALL_PLUGIN(Get);
    popstring(result);
    assert_tchar_equal(result, _T("file"));
}

int 
main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_url_set_get_default),
        cmocka_unit_test(test_url_set_get_single_tree),
        cmocka_unit_test(test_url_set_get_multi_tree),
        cmocka_unit_test(test_buffer_set_get_default),
        cmocka_unit_test(test_file_set_get_default),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}