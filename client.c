#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include "buffer.h"

#define LEN 128

// Puts the message from the server in a buffer
void get_server_message(int sockfd, char *message, buffer *buff)
{
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    buff->data = receive_from_server(sockfd);
    buff->size = strlen(buff->data);
    close_connection(sockfd);
}

// Gets the status code from the server message
int get_status_code(buffer buff)
{
    char *string_code_start = buff.data +
                        buffer_find_insensitive(&buff, "HTTP/1.1", 8) + 9;
    char string_code[4];
    strncpy(string_code, string_code_start, 3);
    string_code[3] = '\0';
    return atoi(string_code);
}

int main(int argc, char *argv[])
{
    char *message;
    int sockfd;
    buffer buff = buffer_init();
    char *auth_cookie = NULL;
    char *token = NULL;

    while (1) {
        char buffer[LEN] = {0};
        if (fgets(buffer, LEN, stdin)) {
            if (!strcmp(buffer, "register\n")) {  // REGISTER
                char username[LEN], password[LEN];

                // Get credentials
                printf("username=");
                fgets(username, LEN, stdin);
                username[strlen(username) - 1] = '\0';

                printf("password=");
                fgets(password, LEN, stdin);
                password[strlen(password) - 1] = '\0';

                if (strlen(username) == 0 || strlen(password) == 0) {
                    printf("ERROR - Invalid credentials!\n");
                    continue;
                }

                if (strchr(username, ' ') || strchr(password, ' ')) {
                    printf("ERROR - No spaces allowed in the credentials!\n");
                    continue;
                }

                // Put the credentials in a JSON format
                JSON_Value *user_val = json_value_init_object();
                JSON_Object *user_obj = json_value_get_object(user_val);
                json_object_set_string(user_obj, "username", username);
                json_object_set_string(user_obj, "password", password);

                char *payload = json_serialize_to_string(user_val);

                // Send a POST request with the credentials and get a response
                message = compute_post_request(HOST,
                        "/api/v1/tema/auth/register", "application/json",
                        &payload, strlen(payload), NULL, 0, NULL);
                get_server_message(sockfd, message, &buff);

                // Check response status
                int code = get_status_code(buff);
                if (code == 200 || code == 201) {
                    printf("SUCCESS - User registered successfully!\n");
                } else {
                    printf("ERROR - User already exists!\n");
                }
                free(message);
            } else if (!strcmp(buffer, "login\n")) {  // LOGIN
                char username[LEN], password[LEN];

                // Get credentials
                printf("username=");
                fgets(username, LEN, stdin);
                username[strlen(username) - 1] = '\0';

                printf("password=");
                fgets(password, LEN, stdin);
                password[strlen(password) - 1] = '\0';

                if (strlen(username) == 0 || strlen(password) == 0) {
                    printf("ERROR - Invalid credentials!\n");
                    continue;
                }

                if (strchr(username, ' ') || strchr(password, ' ')) {
                    printf("ERROR - No spaces allowed in the credentials!\n");
                    continue;
                }

                // Put the credentials in a JSON format
                JSON_Value *user_val = json_value_init_object();
                JSON_Object *user_obj = json_value_get_object(user_val);
                json_object_set_string(user_obj, "username", username);
                json_object_set_string(user_obj, "password", password);

                char *payload = json_serialize_to_string(user_val);

                // Send a POST request with the credentials and get a response
                message = compute_post_request(HOST, "/api/v1/tema/auth/login",
                            "application/json", &payload, strlen(payload), NULL,
                            0, NULL);
                get_server_message(sockfd, message, &buff);

                // Check response status
                int code = get_status_code(buff);
                if (code == 200) {
                    // Get authentication cookie
                    char *aux = buff.data + buffer_find_insensitive(&buff,
                                                "connect.sid=", 12);
                    int token_len = strlen(aux) - strlen(strchr(aux, ';'));
                    if (auth_cookie == NULL) {
                        auth_cookie = (char *)malloc((token_len + 1) * sizeof(char));
                    }
                    strncpy(auth_cookie, aux, token_len);
                    auth_cookie[token_len] = '\0';

                    // Reset access token
                    if (token) {
                        free(token);
                        token = NULL;
                    }
                    printf("SUCCESS - User logged in successfully!\n");
                } else {
                    printf("ERROR - Incorrect username or password!\n");
                }
                free(message);
            } else if (!strcmp(buffer, "enter_library\n")) {  // Enter Library
                // Check if the user is logged in
                if (auth_cookie == NULL) {
                    printf("ERROR - You must be logged in first!\n");
                    continue;
                }

                /* Send a GET request with the authentication cookie and
                receive a response */
                message = compute_get_request(HOST,
                            "/api/v1/tema/library/access",
                            NULL, &auth_cookie, 1, NULL);
                get_server_message(sockfd, message, &buff);

                // Check response status
                int code = get_status_code(buff);
                if (code == 200) {
                    // Get new token
                    char *data = basic_extract_json_response(buff.data);
                    JSON_Value *json_data = json_parse_string(data);
                    char *aux = json_object_get_string(json_object(json_data), "token");

                    // Reset old token
                    if (token != NULL) {
                        free(token);
                        token = NULL;
                    }

                    // Replace old token
                    token = calloc(strlen(aux), sizeof(char));
                    strcpy(token, aux);
                    printf("SUCCESS - Access granted!\n");
                } else {
                    printf("ERROR - Unexpected error occured!\n");
                }
                free(message);
            } else if (!strcmp(buffer, "get_books\n")) {  // Get books
                // Check if the user has access to library
                if (token == NULL) {
                    printf("ERROR - Permissions denied!\n");
                    continue;
                }
                /* Send a GET request with the access token and
                receive a response */
                message = compute_get_request(HOST,
                            "/api/v1/tema/library/books", NULL,
                            NULL, 0, token);
                get_server_message(sockfd, message, &buff);

                // Check response status
                int code = get_status_code(buff);
                if (code == 200) {
                    // Get books
                    char *data = basic_extract_json_array_response(buff.data);
                    if (data == NULL) {
                        printf("[]\n");
                    }
                    JSON_Value *json_data = json_parse_string(data);
                    JSON_Array *json_array = json_value_get_array(json_data);
                    int n = json_array_get_count(json_array);

                    // Print all books
                    for (int i = 0; i < n; i++) {
                        JSON_Value *value = json_array_get_value(json_array, i);
                        printf("%s\n", json_serialize_to_string(value));
                    }
                }
                else {
                    printf("ERROR - Unexpected error occured!\n");
                }
                free(message);
            } else if (!strcmp(buffer, "get_book\n")) {  // Get book
                // Check if the user has access to library
                if (token == NULL) {
                    printf("ERROR - Permissions denied!\n");
                    continue;
                }

                // Read the id and compute the URL
                char aux[40] = "/api/v1/tema/library/books/";
                int id;
                printf("id=");
                int rc = scanf("%d", &id);

                // Clear the input buffer
                int c;
                while ((c = getchar()) != '\n' && c != EOF);

                if (rc == 0) {
                    printf("ERROR - ID must be a number!\n");
                    continue;
                }
                snprintf(aux + 27, 13, "%d", id);

                /* Send a GET request with the access token and
                book id and receive a response */
                message = compute_get_request(HOST, aux, NULL, NULL, 0, token);
                get_server_message(sockfd, message, &buff);

                // Check response status
                int code = get_status_code(buff);
                if (code == 200) {
                    // Get book info
                    char *data = basic_extract_json_response(buff.data);
                    JSON_Value *json_data = json_parse_string(data);

                    // Print book info
                    printf("%s\n", json_serialize_to_string(json_data));
                } else if (code == 404) {
                    printf("ERROR - Book with id=%d not found!\n", id);
                } else {
                    printf("ERROR - Unexpected error occured!\n");
                }
                free(message);
            } else if (!strcmp(buffer, "add_book\n")) {  // Add book
                // Check if the user has access to library
                if (token == NULL) {
                    printf("ERROR - Permissions denied!\n");
                    continue;
                }
                char title[LEN], author[LEN], genre[LEN], publisher[LEN];
                int page_count;

                // Get book info
                printf("title=");
                fgets(title, LEN, stdin);
                title[strlen(title) - 1] = '\0';

                printf("author=");
                fgets(author, LEN, stdin);
                author[strlen(author) - 1] = '\0';

                printf("genre=");
                fgets(genre, LEN, stdin);
                genre[strlen(genre) - 1] = '\0';

                printf("publisher=");
                fgets(publisher, LEN, stdin);
                publisher[strlen(publisher) - 1] = '\0';

                printf("page_count=");
                int rc = scanf("%d", &page_count);

                // Clear the input buffer
                int c;
                while ((c = getchar()) != '\n' && c != EOF);

                // Page count is not a number
                if (rc == 0) {
                    printf("ERROR - Invalid page count!\n");
                    continue;
                }

                // Book info is empty/starts with ' '
                if (strlen(title) == 0 || strlen(author) == 0 ||
                    strlen(genre) == 0 || strlen(publisher) == 0 ||
                    title[0] == ' ' || author[0] == ' ' ||
                    genre[0] == ' ' || publisher[0] == ' ') {
                    printf("ERROR - Invalid book information!\n");
                    continue;
                }

                // Put data in JSON format
                JSON_Value *book_val = json_value_init_object();
                JSON_Object *book_obj = json_value_get_object(book_val);
                json_object_set_string(book_obj, "title", title);
                json_object_set_string(book_obj, "author", author);
                json_object_set_string(book_obj, "genre", genre);
                json_object_set_number(book_obj, "page_count", page_count);
                json_object_set_string(book_obj, "publisher", publisher);

                char *payload = json_serialize_to_string(book_val);

                // Send a POST request with the book info and get a response
                message = compute_post_request(HOST,
                            "/api/v1/tema/library/books", "application/json",
                            &payload, strlen(payload), NULL, 0, token);
                get_server_message(sockfd, message, &buff);

                // Check response status
                int code = get_status_code(buff);
                if (code == 200) {
                    printf("SUCCESS - Book added successfully!\n");
                } else {
                    printf("ERROR - Unexpected error occured!\n");
                }
                free(message);
            } else if (!strcmp(buffer, "delete_book\n")) {  // Delete book
                // Check if the user has access to library
                if (token == NULL) {
                    printf("ERROR - Permissions denied!\n");
                    continue;
                }

                // Read the id and compute the URL
                char aux[40] = "/api/v1/tema/library/books/";
                int id;
                printf("id=");
                int rc = scanf("%d", &id);

                // Clear the input buffer
                int c;
                while ((c = getchar()) != '\n' && c != EOF);

                if (rc == 0) {
                    printf("ERROR - ID must be a number!\n");
                    continue;
                }
                snprintf(aux + 27, 13, "%d", id);

                // Send a DELETE request with the book id and get a response
                message = compute_delete_request(HOST, aux, NULL, 0, token);
                get_server_message(sockfd, message, &buff);

                // Check response status
                int code = get_status_code(buff);
                if (code == 200) {
                    printf("SUCCESS - Book deleted successfully!\n");
                } else if (code == 404) {
                    printf("ERROR - Book with id=%d not found!\n", id);
                } else {
                    printf("ERROR - Unexpected error occured!\n");
                }
                free(message);
            } else if (!strcmp(buffer, "logout\n")) {  // Logout
                // Check if the user is logged in
                if (auth_cookie == NULL) {
                    printf("ERROR - You are not currently logged in!\n");
                    continue;
                }

                /* Send a GET request with the authentication token and
                receive a response */
                message = compute_get_request(HOST, "/api/v1/tema/auth/logout",
                                NULL, &auth_cookie, 1, NULL);
                get_server_message(sockfd, message, &buff);

                // Check response status
                int code = get_status_code(buff);
                if (code == 200) {
                    // Reset auth cookie
                    free(auth_cookie);
                    auth_cookie = NULL;

                    // Reset access token
                    if (token) {
                        free(token);
                        token = NULL;
                    }
                    printf("SUCCESS - User logged out successfully!\n");
                } else {
                    printf("ERROR - Unexpected error occured!\n");
                }
                free(message);
            } else if (!strcmp(buffer, "exit\n")) {  // Exit
                if (auth_cookie) {
                    free(auth_cookie);
                }
                if (token) {
                    free(token);
                }
                break;
            } else {  // Invalid command
                printf("ERROR - Invalid command!\n");
            }
        }
    }

    return 0;
}
