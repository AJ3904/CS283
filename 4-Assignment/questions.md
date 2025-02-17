1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  We use fork/execvp instead of just calling execvp to ensure that any errors caused by the child process do not affect the parent process i.e. the shell program itself. If we had used just execvp, any error raised by the commands we provide to the drexel shell would terminate the shell itself. The fork prevents this by creating a child process which is an exact copy of the parent process which ensures the parent process maintains control over its own execution while the child process can execute the provided shell command.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If the fork system call fails, my implementation stores the command execution error code to the rc and doesn't make the execvp call.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  It locates the executable command to run using a combination of the path provided to it and the environment variable PATH.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  Calling wait() in the parent process after forking ensures that the parent process waits for the child process to finish executing and collects the result from the child process i.e. the return code after the provided command was executed. Not using wait() might end up causing unpredictable behaviour as parent process would keep executing without waiting for the child process.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS() provides the exit status of the child process that terminated. This is important because in the drexel shell, it allows to implement error handling and gives the parent process control over the child process to decide the next best course of action, such as retrying the failed operation or logging an error.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  My implementation uses a pointer to parse through the arguments string and checks for a starting quote, if a starting quote was found, the pointer is incremented till a matching closing quote is found. The pointer is incremented by one and the null termination ('\0') is added to the arguments string to extract the quoted string out.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  The main change I had to make to my parsing logic was to store arguments as a list of strings. This required me to implement a function which parses through the string using a pointer removing leading and trailing white spaces, extracting each argument from the overall string and store them to a list. There were no unexpected challenges in refactoring my code and it was easier than I initially thought it would be.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals in a Linux system are asynchronous notifications sent to processes to notify them of events such as hardware exceptions, user commands or system conditions. They enable processes to react to these events immediately. They differ from other forms of IPC such that these are asynchronous and can be used to interrupt a process at any point during the process's runtime.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGTSTP: Stop typed at terminal (Ctrl + Z), primarily used to stop a process without killing it.
                   SIGKILL: Kill a process, primarily used to kill unresponsive or misbehaving processes
                   SIGSEGV: Segmentation fault, primarily used to detect memory access violation in programs.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  No, it cannot be caught or ignored like SIGINT. SIGSTOP is designed to forcefully a stop a process without allowing it to perform any cleanup actions within the signal handler. Only the kernel is allowed to handle the signals SIGSTOP and SIGKILL which makes it impossible for us to catch or ignore it.