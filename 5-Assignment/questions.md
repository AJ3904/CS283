1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My implementation ensures that all child processes complete before the shell continues accepting user input by using waitpid() inside the execute_pipeline() function, after forking all child processes and setting up pipes, the parent process waits for each child process to terminate using:
for (int i = 0; i < clist->num; i++) {
    int status;
    waitpid(pids[i], NULL, 0);
}
Forgetting to call waitpid() on all child processes can cause the following problems:
If many commands are executed without waitpid(), the system may eventually run out of available process slots, causing failures when trying to create new processes.
The shell might start accepting new user input while some child processes are still running and exhibhit unpredictable behavior

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

We need to close unused pipes after dup2() because leaving unused pipes open could cause shell to eventually run out of file descriptors, preventing new pipes, sockets, or files from being opened. Moreover, it can cause a process to hang if it is reading from a pipe because if the corresponding pipe is never closed, the process will hang indefinitely, waiting for input that will never come.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

cd is implemented as a built-in command because it modifies the shell's current working directory and the change must persist after the command executes. Using a fork and exec would mean that the directory change would happen in the temporary child process which doesn't affect the directory the actual shell is running in. Using shell commands as workarounds are inefficient so it is better to implement it as a built-in.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

We can modify the implementation to allow an arbitary number of piped commands using dynamic memory allocation i.e. dynamically resizing the array to store command strings and using malloc of that size to create an array to store the file descriptors. Some complexities with this involve malloc/realloc overhead, system limit on the number of pipes, proper error handling and harder memory management.
