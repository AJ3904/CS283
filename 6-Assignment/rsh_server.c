
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>

//INCLUDES for extra credit
#include <signal.h>
//#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"
#include "dragon.h"

int server_socket = -1;
void handle_signal(int sig) {
    printf("Received signal %d, shutting down server...\n", sig);
    if (server_socket != -1) {
        printf("Closing server socket %d\n", server_socket);
        close(server_socket);
    }
    printf("Server shutdown complete\n");
    fflush(stdout);
    exit(0);  // Exit cleanly
}

int handle_error(int svr_socket, const char *msg) {
    perror(msg);
    if (svr_socket >= 0) close(svr_socket);
    return ERR_RDSH_COMMUNICATION;
}

int handle_cli_error(int svr_socket, int cli_socket, const char *msg) {
    perror(msg);
    if (cli_socket >= 0) close(cli_socket);
    if (svr_socket >= 0) close(svr_socket);
    return ERR_RDSH_COMMUNICATION;
}

// Server initialization function
void initialize_server() {
    // Uncomment the signal.h include at the top of your file!
    
    // Setup signal handling
    signal(SIGINT, handle_signal);   // Handle Ctrl+C
    signal(SIGTERM, handle_signal);  // Handle kill command
    
    printf("Signal handlers registered for server\n");
    fflush(stdout);
}
/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded){
    int rc;

    // Do nothing with is_threaded
    rc = is_threaded == 0 ? 0 : 1;

    // Initialize signal handlers
    initialize_server();
    
    // Debug output with PID
    printf("START_SERVER PID: %d\n", getpid());
    fflush(stdout);

    // Boot server and return error code if any
    server_socket = boot_server(ifaces, port);
    if (server_socket < 0) {
        return server_socket;
    }
    
    rc = process_cli_requests(server_socket);

    // Stop Server
    stop_server(server_socket);
    
    // Reset global server socket variable
    server_socket = -1;

    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port) {
    int svr_socket = -1;
    int ret;
    struct sockaddr_in addr;

    // Create socket
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) return handle_error(svr_socket, "socket creation failed");

    // Configure server address
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if (strcmp(ifaces, "0.0.0.0") == 0) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else if (inet_pton(AF_INET, ifaces, &addr.sin_addr) <= 0) {
        return handle_error(svr_socket, "Invalid address");
    }

    // Bind socket to address and port
    if (bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return handle_error(svr_socket, "bind failed");
    }

    ret = listen(svr_socket, 20);
    if (ret == -1) return handle_error(svr_socket, "listen failed");

    return svr_socket;
}


/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int svr_socket) {
    int cli_socket;
    int rc = OK;    
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (1) {
        // Accept new client connection
        cli_socket = accept(svr_socket, (struct sockaddr *)&client_addr, &client_addr_len);

        if (cli_socket < 0) {
            return handle_cli_error(svr_socket, cli_socket, "Accept failed");
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

        // Process client requests
        rc = exec_client_requests(cli_socket);

        // Close client socket
        close(cli_socket);

        if (rc < 0 || rc == EXIT_SC) {
            close(svr_socket);
            break;
        }
    }

    return rc;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int cli_socket) {
    int io_size;
    command_list_t cmd_list;
    int rc;
    char *io_buff;
    char *command_buffer = NULL;
    size_t command_buffer_size = 0;
    int command_complete = 0;
    int result = OK;

    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL) {
        return ERR_RDSH_SERVER;
    }

    while(1) {
        // Reset variables for new command
        command_complete = 0;
        if (command_buffer != NULL) {
            free(command_buffer);
            command_buffer = NULL;
            command_buffer_size = 0;
        }
        
        // Read command chunks until null terminator is found
        while (!command_complete) {
            memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
            io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ - 1, 0);
            
            if (io_size <= 0) {
                if (io_size == 0) {
                    printf("Client disconnected\n");
                } else {
                    perror("recv failed");
                }
                free(io_buff);
                free(command_buffer);
                return OK;
            }
            
            // Check if this chunk contains the null terminator
            char *null_terminator = memchr(io_buff, '\0', io_size);
            if (null_terminator != NULL) {
                io_size = null_terminator - io_buff;
                command_complete = 1;
            }
            
            // Append this chunk to the command buffer
            size_t new_size = command_buffer_size + io_size;
            char *new_buffer = realloc(command_buffer, new_size + 1);

            if (new_buffer == NULL) {
                free(io_buff);
                free(command_buffer);
                return ERR_MEMORY;
            }
            
            command_buffer = new_buffer;
            memcpy(command_buffer + command_buffer_size, io_buff, io_size);
            command_buffer_size = new_size;
            command_buffer[command_buffer_size] = '\0';
            
            if (command_complete) {
                break;
            }
        }
        
        // Build command list from input
        rc = build_cmd_list(command_buffer, &cmd_list);
        
        if (rc < 0) {
            send_message_string(cli_socket, "Error parsing commands");
            continue;
        }
        
        // Check for built-in commands
        if (cmd_list.num > 0) {
            Built_In_Cmds cmd_type = rsh_match_command(cmd_list.commands[0].argv[0]);
            
            switch (cmd_type) {
                // Custom exit
                case BI_CMD_EXIT:
                    free(io_buff);
                    free(command_buffer);
                    return OK;
                
                // Stop-server
                case BI_CMD_STOP_SVR:
                    // Stop-server command - shutdown server
                    free(io_buff);
                    free(command_buffer);
                    return EXIT_SC;
                
                // Custom cd
                case BI_CMD_CD:
                    if (cmd_list.commands[0].argc == 2) {
                        if (chdir(cmd_list.commands[0].argv[1]) == 0) {
                            send_message_eof(cli_socket);
                        }
                    }
                    continue;
                
                // Dragon
                case BI_CMD_DRAGON:
                    send_message_string(cli_socket, dragon);
                    continue;
                    
                // rc command
                case BI_CMD_RC:
                    send_message_string(cli_socket, BI_NOT_IMPLEMENTED);
                    continue;
                    
                default:
                    break;
            }
        }
        
        // Execute command pipeline
        result = rsh_execute_pipeline(cli_socket, &cmd_list);
        
        // Send EOF to indicate command completion
        send_message_eof(cli_socket);
    }
    
    free(io_buff);
    free(command_buffer);
    
    return result;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket){
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len){
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}


/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff){
    int bytes_sent = 0;
    
    // Send message if provided
    if (buff != NULL && strlen(buff) > 0) {
        bytes_sent = send(cli_socket, buff, strlen(buff), 0);
        if (bytes_sent < 0) {
            return ERR_RDSH_COMMUNICATION;
        }
    }
    
    // Send EOF character
    return send_message_eof(cli_socket);
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];  // Array of pipes
    pid_t pids[clist->num];
    int pids_st[clist->num];         // Array to store process status
    int rc;

    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ESPIPE;
        }
    }

    // Create processes for each command
    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            return ERR_MEMORY;
        } 
        else if (pids[i] == 0) {
            // First process in pipeline
            if (i == 0) {
                // Close read end of first pipe
                if (clist->num > 1) {
                    close(pipes[0][0]);
                }
                
                // Redirect stdin from client socket and stdout to client/next pipe
                dup2(cli_sock, STDIN_FILENO);
                if (clist->num > 1) {
                    dup2(pipes[0][1], STDOUT_FILENO);
                } else {
                    dup2(cli_sock, STDOUT_FILENO);
                }
                dup2(cli_sock, STDERR_FILENO);
                
                // Close all other pipe fds
                for (int j = 1; j < clist->num - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                if (clist->num > 1) {
                    close(pipes[0][1]);
                }
            }
            // Last process in pipeline
            else if (i == clist->num - 1) {
                // Close write end of last pipe
                close(pipes[i-1][1]);
                
                // Redirect stdin from pipe and stdout/stderr to client socket
                dup2(pipes[i-1][0], STDIN_FILENO);
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
                
                // Close all other pipe fds
                for (int j = 0; j < clist->num - 2; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                close(pipes[i-1][0]);
            }
            // Middle processes in pipeline
            else {
                // Close read end of current pipe and write end of previous pipe
                close(pipes[i][0]);
                close(pipes[i-1][1]);
                
                // Redirect stdin from previous pipe and stdout to current pipe
                dup2(pipes[i-1][0], STDIN_FILENO);
                dup2(pipes[i][1], STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
                
                // Close all other pipe fds
                for (int j = 0; j < clist->num - 1; j++) {
                    if (j != i && j != i-1) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                }
                close(pipes[i-1][0]);  // Close after dup2
                close(pipes[i][1]);    // Close after dup2
            }
            
            // Execute command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            
            // If execvp returns, it must have failed
            fprintf(stderr, "%s: command not found\n", clist->commands[i].argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Parent process: close all pipe ends
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    //by default get exit code of last process
    //use this as the return value
    rc = WEXITSTATUS(pids_st[clist->num - 1]);
    for (int i = 0; i < clist->num; i++) {
        //if any commands in the pipeline are EXIT_SC
        //return that to enable the caller to react
        if (WEXITSTATUS(pids_st[i]) == EXIT_SC) {
            rc = EXIT_SC;
        }
    }
    return rc;
}

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}