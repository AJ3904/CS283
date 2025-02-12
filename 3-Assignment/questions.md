1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  Shell commands are usually entire lines of input, which may contain spaces. fgets() reads the entire line including spaces, which is helpful in reading the provided command with its arguments. Other options like scanf or getchar may require more complex handling to get the full command.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  By using malloc(), we are dynamically allocating memory from the heap at runtime. This provides us with flexibility in how much memory can be allocated, especially when we want to adjust the buffer sizes in future versions of the cell. Moreover, dynamic memory allocation is important for avoiding memory leaks, especially in long-running programs like a shell that may allocate memory repeatedly.

3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  Overwrite redirection (<, >) Ex. ls > file1.txt 
                   Append redirection (<<, >>) Ex. echo "Hello" >> fiel1.txt
                   Merge redirection (<&, >&) Ex. ls non_existent_file > output.txt 2>&1
    The challenges we might face in implementing these is proper handling of file descriptors, ensuring error handling for failure of file I/O operations and ensuring their effects don't remain after the execution of the command involving redirection.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection allows us to control where the input or output of a single command goes, usually to or from a file, while piping sends the output of one command as the input to another command, making chaining multiple commands together to perform a series of operations possible.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  Combining them would make it difficult for users to sort between useful output and errors. Moreover, not keeping them separate would cause issues in pipelining. The output sent to STDOUT by the previous command is sent as input to the following command in pipelining. Not separating STDERR and STDOUT would result in the errors of previous commands being sent as input to the following commands if the previous command raises an error.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  If a command fails, our custom shell should only print the error message without mixing it with the normal output. Our shell should make use of the exit code to determine a command's failure. Yes, we should provide a way to merge them. We can implement merge redirection to combine output from both STDOUT and STDERR.