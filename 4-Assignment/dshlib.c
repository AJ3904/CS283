#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dragon.h"
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

void parse_argument(char* args, char* cmd[], int* rc, int* arg_count)
{
    int count = 1;
    char* start = args;

    // Remove leading spaces
    while(*start == ' ') {
        start++;
    }

    // Loop through the arguments
    while(*start) {

        // Too many arguments provided
        if(count > CMD_ARGV_MAX) {
            *rc = ERR_CMD_ARGS_BAD;
            break;
        }
 
        char* arg_start = start;

        // Check if quoted string and remove quotes
        if(*start == '"' || *start == '\'') {
            char quote = *start;
            start++;
            arg_start++;

            while(*start && *start != quote) {
                start++;
            }

            if (*start == quote) {
                *start = '\0';
                start++;
            }
        }

        else {
            // Handle other arguments
            while(*start && *start != ' ' && *start != '\0') {
                start++;
            }
        }

        // Null terminate after successfully reading an argument
        if (*start) {
            *start = '\0';
            start++;
        }

        // Store extracted argument to list
        cmd[count++] = arg_start;
    }

    cmd[count] = NULL;
    *arg_count = count;
}

int exec_local_cmd_loop()
{
    char *cmd_buff = malloc(ARG_MAX);
    int rc = 0;
    char* cmd[CMD_ARGV_MAX];
    int arg_count = 1;

    // Main loop
    while(1) {
        printf("%s", SH_PROMPT);
        if(fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';

        // No command
        if(strlen(cmd_buff) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }
    
    // Break pipes into individual commands
    char *token = strtok(cmd_buff, PIPE_STRING);
    int cmd_count = 0;
    while(token != NULL) {
        if(cmd_count >= CMD_MAX) {
            rc = ERR_TOO_MANY_COMMANDS;
            break;
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
            rc = ERR_CMD_OR_ARGS_TOO_BIG;
            break;
        }

        // Store command and args to list
        cmd[0] = token;
        parse_argument(args, cmd, &rc, &arg_count);

        // Custom exit
        if(strcmp(cmd[0], EXIT_CMD) == 0)
        {
            exit(0);
        }
        // Dragon
        else if(strcmp(cmd[0], "dragon") == 0)
        {
            print_dragon();
        }
        // Custom cd
        else if((strcmp(cmd[0], "cd") == 0))
        {
            if(arg_count == 2) {
                chdir(cmd[1]);
            }
        }
        // Fork and exec other commands
        else
        {
            pid_t p = fork();
            if(p < 0) {
                rc = ERR_EXEC_CMD;
            }
            else if (p > 0) {
                wait(NULL);
            }
            else {
                execvp(cmd[0], cmd);
                int status;
                waitpid(p, &status, 0);
                rc = WEXITSTATUS(status);
            }
        }

        cmd_count++;
        token = strtok(NULL, PIPE_STRING);
    }
    }
    return OK;
}
