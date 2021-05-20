#include "utils.h"


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

void free_memory(queue* trash){
    while(!isEmpty(trash)){
        free(front(trash));
        pop(trash);
    }
}

