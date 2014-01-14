/* 
 * File:   user.h
 * Author: Filip Sergot
 *
 * Created on December 25, 2013, 2:34 PM
 */

#ifndef USER_H
#define	USER_H

#include "settings.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct usr {
    pid_t pid;
    char name[MAX_NAME_LEN];
    char channel[MAX_CHAN_LEN];
    struct usr *next;
} user;

user *new_user(pid_t, char *, char *);

int add_user(user *, user *);

user *find_user(pid_t, user *);

pid_t get_user_pid(char *, user *);
char *get_user_name(pid_t, user *);
char *get_user_channel(pid_t, user *);

void print_users(user *);

int count_users_on_channel(char *, user *);


#endif	/* USER_H */

