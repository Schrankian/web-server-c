/*----------------------------------------------------------------------------------------------------------------------------*
 * File: filemanager.h
 *
 * Description:
 * This file contains all function prototypes for the filemanager.c file.
 *
 * Author: Fabian Schuster
 * Created: 27.01.2023
 *
 * Copyright: 2023 Fabian Schuster
 * License: NONE
 *----------------------------------------------------------------------------------------------------------------------------*/

// include guard
#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#endif

// reads any file and returns the content as a string
char *readFile(char *fileName);

// writes a string to a file
void writeFile(char *fileName, char *string);