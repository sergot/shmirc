#include <string.h>

#include "user.h"

user *new_user(pid_t pid, char *name, char *channel) {
    user *u = malloc(sizeof(user));
    
    u->pid = pid;
    strncpy(u->name, name, MAX_NAME_LEN);
    strncpy(u->channel, channel, MAX_CHAN_LEN);
    u->next = NULL;
    
    return u;
}

int add_user(user *new_user, user *first_user) {
    int i = 1;
    
    user *u = first_user;
    while(u->next != NULL) {
        i++;
        u = u->next;
    }

    u->next = new_user;
    
    return i;
}

user *find_user(pid_t pid, user *first_user) {
    user *u = first_user;
    
    while(u != NULL) {
        if(u->pid == pid)
            return u;
        
        u = u->next;
    }
    
    return NULL;
}

pid_t get_user_pid(char *name, user *first_user) {
    user *u = first_user;
    
    while(u != NULL) {
        if(strncmp(name, u->name, MAX_NAME_LEN) == 0)
            return u->pid;
        
        u = u->next;
    }
    
    return -1;
}
char *get_user_name(pid_t pid, user *first_user) {
    user *u = first_user;
    
    while(u != NULL) {
        if(u->pid == pid)
            return u->name;
        
        u = u->next;
    }
    
    return NULL;
}
char *get_user_channel(pid_t pid, user *first_user) {
    user *u = first_user;
    
    while(u != NULL) {
        if(u->pid == pid)
            return u->channel;
        
        u = u->next;
    }
    
    return NULL;
}

void print_users(user *first_user) {
    user *u = first_user;
    
    while(u != NULL) {
        printf("[%d] %s in %s\n", u->pid, u->name, u->channel);
        
        u = u->next;
    }
}

int count_users_on_channel(char *channel, user *first_user) {
    user *u = first_user;
    int i = 0;

    while(u != NULL) {
        if(strncmp(u->channel, channel, strlen(channel)) == 0)
            i++;
        u = u->next;
    }

    return i;
}
