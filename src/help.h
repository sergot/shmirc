/* 
 * File:   help.h
 * Author: Filip Sergot
 *
 * Created on December 22, 2013, 11:49 PM
 */

#ifndef HELP_H
#define	HELP_H

#define IN_OK       0
#define IN_NO_INPUT 1
#define IN_TOO_LONG 2

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

void error(char *);
int getLine (char *, char *, size_t);
char first_char(char *);
void cmd(char *, char *);
int find_space(char *);
void remove_cmd(char *);
void first_word(char *, char *);
void get_msg(char *, char *);

#endif	/* HELP_H */

