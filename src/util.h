#ifndef __NSCJ_UTIL_H
#define __NSCJ_UTIL_H

/**********************************************************
 * \brief Convert TCHAR to utf8 char
 * 
 * \param src TCHAR to convert
 * \param dst Destination char*
 * \param max Max length
 **********************************************************/
VOID tchar_to_utf8(_In_ const TCHAR* src, _Out_ LPSTR dst, _In_ INT max);

/**********************************************************
 * \brief Convert utf8 to TCHAR
 *
 * \param src char* to convert
 * \param dst Destination TCHAR
 * \param max Max length
 **********************************************************/
VOID utf8_to_tchar(_In_ LPCSTR src, _Out_ TCHAR* dst, _In_ INT max);

/**********************************************************
 * \brief Convert TCHAR to wchar_t
 *
 * \param src TCHAR to convert
 * \param dst Destination tchar_t
 * \param max Max length
 **********************************************************/
VOID tchar_to_wchar(_In_ LPCTSTR src, _Out_ LPWSTR dst, _In_ INT max);

/**********************************************************
 * \brief Check if string is anumber
 *
 * \param src TCHAR to check
 * 
 * \returns BOOL true if number false otherwise
 **********************************************************/
BOOL is_number(_In_ LPCTSTR str);

/**********************************************************
 * \brief Read file content into buffer
 * 
 * \param filename File to read
 * \param buffer Buffer to read into
 * \param outlength Length of buffer read
 * 
 * \return BOOL true if success false otherwise
 **********************************************************/
BOOL read_file(_In_ LPCTSTR filename, _Out_ PTCHAR buffer, _Out_ LPDWORD outlength);

#endif /* __NSCJ_UTIL_H */