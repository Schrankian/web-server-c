/*----------------------------------------------------------------------------------------------------------------------------*
 * File: filemanager.c
 *
 * Description:
 * This contains all functions which are needed to write to and from a file.
 *
 * Author: Fabian Schuster
 * Created: 27.01.2023
 *
 * Copyright: 2023 Fabian Schuster
 * License: NONE
 *----------------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *readFile(char *fileName) {
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		printf("Error opening file!\n");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	char *content = (char *)malloc((fileSize + 1) * sizeof(char));
	memset(content, 0, fileSize + 1);
	fread(content, sizeof(char), fileSize, file);
	fclose(file);

	content[fileSize] = '\0';

	return content;
}

void writeFile(char *fileName, char *string) {
	FILE *file = fopen(fileName, "w");
	if (file == NULL) {
		printf("Error opening file!\n");
		return;
	}
	fputs(string, file);
	fclose(file);
}