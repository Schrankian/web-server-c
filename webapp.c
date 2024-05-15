/*----------------------------------------------------------------------------------------------------------------------------*
 * File: web-app.c
 *
 * Description:
 * This is the code for Leons htw task.
 *
 * Author: Fabian Schuster
 * Created: 20.01.2023
 *
 * Copyright: 2023 Fabian Schuster
 * License: NONE
 *----------------------------------------------------------------------------------------------------------------------------*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "filemanager.h"
#include "linuxweb.h"

// register database as global variable
Item *database = NULL;
int STOPSERVER = 0;

void callback(HTTP_REQUEST *request, int clientSocket) {
	printf("\n[#%ld]New Request\n------------------------\nMethod: %s\nPath: %s\nBody: %s\n------------------------\n", pthread_self(), request->method, request->path, request->body);

	if (strcmp(request->path, "/favicon.ico") == 0) {
		// ignore favicon.ico
		return;
	} else if (strcmp(request->method, "GET") == 0 && (strcmp(request->path, "/") == 0 || strcmp(request->path, "/add") == 0)) {
		// return index.html
		printf("[#%ld]Sending response\n\n", pthread_self());
		char *document = createHTML(readFile("pages/datatable.html"), database);

		sendResponse(clientSocket, document);
		free(document);

		return;
	} else if (strcmp(request->method, "POST") == 0 && strcmp(request->path, "/add") == 0) {
		// add new item to database
		MAP *map = parse(request->body);
		int key = 0;
		char *name = NULL;
		int value = 0;
		while (map != NULL) {
			if (strcmp(map->key, "id") == 0) {
				key = atoi(map->value);
			} else if (strcmp(map->key, "name") == 0) {
				name = map->value;
			} else if (strcmp(map->key, "quantity") == 0) {
				value = atoi(map->value);
			}
			map = map->next;
		}
		insert(key, name, value, database);

		printf("[#%ld]Sending response\n\n", pthread_self());
		char *document = createHTML(readFile("pages/datatable.html"), database);
		sendResponse(clientSocket, document);
		free(document);
		return;
	} else if (strcmp(request->method, "POST") == 0 && strcmp(request->path, "/change") == 0) {
		// add new item to database
		MAP *map = parse(request->body);
		int key = 0;
		int value = 0;
		while (map != NULL) {
			if (strcmp(map->key, "id") == 0) {
				key = atoi(map->value);
			} else if (strcmp(map->key, "quantity") == 0) {
				value = atoi(map->value);
			}
			map = map->next;
		}
		modifyQuantity(key, value, database);

		printf("[#%ld]Sending response\n\n", pthread_self());
		char *document = createHTML(readFile("pages/datatable.html"), database);
		sendResponse(clientSocket, document);
		free(document);
		return;
	} else if (strcmp(request->method, "POST") == 0 && strcmp(request->path, "/delete") == 0) {
		// add new item to database
		MAP *map = parse(request->body);
		int key = 0;

		if (map != NULL && strcmp(map->key, "id") == 0) {
			key = atoi(map->value);
		} else {
			printf("%s-%s", map->key, map->value);
		}

		if (delete (key, database) != 0) {
			printf("Error deleting item with id %d\n", key);
		} else {
			printf("Deleted item with id %d\n", key);
		}

		printf("[#%ld]Sending response\n\n", pthread_self());
		char *document = createHTML(readFile("pages/datatable.html"), database);
		sendResponse(clientSocket, document);
		free(document);
		return;
	} else if (strcmp(request->path, "/stopServer") == 0) {
		STOPSERVER = 1;
		printf("[#%ld]Sending response\n\n", pthread_self());
		char *document = readFile("pages/stopServer.html");
		sendResponse(clientSocket, document);
		free(document);
		return;
	} else {
		// return 404
		printf("[#%ld]Sending response\n\n", pthread_self());
		char *document = readFile("pages/error404.html");
		sendNotFound(clientSocket, document);
		free(document);
		return;
	}
}

int main(void) {
	database = createDatabase();
	char *data = readFile("database.txt");
	importDatabase(data, database);
	free(data);

	int ListenSocket = startWebServer("3333");
	while (1) {
		acceptClient(ListenSocket, 2000, callback);
		if (STOPSERVER)
			break;
	}
	stopWebServer(ListenSocket);

	char *dataFile = exportDatabase(database);
	writeFile("database.txt", dataFile);
	free(dataFile);

	deleteDatabase(database);

	return 0;
}

/*
Important side notes:
- If a browser established a connection, it usally uses the connection:keep-alive header, which means that the browser will not close the connection after the request.
  This means, that the connection stays open, until either the browser or the server make a request either without the connection header or with the connection:close header.
  The connection also closes, if the user closes the browser window.
  So don't worry if a thread stays open even after the browser window has loaded.
*/