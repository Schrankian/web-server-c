/*----------------------------------------------------------------------------------------------------------------------------*
 * File: database.c
 *
 * Description:
 * This file contains all functions for a simple database.
 *
 * Author: Fabian Schuster
 * Created: 27.01.2023
 *
 * Copyright: 2023 Fabian Schuster
 * License: NONE
 *----------------------------------------------------------------------------------------------------------------------------*/

#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// create a new database and return a pointer to it
Item *createDatabase() {
	Item *database = (Item *)malloc(sizeof(Item));
	database->key = 0;
	database->name = NULL;
	database->quantity = 0;
	database->next = NULL;
	return database;
}

// delete the whole database
void deleteDatabase(Item *database) {
	while (database != NULL) {
		Item *temp = database;
		database = database->next;
		free(temp);
	}
}

// insert a new item into the database
void insert(int key, char *name, int quantity, Item *database) {
	Item *current = database;
	while (current->next != NULL) {
		// Compare all names in the database with the new name (alphabetically) and insert the new item at the right position
		if (strcmp(name, (current->next)->name) <= 0) {
			break;
		} else {
			current = current->next;
		}
	}
	Item *newItem = (Item *)malloc(sizeof(Item));
	newItem->key = key;
	newItem->name = name;
	newItem->quantity = quantity;
	newItem->next = current->next;
	current->next = newItem;
}

// modify the quantity of an item
int modifyQuantity(int key, int quantity, Item *database) {
	Item *current = database;

	while (current != NULL) {
		if (current->key == key) {
			current->quantity = quantity;
			return 0;
		}
		current = current->next;
	}
	return -1;
}

// delete an item from the database
int delete(int key, Item *database) {
	Item *current = database->next;
	Item *prev = database;

	while (current != NULL) {
		if (current->key == key) {
			prev->next = current->next;
			free(current);
			return 0;
		}
		prev = current;
		current = current->next;
	}
	return -1;
}

char *createHTML(char *document, Item *database) {
	// allocate memory for the html document and set the maximum size to 10000 characters
	char *html = (char *)malloc(10000 * sizeof(char) + sizeof(*document));
	memset(html, 0, 10000 + sizeof(*document) / sizeof(char));
	html[0] = '\0';  // Add a null terminator to the string

	char *token = strtok(document, "§");  // split the document into tokens and asign everything before § to variable token

	strcat(html, token);

	// add html for each item in the database
	Item *current = database->next;
	while (current != NULL) {
		char *key = (char *)malloc(50 * sizeof(char));
		sprintf(key, "%d", current->key);
		char *quantity = (char *)malloc(50 * sizeof(char));
		sprintf(quantity, "%d", current->quantity);

		strcat(html, "<tr>");
		strcat(html, "<td>");
		strcat(html, current->name);
		strcat(html, "</td>");
		strcat(html, "<td>");
		strcat(html, key);
		strcat(html, "</td>");
		strcat(html, "<td>");
		strcat(html, quantity);
		strcat(html, "</td>");
		strcat(html, "<td>");
		strcat(html, "<form action=\"/delete\" method=\"post\">");
		strcat(html, "<input type=\"hidden\" id=\"id\" name=\"id\" value=\"");
		strcat(html, key);
		strcat(html, "\">");
		strcat(html, "<button id=\"delArticle\">Loeschen</button>");
		strcat(html, "</form>");
		strcat(html, "</td>");
		strcat(html, "</tr>");

		free(key);
		free(quantity);
		current = current->next;
	}

	token = strtok(NULL, "§");  // access the parte after §
	strcat(html, token);
	free(document);
	return html;
}

void importDatabase(char *string, Item *database) {
	char *str = strdup(string);  // make a copy of the input string so that strtok_r() can't change the \n to \0
	char *line = strtok_r(str, "\n", &str);
	while (line != NULL) {
		char *key = strtok_r(line, ",", &line);
		char *quantity = strtok_r(NULL, ",", &line);
		char *name = strtok_r(NULL, ",", &line);
		// insert the item into the database
		insert(atoi(key), name, atoi(quantity), database);
		line = strtok_r(NULL, "\n", &str);
	}
}
char *exportDatabase(Item *database) {
	Item *current = database->next;
	char *dataFile = NULL;
	char line[1000];  // assume maximum line length of 1000 characters
	while (current != NULL) {
		sprintf(line, "%d,%d,%s\n", current->key, current->quantity, current->name);
		if (dataFile == NULL) {
			dataFile = strdup(line);  // create a new string if this is the first line
		} else {
			dataFile = realloc(dataFile, strlen(dataFile) + strlen(line) + 1);  // append to existing string
			strcat(dataFile, line);
		}
		current = current->next;
	}

	return dataFile;
}