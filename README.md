[![Build](https://github.com/caaallum/nscj/actions/workflows/build.yml/badge.svg)](https://github.com/caaallum/nscj/actions/workflows/build.yml)
[![Test](https://github.com/caaallum/nscj/actions/workflows/test.yml/badge.svg)](https://github.com/caaallum/nscj/actions/workflows/test.yml)

# nscj — NSIS JSON Parser

A lightweight NSIS plugin for loading, fetching, and querying JSON data. Supports reading JSON from a string buffer, a file, or a remote URL via HTTP, and navigating deeply nested structures using a simple key/index traversal syntax.

Multiple independent JSON trees can be stored simultaneously and referenced by name.

---

## Functions

### `nscj::Set`

Loads JSON data into a named tree from a buffer, file, or remote URL. Optionally configure the HTTP request method, headers, and authentication.

**Syntax**
```nsis
nscj::Set [/tree "name"] <source> [http options...]
Pop $result
```

**Parameters**

| Parameter | Value | Description |
|-----------|-------|-------------|
| `/tree` | `"name"` | *(Optional)* Name of the tree to store data in. Defaults to `"default"` if omitted |
| `/buffer` | `"{ ... }"` | Load JSON directly from an inline string |
| `/file` | `"path"` | Load JSON from a file on disk |
| `/url` | `"https://..."` | Fetch JSON from a remote URL |
| `/method` | `"POST"` etc. | *(Optional)* HTTP method. Defaults to `GET` |
| `/body` | `"..."` | *(Optional)* HTTP request body |
| `/header` | `"Key: Value"` | *(Optional)* Add an HTTP request header. Can be repeated for multiple headers |
| `/user` | `"username"` | *(Optional)* HTTP basic auth username |
| `/pass` | `"password"` | *(Optional)* HTTP basic auth password |

**Return value (via `Pop`)**

| Value | Meaning |
|-------|---------|
| `"1"` | JSON was loaded and parsed successfully |
| `"0"` | Loading or parsing failed |

**Examples**
```nsis
; Load from an inline buffer into the default tree
nscj::Set /buffer '{"name":"Alice","age":30}'
Pop $0

; Load from a file
nscj::Set /file "$INSTDIR\config.json"
Pop $0

; Fetch from a URL into the default tree
nscj::Set /url "https://jsonplaceholder.typicode.com/users/1"
Pop $0

; Fetch from a URL into a named tree
nscj::Set /tree "user1" /url "https://jsonplaceholder.typicode.com/users/1"
Pop $0

; POST request with a body and custom header
nscj::Set /url "https://api.example.com/data" \
    /method "POST" \
    /body '{"query":"value"}' \
    /header "Authorization: Bearer mytoken" \
    /header "Content-Type: application/json"
Pop $0
```

---

### `nscj::Get`

Retrieves a value from a loaded JSON tree by traversing a path of object keys and array indices.

**Syntax**
```nsis
nscj::Get [/tree "name"] ["key"] [index] [...]
Pop $result
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `/tree` | `"name"` | *(Optional)* Name of the tree to read from. Defaults to `"default"` if omitted |
| `"key"` | String | An object key to traverse into |
| `index` | Integer | An array index to traverse into |

Path segments are resolved in order from left to right. You can chain as many keys and indices as needed to navigate nested structures.

**Return value (via `Pop`)**

| Value | Meaning |
|-------|---------|
| `"<value>"` | The string value of the resolved node |
| `"1"` / `"0"` | Boolean value (`true` / `false`) |
| `""` | Path could not be resolved, or value type is not supported |

> **Note:** Number values are not currently returned — numeric nodes resolve to `""`. Use string fields where possible, or convert numbers to strings in your API/JSON source.

**Examples**
```nsis
; Get a top-level key from the default tree
nscj::Get "name"
Pop $0
DetailPrint "Name: $0"

; Get a nested key
nscj::Get "address" "city"
Pop $0
DetailPrint "City: $0"

; Get an item from an array
nscj::Get "tags" 0
Pop $0
DetailPrint "First tag: $0"

; Get a value from a named tree
nscj::Get /tree "user1" "name"
Pop $0
DetailPrint "User 1 name: $0"
```

---

## Full Example

This example fetches three users from a REST API into separate named trees and then queries each one independently.

```nsis
Section

    ; Fetch users into named trees
    nscj::Set /url "https://jsonplaceholder.typicode.com/users/1"
    Pop $0
    DetailPrint "Fetch default: $0"

    nscj::Set /tree "user1" /url "https://jsonplaceholder.typicode.com/users/2"
    Pop $0
    DetailPrint "Fetch user1: $0"

    nscj::Set /tree "user2" /url "https://jsonplaceholder.typicode.com/users/3"
    Pop $0
    DetailPrint "Fetch user2: $0"

    ; Query each tree
    nscj::Get "name"
    Pop $0
    DetailPrint "Default name: $0"

    nscj::Get /tree "user1" "name"
    Pop $0
    DetailPrint "User1 name: $0"

    nscj::Get /tree "user2" "name"
    Pop $0
    DetailPrint "User2 name: $0"

    ; Navigate a nested object
    nscj::Get "address" "city"
    Pop $0
    DetailPrint "Default city: $0"

SectionEnd
```

**Output:**
```
Fetch default: 1
Fetch user1: 1
Fetch user2: 1
Default name: Leanne Graham
User1 name: Ervin Howell
User2 name: Clementine Bauch
Default city: Gwenborough
```

---

## Notes

- If `/tree` is omitted in either `Set` or `Get`, the `"default"` tree is used.
- Multiple trees are independent — data stored in one tree does not affect another.
- For `Set`, `/buffer`, `/file`, and `/url` are mutually exclusive. If both `/url` and a local source are provided, the URL takes precedence.
- `/header` may be specified multiple times to add multiple HTTP request headers.
- `Get` returns `""` for any path that cannot be resolved, including type mismatches (e.g. treating an object as an array) and unsupported value types such as numbers and null.