/*----------------------------------------------------------------------------------------------------------------------------*
 * File: linuxweb.h
 *
 * Description:
 * This is the header file for linuxweb.c.
 *
 * Author: Fabian Schuster
 * Created: 18.02.2023
 *
 * Copyright: 2023 Fabian Schuster
 * License: NONE
 *----------------------------------------------------------------------------------------------------------------------------*/

// Include guard
#ifndef WINWEB_H
#define WINWEB_H
#endif

typedef struct HTTP_REQUEST {
	char* method;
	char* path;
	char* body;
} HTTP_REQUEST;

typedef struct THREAD_DATA {
	int clientSocket;
	int buflen;
	void (*callback)(HTTP_REQUEST*, int);
} THREAD_DATA;

typedef struct MAP MAP;
typedef struct MAP {
	char* key;
	char* value;
	MAP* next;
} MAP;

// starts the webserver and returns the socket, which was created
int startWebServer(char port[]);

// waits until a client connects to the server and give the rest to a worker thread
int acceptClient(int serverSocket, int buflen, void (*callback)(HTTP_REQUEST*, int));

// sends a response to the client which automatically adds the HTTP header
int sendResponse(int clientSocket, char* response);
int sendNotFound(int clientSocket, char* response);

// stops the webserver and cleans all resources
int stopWebServer(int serverSocket);

// parses either a request body or a query string and returns a map
MAP* parse(char* data);