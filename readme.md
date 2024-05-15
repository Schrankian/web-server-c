# Project

A simple web-server with c as the backend and html as the frontend. It only runs on linux.
This is an example project which serves no purpose and was simply for learning some low level concepts.

## Compile

To compile with gcc, run the following command (on an linux machine):

`gcc webapp.c database.c filemanager.c linuxweb.c -o webapp -pthread -lm`

## How to Start/Stop the server

Start the server with `./webapp` (The file needs to be executable. Do this with `chmod +x webapp`). An `database.txt` file needs to be present in the the working directory as well as the `pages` folder which includes the html sites<br>
Stop the server via an http-call to the `/stopServer` endpoint to save the database in the process. But this method only executes once a second, finishing call is made.
Another method is to just close the program by force, which also works but doesnt save the database in the proces.

## Program flow

- The database is created
- The database is filled with values from the database.txt file
- A socket for our app is requested from the kernel on which it listens for incoming requests. The port for this socket is hardcoded in the `webapp.c` file (3333)
- Every incoming request is added onto an stack. This stack can then be processed with the `acceptClient` function, which takes a buffer size and an callback function which is called for every request.
- An request is picked and registered as a new thread, so that multiple requests can be processed in parallel.
- For the request, a struct with method, route and body is created and passed to the callback function
- Depending on the request and the logic defined in the callback function, some actions are performed
- If the STOPSERVER variable is 1, the next try to process an request breaks out of the while loop and the program begins to close
- The socket is freed
- The database is dumped into an string and saved into the `database.txt` file
- At last, the allocated memory for the database is freed

## Explanation of the files

### webapp.c

This is the entry point of the program which contains the main function. All headers of the other files must be included in this function (this is not necessary in the files themselves. Example: if database.c does not need any declarations from database.h, database.h does not have to be included in database.c, as database.h only has function prototypes. However, since all functions of the other files are required in webapp.c, all header files must also be included in it). The file is actually only there to call the functions of the other files and thus provide a clear overview of the functionality. It is important that the callback that is passed to each request is declared in this file.

### database.c

This contains all the functions for a very simple database. The database is a linked list of `Item` elements. The start element, which is passed to all functions as a database pointer, is simply an element with the value NULL, which marks the start. If a function is then called, this start element is passed, which allows iteration through the list thanks to the next pointer.<br>
When a new element is added, the system looks directly at where it belongs alphabetically, so that the list is sorted alphabetically at all times.<br>
The CreateHTML method is special because it fills the datatable.html document with the values from the database. Here, `§` is used as a placeholder.

### filemanager.c

This file contains two functions for saving and reading files.

### linuxweb.c

This file contains all the functions required to operate a web server. These are
- Register a socket
- Accepting requests and setting them to a new thread
- Processing requests
- Parsing the requests so that the data is easy to use
- Send responses to the client (200 Success and 404 Not found)
- Parse a map data type so that the body sent by the client can be easily processed as key-value pairs
The functionality of the registration of a web server has already been explained above


