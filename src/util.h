#ifndef __NSCJ_UTIL_H
#define __NSCJ_UTIL_H

/**********************************************************
 * \brief Convert TCHAR to utf8 char
 * 
 * \param src TCHAR to convert
 * \param dst Destination char*
 * \param max Max length
 **********************************************************/
void tchar_to_utf8(const TCHAR* src, char* dst, int max);

/**********************************************************
 * \brief Convert utf8 to TCHAR
 *
 * \param src char* to convert
 * \param dst Destination TCHAR
 * \param max Max length
 **********************************************************/
void utf8_to_tchar(const char* src, TCHAR* dst, int max);

/**********************************************************
 * \brief Convert TCHAR to wchar_t
 *
 * \param src TCHAR to convert
 * \param dst Destination tchar_t
 * \param max Max length
 **********************************************************/
void tchar_to_wchar(const TCHAR* src, wchar_t* dst, int max);

/**********************************************************
 * \brief Check if string is anumber
 *
 * \param src TCHAR to check
 * 
 * \returns BOOL true if number false otherwise
 **********************************************************/
BOOL is_number(const TCHAR* str);

#endif /* __NSCJ_UTIL_H */