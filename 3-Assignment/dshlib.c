#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    // Split commands joined by pipes to separate commands
    memset(clist, 0, sizeof(command_list_t));
    char *token = strtok(cmd_line, PIPE_STRING);
    int cmd_count = 0;

    // Loop through and identify arguments
    while(token != NULL) {
        if(cmd_count >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }

        //Remove leading white spaces
        while (*token == ' ')
        {
            token++;
        }

        //Remove trailing white spaces
        char* end = token + strlen(token) - 1;
        while (end > token && *end == ' ')
        {
            end--;
        }
        *(end + 1) = '\0';
        
        // Split commands and args
        char *command_end = strchr(token, ' ');
        if (command_end != NULL) {
            *command_end = '\0';
            command_end++;
        }
        char *args = (command_end != NULL) ? command_end : "";

        // Check if args exceed maximum arg size
        if(strlen(args) > ARG_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Create a new command object with executable and argumeents
        command_t parsed_command;
        strcpy(parsed_command.exe, token);
        strcpy(parsed_command.args, args);

        // Add command object to command list
        clist->commands[cmd_count] = parsed_command;
        cmd_count++;
        token = strtok(NULL, PIPE_STRING);
    }
    // Add command count to command list 
    clist->num = cmd_count;
    return OK;
}