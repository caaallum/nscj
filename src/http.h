#ifndef __NSCJ_HTTP_H
#define __NSCJ_HTTP_H

/**********************************************************
 * \brief Create http request
 *
 * \param url Request url
 * \param method Request method
 * \param headers Request headers
 * \param body Request body
 * \param username Request auth username
 * \param password Request auth password
 * \param response Response output, to be freed by caller
 * 
 * \returns TRUE on success
 **********************************************************/
BOOL http_fetch(_In_ TCHAR* url, _In_ TCHAR* method, _In_ TCHAR* headers, _In_ TCHAR* body, _In_ TCHAR* username, _In_ TCHAR* password, _Out_ TCHAR** response);

#endif /* __NSCJ_HTTP_H */