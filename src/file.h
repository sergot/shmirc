/* 
 * File:   file.h
 * Author: Filip Sergot
 *
 * Created on December 25, 2013, 2:34 PM
 */

#ifndef FILE_H
#define	FILE_H

#include "settings.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct s {
    int channels;
    int users;
    int messages;
} stats;

void write_stats(char *, stats *);
void read_stats(char *, stats *);

#endif	/* FILE_H */

