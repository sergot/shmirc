#include "help.h"
#include "msg.h"

void error(char *s) {
    perror(s);
    
    exit(EXIT_FAILURE);
}

char first_char(char *s) {
    return *s;
}

int find_space(char *s) {
    int i;
    for(i = 0; i < strlen(s); i++)
        if(s[i] == ' ')
            break;
    
    if(i == strlen(s))
        return -1;
    
    return i;
}

void remove_cmd(char *s) {
    int i = find_space(s);
    
    memmove(s, s + i, strlen(s));
}

void cmd(char *s, char *cmd) {
    strncpy(cmd, s + 1, find_space(s));
}

int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return IN_NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? IN_TOO_LONG : IN_OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    
    return IN_OK;
}