/*----------------------------------------------------------------------------------------------------------------------------*
 * File: database.h
 *
 * Description:
 * This file contains all functions prototypes for the database.c file.
 *
 * Author: Fabian Schuster
 * Created: 27.01.2023
 *
 * Copyright: 2023 Fabian Schuster
 * License: NONE
 *----------------------------------------------------------------------------------------------------------------------------*/

// include guard
#ifndef DATABASE_H
#define DATABASE_H
#endif

typedef struct Item {
	int key;       // article number
	int quantity;  // quantity of the item
	char* name;    // name of the item
	struct Item* next;
} Item;

// create a hash table and return a pointer to it
Item* createDatabase();

// clear the whole database (should be called before the program ends to free the memory)
void deleteDatabase(Item* database);

// insert a new item into the database
void insert(int key, char* name, int quantity, Item* database);

// change the quantity of an item. Returns -1 if key was not found
int modifyQuantity(int key, int quantity, Item* database);

// delete an item from the database. Returns -1 if key was not found
int delete(int key, Item* database);

// create a HTML document from the database
char* createHTML(char* document, Item* database);

// create Items for the database from a file
void importDatabase(char* string, Item* database);

// create a string from the database
char* exportDatabase(Item* database);