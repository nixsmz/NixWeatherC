/*
    File        : json.h
    Author      : nix.dev
    Date        : 16.03.23
    Version     : 2.0.0
    Description : JSON file reader API Header file
*/
#ifndef _JSON_H_
#define _JSON_H_

// Librairies
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Equivalences
#define JSON_BUFSIZE    64

#define JSON_INDEX_CHR  '%'

// Fonctions prototypes
char* json_search(char *buf, char *path, char *json);

#endif
