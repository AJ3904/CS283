#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"
#include "dragon.h"

/**** 
 **** FOR REMOTE SHELL USE YOUR SOLUTION FROM SHELL PART 3 HERE
 **** THE MAIN FUNCTION CALLS THIS ONE AS ITS ENTRY POINT TO
 **** EXECUTE THE SHELL LOCALLY
 ****
 */

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

int parse_argument(char* args, cmd_buff_t* command)
{
    char* start = args;

    // Remove leading spaces
    while(*start == ' ') {
        start++;
    }

    // Loop through the arguments
    while(*start) {

        // Too many arguments provided
        if(command->argc >= CMD_ARGV_MAX) {
            return ERR_CMD_ARGS_BAD;
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
        command->argv[command->argc++] = arg_start;
    }

    command->argv[command->argc] = NULL;
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *token = strtok(cmd_line, PIPE_STRING);
    clist->num = 0;

    while(token != NULL) {
        if(clist->num >= CMD_MAX) {
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

        //Create cmd_buff_t and allocate values to it
        cmd_buff_t* command = malloc(sizeof(cmd_buff_t));
        if(command == NULL) {
            return ERR_MEMORY;
        }
        command->_cmd_buffer = token;

        // Split commands and args
        char *command_end = strchr(token, ' ');
        if (command_end != NULL) {
            *command_end = '\0';
            command_end++;
        }
        char *args = (command_end != NULL) ? command_end : "";

        // Store command and args to cmd_buff_t
        command->argv[0] = token;
        command->argc = 1;
        int rc = parse_argument(args, command);
        if(rc != OK) {
            return rc;
        }

        //Add command to list and increment count
        clist->commands[clist->num++] = *command;
        token = strtok(NULL, PIPE_STRING);
    }

    return OK;
}

int exec_command(cmd_buff_t *command) {
    pid_t pid = fork();
    if (pid < 0) {
        return ERR_EXEC_CMD;
    }

    if (pid == 0) {
        execvp(command->argv[0], command->argv);
        exit(errno);
    }
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

int execute_pipeline(command_list_t *clist) {

    int pipes[CMD_MAX - 1][2];
    pid_t pids[CMD_MAX]; 

    // Create pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            return ERR_MEMORY;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            return ERR_MEMORY;
        }
        
        if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            exit(errno);
        }
    }

    // Close all pipes in parent process
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes to complete
    for (int i = 0; i < clist->num; i++) {
        int status;
        waitpid(pids[i], &status, 0);

        if (WIFEXITED(status)) {
            int child_exit_status = WEXITSTATUS(status);
            if (child_exit_status != OK) {
                return child_exit_status;
            }
        }
    }

    return OK;
}

int exec_local_cmd_loop()
{
    char *cmd_buff = malloc(ARG_MAX);
    if(cmd_buff == NULL) {
        return ERR_MEMORY;
    }

    command_list_t* cmd_list = malloc(sizeof(command_list_t));
    if(cmd_list == NULL) {
        return ERR_MEMORY;
    }

    int rc = OK;
    int rc_previous = OK;

    // Main loop
    while(1) {
        rc_previous = rc;
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

        rc = build_cmd_list(cmd_buff, cmd_list);
        if(rc != OK) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            return rc;
        }

        if(cmd_list->num == 1)
        {
            // Custom exit
            if(strcmp(cmd_list->commands[0].argv[0], EXIT_CMD) == 0)
            {
                printf("exiting...\n");
                return rc_previous;
            }
            // Dragon
            else if(strcmp(cmd_list->commands[0].argv[0], "dragon") == 0)
            {
                print_dragon();
            }
            // Custom cd
            else if((strcmp(cmd_list->commands[0].argv[0], "cd") == 0))
            {
                if(cmd_list->commands[0].argc == 2) {
                    chdir(cmd_list->commands[0].argv[1]);
                }
            }
            //Other individual commands
            else
            {
                rc = exec_command(&cmd_list->commands[0]);
                if(rc != OK) {
                    printf(CMD_ERR_EXECUTE, rc);
                }
            }
        }
        else
        {
            rc = execute_pipeline(cmd_list);
            if(rc != OK) {
                printf(CMD_ERR_EXECUTE, rc);
            }
        }
    }

    return rc;
}