#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
 
    // Write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
 
    compute_message(message, line);
 
    // Clean up the line
    memset(line, 0, LINELEN);
 
    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Clean up the line
    memset(line, 0, LINELEN);

    if (token) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Clean up the line
    memset(line, 0, LINELEN);
 
    // Add headers and/or cookies, according to the protocol format
    if (cookies) {
        sprintf(line, "Cookie: ");
        int i, len = 8;
        for (i = 0; i < cookies_count - 1; i++) {
            sprintf(line + len, "%s; ", cookies[i]);
            len += strlen(cookies[i]);
        }
        sprintf(line + len, "%s", cookies[i]);
        
        compute_message(message, line);
    }
 
    // Add final new line
    compute_message(message, "");

    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **payload,
                            int payload_length, char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Clean up the line
    memset(line, 0, LINELEN);

    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
 
    // Clean up the line
    memset(line, 0, LINELEN);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    // Clean up the line
    memset(line, 0, LINELEN);

    sprintf(line, "Content-Length: %d", payload_length);
    compute_message(message, line);

    // Clean up the line
    memset(line, 0, LINELEN);

    if (token) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Clean up the line
    memset(line, 0, LINELEN);

    // Add cookies
    if (cookies) {
        sprintf(line, "Cookie: ");
        int i, len = 8;
        for (i = 0; i < cookies_count - 1; i++) {
            sprintf(line + len, "%s; ", cookies[i]);
            len += strlen(cookies[i]);
        }
        sprintf(line + len, "%s", cookies[i]);
        
        compute_message(message, line);
    }

    // Add new line at end of header
    compute_message(message, "");

    // Add the actual payload data
    memset(line, 0, LINELEN);
    strcat(message, *payload);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
 
    // Write the method name, URL and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);
 
    compute_message(message, line);
 
    // Clean up the line
    memset(line, 0, LINELEN);
 
    // Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Clean up the line
    memset(line, 0, LINELEN);

    if (token) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Clean up the line
    memset(line, 0, LINELEN);
 
    // Add headers and/or cookies, according to the protocol format
    if (cookies) {
        sprintf(line, "Cookie: ");
        int i, len = 8;
        for (i = 0; i < cookies_count - 1; i++) {
            sprintf(line + len, "%s; ", cookies[i]);
            len += strlen(cookies[i]);
        }
        sprintf(line + len, "%s", cookies[i]);
        
        compute_message(message, line);
    }
 
    // Add final new line
    compute_message(message, "");

    return message;
}
