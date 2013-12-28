/* 
 * File:   msg.h
 * Author: user
 *
 * Created on December 22, 2013, 11:31 PM
 */

#ifndef MSG_H
#define	MSG_H

#define MAX_MSG_LENGTH 128
#define MAX_CMD_LENGTH   8

#define TYPE_CLIENT_MSG 0
#define TYPE_SERVER_MSG 1

#include <sys/types.h>
#include "settings.h"

struct msg {
    int type;
    char read;
    pid_t pid;
    char channel[MAX_CHAN_LEN];
    char cmd[MAX_CMD_LENGTH];
    char content[MAX_MSG_LENGTH];
};

#endif	/* MSG_H */

