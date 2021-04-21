#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <ctype.h>


/**
 * @brief treats user argument input errors
 * 
 */
void input_error(){
    fprintf(stderr,"Usage: ./s <-t nsecs> [-l bufsize] fifoname\n");
    exit(1);
}

/**
 * @brief compares two strings (wrapper function for strcmp)
 * 
 * @param string1 first string to be compare with
 * @param string2 second string to compare with the first
 * @return true if strings are the same, false otherwise
 */
bool str_cmp(char *string1, char *string2){
    if(strcmp(string1, string2) == 0) return true;
    else return false;
}

/**
 * @brief checks if a string represents a valid number
 * 
 * @param str the string to be checked
 * @return true if it a number, false otherwise
 */
bool is_number(char *str){
     for (int i = 0; str[i] != '\0'; i++)
        if (isdigit(str[i]) == false)
            return false;
 
    return true;
}

int main(int argc, char* argv[]){

    //when less than the minimun arguments number is given
    if(argc < 4) input_error();

    if(!str_cmp(argv[1], "-t") || !is_number(argv[2])) input_error();

    return 0;
}
