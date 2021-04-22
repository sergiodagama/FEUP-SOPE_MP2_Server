#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

/*--------------------GLOBAL VARIABLES--------------------*/

char *public_fifo;  //holds the public fifo name/path

bool time_is_up = false;  //to keep track of server execution time

/*------------------END GLOBAL VARIABLES------------------*/


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
    /*
    # ARGV 0 -> ./s         ARGV 3 -> fifoname / -l   #
    # ARGV 1 -> -t          ARGV 4 -> bufsize         #
    # ARGV 2 -> nsecs       ARGV 5 -> fifoname        #
    */

   /*--------------------INPUT ERROR VERIFICATION--------------------*/

    //when less than the minimun arguments number is given
    if(argc < 4) input_error();

    //when -t or time is not passed correctly
    if(!str_cmp(argv[1], "-t") || !is_number(argv[2])) input_error();

    //-l passed but with no arguments
    if(str_cmp(argv[3], "-l") && argc != 6) input_error();

    //-l not passed or bufsize argmuent is not a number
    if(argc == 6 && (!str_cmp(argv[3], "-l") || !is_number(argv[4]))) input_error();

    //after this the program assumes inputs are correctly inputted

    /*------------------END INPUT ERROR VERIFICATION------------------*/   

    int nsecs = atoi(argv[2]);
    int bufsize = 1;

    if(str_cmp(argv[3], "-l")){
        bufsize = atoi(argv[4]);
        public_fifo = argv[5];
    }
    else{
        public_fifo = argv[3];
    }

    while(!time_is_up){
    }

    return 0;
}
