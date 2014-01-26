/* 
 * File:   settings.h
 * Author: Filip Sergot
 *
 * Created on December 22, 2013, 11:50 PM
 */

#ifndef SETTINGS_H
#define	SETTINGS_H

#define SHM_PATH "/shmirc"

#define SEM_PATH "/semirc"
#define SEM_VALUE 0

#define MAX_NAME_LEN 40
#define MAX_CHAN_LEN 40

// max line length
#define MAX_LINE_LEN 100

// how many numbers, not how many items - for 100 channels write 3, for 1000 write 4 etc....
#define MAX_MSG_COUNT 10000
#define MAX_USERS_COUNT 100
#define MAX_CHANNELS_COUNT 100

#endif	/* SETTINGS_H */

