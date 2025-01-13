#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
int  reverse_string(char *, int, int);
int  word_print(char *, int, int);
void print_word(char *, int);
int find_and_replace_word(char*, int, int, char*, char*);

int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    if (buff == NULL || user_str == NULL || len <= 0) {
        return -2;  // Invalid input arguments
    }

    int length = 0;
    char previous = '\0';
    char* current = user_str;
    char* bufferTracker = buff;

    while(*current != '\0') {
        if (length > len) {
            return -1;
        }
        if ((previous == ' ' || previous == '\t') && (*current == ' ' || *current == '\t')) {
            current++;
        }
        else {
            if (*current == '\t') {
                *bufferTracker = ' ';
            }
            else {
                previous = *current;
                *bufferTracker = *current;
            }
            bufferTracker++;
            length++;
            current++;
        }
    }

    int spaceLeft = len - length;
    while (spaceLeft > 0) {
        *bufferTracker = '.';
        bufferTracker++;
        spaceLeft--;
    }

    return length; //for now just so the code compiles. 
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    int wordCount = 0;
    int inWord = 0;
    //YOU MUST IMPLEMENT
    if(buff == NULL || len == 0) {
        return -1;
    }

    for(int i = 0; i < str_len; i++) {
        if (*(buff + i) == ' ') {
            if(inWord) {
                wordCount++;
                inWord = 0;
            }
        }
        else {
            inWord = 1;
        }
    }

    if(inWord) {
        wordCount++;
    }

    return wordCount;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int reverse_string(char* buff, int len, int str_len){
    if(buff == NULL || len == 0) {
        return -1;
    }

    char* start = buff;
    char* end = buff + str_len - 1;
    
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    return 0;
}

void print_string (char* buff, int str_len) {
    for (int i = 0; i < str_len; i++) {
        putchar(*(buff+i));
    }
    putchar('\n');
}

int word_print (char* buff, int len, int str_len) {
    if(buff == NULL || len == 0) {
        return -1;
    }

    int wordLength = 0;
    int wordCount = 0;
    int inWord = 0;
    char* wordStart = NULL;
    printf("Word Print\n----------\n");

    for(int i = 0; i < str_len; i++)
    {
        if (*(buff + i) == ' ') {
            if(inWord) {
                wordCount++;
                inWord = 0;
                printf("%d. ", wordCount);
                print_word(wordStart, wordLength);
                printf(" (%d)\n", wordLength);
                wordLength = 0;
                wordStart = NULL;
            }
        }
        else {
            wordLength++;
            inWord = 1;
            if(wordStart == NULL)
            {
                wordStart = buff + i;
            }
        }
    }

    if(inWord) {
        wordCount++;
        printf("%d. ", wordCount);
        print_word(wordStart, wordLength);
        printf(" (%d)\n", wordLength);
    }

    return 0;
}

int find_and_replace_word(char* buff, int len, int str_len, char* target, char* replacement) {
    if(buff == NULL || len == 0) {
        return -1;
    }
    
    // Find length of target word
    int target_length = 0;
    char* target_ptr = target;
    while (*target_ptr != '\0') {
        target_length++;
        target_ptr++;
    }

    // Find length of replacement word
    int replacement_length = 0;
    char* replacement_ptr = replacement;
    while (*replacement_ptr != '\0') {
        replacement_length++;
        replacement_ptr++;
    }

    int new_string_length = str_len - target_length + replacement_length;
    if (new_string_length > len) {
        return -2; // Addition of replacement string causes buffer overflow
    }

    target_ptr = target;
    char* start = NULL;

    for (int i = 0; i < str_len; i++) {
        if(*target_ptr == '\0' && ((*(buff + i) == ' ') || (*(buff + i) == '.'))) {
            start = buff + i - target_length;
            break;
        }
        else if(*target_ptr == *(buff + i)) {
            target_ptr++;
        }
        else {
            target_ptr = target;
        }
    }

    if(start == NULL) {
        return -3; // No match found
    }
    
    target_ptr = target;
    char* temp = malloc(new_string_length * sizeof(char));
    if(temp == NULL) {
        return -4; // Temporary buffer creation failed
    }

    char* temp_ptr = temp;
    // Copy till beginning of target word
    for(char* i = buff; i < start; i++) {
        *temp_ptr = *i;
        temp_ptr++;
    }

    // Copy the replacement word
    for(int i = 0; i < replacement_length; i++) {
        *temp_ptr = *(replacement + i);
        temp_ptr++;
    }

    //Copy the rest of the buffer
    for(char* i = start + target_length; i < buff + str_len; i++) {
        *temp_ptr = *i;
        temp_ptr++;
    }

    //Copy from temp to main buffer
    for(int i = 0; i < new_string_length; i++) {
        *(buff + i) = *(temp + i);
    }

    temp_ptr = NULL;
    free(temp);

    printf("Modified String: ");
    print_string(buff, new_string_length);
    return 0;
}

void print_word(char* start, int end) {
    for(int i = 0; i < end; i++) {
        putchar(*(start + i));
    }
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      This is safe because a short circuit evaluation is used for or. If argv[1] does not exist
    //      the case argc < 2 will evaluate to true. Because the first case is true, the second case is not evaluated. 
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    // The purpose of this if statement is to check whether both the operation type 
    // and the string to be operated on are provided as command-line arguments.
    // If the number of arguments is less than 3, it calls the usage() method to display the correct
    // usage instructions for the program and then exits with an error code of 1.
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = malloc(BUFFER_SZ * sizeof(char));
    if(buff == NULL)
    {
        return 99;
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;
        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            rc = reverse_string(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error reversing string, rc = %d", rc);
                exit(2);
            }
            printf("Reversed String: ");
            print_string(buff, user_str_len);
            break;
        case 'w':
            rc = word_print(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error printing words, rc = %d", rc);
                exit(2);
            }
            break;
        case 'x':
            if(argc < 5){
                usage(argv[0]);
                exit(1);
            }
            rc = find_and_replace_word(buff, BUFFER_SZ, user_str_len, argv[3], argv[4]);
            if (rc < 0){
                printf("Error finding and replacing word, rc = %d", rc);
                exit(2);
            }
            break;
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          Passing the buffer size, makes the code easy to reuse with different buffer sizes in the future while
//          also ensuring that functions can validate operations against the specified length ensuring no out
//          of bounds memory is accessed. Moreover, it makes it easier to debug and validate operations within the specified bounds.
