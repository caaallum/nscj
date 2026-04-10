#ifndef __NSCJ_HTTP_H
#define __NSCJ_HTTP_H

typedef struct {
    PWCHAR url;
    PWCHAR method;
    PWCHAR headers;
    PCHAR body;
    PWCHAR username;
    PWCHAR password;
} http_config_t;

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
_Success_(return) BOOL http_fetch(_In_ http_config_t* config, _Out_ TCHAR** response);

/**********************************************************
 * \brief Create http_config_t with default values
 * 
 * \returns Pointer to http_config_t, to be freed using http_config_free
 **********************************************************/
http_config_t* http_config_new();

/**********************************************************
 * \brief Set http_config_t url field
 * 
 * \param this Pointer to http_config_t
 * \param url URL string, to be copied into http_config_t
 **********************************************************/
VOID http_config_set_url(_In_ http_config_t* this, _In_ LPCTSTR url);

/**********************************************************
 * \brief Set http_config_t method field
 * 
 * \param this Pointer to http_config_t
 * \param method HTTP method string (e.g. "GET", "POST"), to be copied into http_config_t
 **********************************************************/
VOID http_config_set_method(_In_ http_config_t* this, _In_ LPCTSTR method);

/**********************************************************
 * \brief Set http_config_t headers field
 * 
 * \param this Pointer to http_config_t
 * \param headers HTTP headers string (e.g. "Content-Type: application/json\r\n"), to be copied into http_config_t
 **********************************************************/
VOID http_config_set_headers(_In_ http_config_t* this, _In_ LPCTSTR headers);

/**********************************************************
 * \brief Add a header to http_config_t headers field
 * 
 * \param this Pointer to http_config_t
 * \param header HTTP header string (e.g. "Authorization: Bearer <token>\r\n"), to be appended to http_config_t headers
 **********************************************************/
VOID http_config_add_header(_In_ http_config_t* this, _In_ LPCTSTR header);

/**********************************************************
 * \brief Set http_config_t body field
 * 
 * \param this Pointer to http_config_t
 * \param body HTTP request body string, to be copied into http_config_t
 **********************************************************/
VOID http_config_set_body(_In_ http_config_t* this, _In_ LPCTSTR body);

/**********************************************************
 * \brief Set http_config_t username field
 * 
 * \param this Pointer to http_config_t
 * \param username HTTP auth username string, to be copied into http_config_t
 **********************************************************/
VOID http_config_set_username(_In_ http_config_t* this, _In_ LPCTSTR username);

/**********************************************************
 * \brief Set http_config_t password field
 * 
 * \param this Pointer to http_config_t
 * \param password HTTP auth password string, to be copied into http_config_t
 **********************************************************/
VOID http_config_set_password(_In_ http_config_t* this, _In_ LPCTSTR password);

/**********************************************************
 * \brief Free http_config_t and its fields
 * 
 * \param config Pointer to http_config_t to free
 **********************************************************/
VOID http_config_free(_In_ http_config_t* config);

#endif /* __NSCJ_HTTP_H */