/*----------------------------------------------------------------------------------------------------------------------------*
 * File: linuxweb.c
 *
 * Description:
 * This file contains all functions necessary to create a simple web server on linux.
 *
 * Author: Fabian Schuster
 * Created: 18.02.2023
 *
 * Copyright: 2023 Fabian Schuster
 * License: NONE
 *----------------------------------------------------------------------------------------------------------------------------*/
#include "linuxweb.h"

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

// starts the webserver and returns the socket, which was created
int startWebServer(char port[]) {
	printf("Initiating Listener Socket\n");
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, optval = 1;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;      // Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;  // Datagram socket
	hints.ai_flags = AI_PASSIVE;      // For wildcard IP address
	hints.ai_protocol = 0;            // Any protocol
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	// Resolve the local address and port to be used by the server
	s = getaddrinfo(NULL, port, &hints, &result);
	if (s != 0) {
		printf("getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	/* getaddrinfo() returns a list of address structures.
	   Try each address until we successfully bind(2).
	   If socket(2) (or bind(2)) fails, we (close the socket
	   and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
		             rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;  // Success

		close(sfd);
	}

	// No address succeeded
	if (rp == NULL) {
		printf("Could not bind\n");
		return -1;
	}

	freeaddrinfo(result);

	if (listen(sfd, SOMAXCONN) == -1) {
		perror("listen");
		exit(1);
	}

	printf("Startup completet! Now listening on port %s for incoming connections...\n\n", port);

	return sfd;
}

// ready the request and modify the given pointers
void parse_request(char *recvbuf, char *method, char *path, char *body, int buflen) {
	char *token, *start;
	int path_length, body_length, method_length;

	// Find the start of the body
	start = strstr(recvbuf, "\r\n\r\n");

	if (start != NULL) {
		start += 4;
		body_length = snprintf(body, buflen, "%s", start);
	} else {
		printf("Error: body not found in request\n");
		strcpy(body, "");
		return;
	}

	// slice the request
	// important: this adds \0 at each space!
	token = strtok(recvbuf, " ");

	if (token != NULL) {
		start = token;
		method_length = snprintf(method, buflen, "%s", start);
	} else {
		printf("Error: invalid request\n");
		return;
	}

	token = strtok(NULL, " ");

	if (token != NULL) {
		start = token;
		path_length = snprintf(path, buflen, "%s", start);
	} else {
		printf("Error: invalid request\n");
		return;
	}
}

// handles the request and sends the response. This function is given to the thread!
void *handleClient(void *args) {
	printf("[#%ld]<-New Thread created\n", pthread_self());
	THREAD_DATA *params = (THREAD_DATA *)args;  // cast the void pointer to the correct type
	if (params->buflen == 0)
		params->buflen = DEFAULT_BUFLEN;
	int iResult;

	// recieve and send data to the client
	char recvbuf[params->buflen];
	int iSendResult;
	int recvbuflen = params->buflen;

	// allocate memory for the parsed request data
	HTTP_REQUEST *request = (HTTP_REQUEST *)malloc(sizeof(HTTP_REQUEST));
	request->method = (char *)malloc(sizeof(char) * DEFAULT_BUFLEN);
	request->path = (char *)malloc(sizeof(char) * DEFAULT_BUFLEN);
	request->body = (char *)malloc(sizeof(char) * DEFAULT_BUFLEN);

	// Receive until the peer shuts down the connection
	do {
		// clean the buffer
		memset(recvbuf, 0, recvbuflen);
		// recieve data from the client
		iResult = recv(params->clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			// clean the request data
			strcpy(request->method, "");
			strcpy(request->path, "");
			strcpy(request->body, "");

			// parse the request
			parse_request(recvbuf, request->method, request->path, request->body, params->buflen);

			// call the callback function, which handles the request
			(*(params->callback))(request, params->clientSocket);
		} else if (iResult == 0) {
			// connection closed by client

		} else {
			printf("recv failed: %d\n", errno);
			close(params->clientSocket);
			free(params);
			return NULL;
		}
	} while (iResult > 0);

	// free the request data
	free(request->method);
	free(request->path);
	free(request->body);
	free(request);

	// cleanup
	close(params->clientSocket);
	free(params);

	printf("[#%ld]<-Thread closed\n", pthread_self());

	return NULL;
}

// accepts a client and creates a new thread for it
int acceptClient(int serverSocket, int buflen, void (*callback)(HTTP_REQUEST *, int)) {
	// accept a client socket
	int clientSocket;
	clientSocket = -1;
	clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket == -1) {
		perror("accept failed");
		close(serverSocket);
		return 1;
	}

	// create a new thread data struct, because only one argument can be passed to the thread
	THREAD_DATA *data = (THREAD_DATA *)malloc(sizeof(THREAD_DATA));
	data->clientSocket = clientSocket;
	data->buflen = buflen;
	data->callback = callback;

	// create a new worker thread for the client
	pthread_t tid;
	int err = pthread_create(&tid, NULL, handleClient, (void *)data);
	if (err != 0) {
		perror("Error creating worker thread");
		close(clientSocket);
		return 1;
	}
	pthread_detach(tid);
}

// sends a response to the client
int sendResponse(int clientSocket, char *response) {
	int iSendResult;
	char *fullResponse = (char *)malloc(sizeof(char) * (strlen(response) + 200));
	memset(fullResponse, 0, strlen(response) + 200);
	// add the http header
	sprintf(fullResponse, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", strlen(response));
	// add the response (the html file)
	strcat(fullResponse, response);
	iSendResult = send(clientSocket, fullResponse, strlen(fullResponse), 0);

	free(fullResponse);

	if (iSendResult == -1) {
		printf("send failed\n");
		close(clientSocket);
		return 1;
	}
	return 0;
}

// same as above but sends a 404 response
int sendNotFound(int clientSocket, char *response) {
	int iSendResult;
	char *fullResponse = (char *)malloc(sizeof(char) * (strlen(response) + 200));
	memset(fullResponse, 0, strlen(response) + 200);
	sprintf(fullResponse, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", strlen(response));
	strcat(fullResponse, response);
	iSendResult = send(clientSocket, fullResponse, strlen(fullResponse), 0);

	free(fullResponse);

	if (iSendResult == -1) {
		perror("send failed");
		close(clientSocket);
		return 1;
	}
	return 0;
}

// stop the serer
int stopWebServer(int listenSocket) {
	// Close the socket when we are done listening for connections
	close(listenSocket);
	printf("Server stopped!\n");
	return 0;
}

// parse the request body of queary string int a map
// example: "key1=value1&key2=value2" -> {key1: value1, key2: value2}
MAP *parse(char *data) {
	MAP *first = NULL;
	MAP *curr = NULL;
	char *token = strtok(data, "&");  // split the string by &
	char *start;
	while (token != NULL) {
		MAP *new = (MAP *)malloc(sizeof(MAP));
		new->key = (char *)malloc(sizeof(char) * DEFAULT_BUFLEN);    //! important Default_buflen is a bit to big but I don't want to choose the buffer to small xD
		new->value = (char *)malloc(sizeof(char) * DEFAULT_BUFLEN);  //! important Default_buflen is a bit to big but I don't want to choose the buffer to small xD
		new->next = NULL;
		if (first != NULL) {
			curr->next = new;
		} else {
			first = new;
		}
		curr = new;

		start = token;
		char *key = (char *)malloc(sizeof(char) * DEFAULT_BUFLEN);    //! important Default_buflen is a bit to big but I don't want to choose the buffer to small xD
		char *value = (char *)malloc(sizeof(char) * DEFAULT_BUFLEN);  //! important Default_buflen is a bit to big but I don't want to choose the buffer to small xD
		char *keyStart = strstr(start, "=");
		if (keyStart != NULL) {
			keyStart += 1;
			snprintf(key, DEFAULT_BUFLEN, "%s", start);
			key[keyStart - start - 1] = '\0';
			snprintf(value, DEFAULT_BUFLEN, "%s", keyStart);
		} else {
			printf("Error: invalid request\n");
			return NULL;
		}
		curr->key = key;
		curr->value = value;
		token = strtok(NULL, "&");
	}

	return first;
}
