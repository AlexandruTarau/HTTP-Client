# HTTP-Client
## Description
This code is a C implementation for a client that interacts with a server using HTTP requests. It provides functionality for user authentication, accessing a library, and managing books.

## Commands
- `register`: Register a new user.
- `login`: Log in with existing credentials.
- `enter_library`: Access the library.
- `get_books`: Get informations about all the books from the library.
- `get_book`: Get information about a specific book.
- `add_book`: Add a new book to the library.
- `delete_book`: Delete a book from the library.
- `logout`: Log out from the current session.
- `exit`: Exit the program.

## Functionalities
- Using `parson.h` for working with JSON format.
- Storing server response in a `buffer` structure.
- When logging in, storing a cookie received from the server which grants access to site functionalities.
- When entering the library, storing a token received from the server which grants the possibility of accessing books, adding and deleting them.
- When logging out, reseting the cookie and token, making it impossible to use mentioned functionalities until logging in.

## Headers
- `requests.h`
    - compute_get_request -> computes a HTTP GET type request message.
    - compute_post_request -> computes a HTTP POST type request message.
    - compute_post_request -> computes a HTTP DELETE type request message.
- `helpers.h`
    - Contains data about the server (HOST, PORT).
    - Has functions that facilitates communication with the server and functions that extract a json array/object from a server response.
- `buffer.h`
    - Contains a `buffer` data structure which helps in storing the server response.
    - Has functions that work on the buffer (init/add/destroy/...).
- `parson.h`
    - Contains functions that facilitate work on JSON messages.

## Source code (client)
- `get_server_message`
    - Sends a request to the server and receives the response
- `get_status_code`
    - Extracts the HTTP status code from the server's response
- `main`
    - Main function that runs the client
    - Implements a command-line interface for interacting with a server.
    - It supports commands for registering a new user, logging in, entering the library, getting a list of books, getting details of a specific book, adding a new book, deleting a book, logging out, and exiting the program.
    - The function continuously reads commands from standard input, processes each command and performs the appropriate actions by communicating with the server through HTTP requests.
    - Each command (register, login, enter_library, ...) involves building a request message (GET, POST, or DELETE), sending it to the server using get_server_message and then handling the server's response by checking the status code and printing appropriate messages.

## Notes
- User authentication is required for accessing library functionalities (enter_library, get_books, add_book...).
- Ensure valid input format for commands and data.
- Error handling is implemented for various scenarios, including invalid input, authentication failure, and server errors.
