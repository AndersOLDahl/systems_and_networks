/*
 * CS3600, Spring 2014
 * C Bootcamp, Homework 3, Problem 1
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to fill in the itoaaa function.
 * This function will take in a 32-bit signed integer, and will
 * return a malloc'ed char * containing the English representation
 * of the number.  A few examples are below:
 *
 * 0 -> "zero"
 * 9 -> "nine"
 * 45 -> "forty five"
 * -130 -> "negative one hundred thirty"
 * 11983 -> "eleven thousand nine hundred eighty three"
 *
 * Do not touch anything outside of the itoaaa function (you may,
 * of course, define any helper functions you wish).  You may also
 * use any of the functions in <string.h>.
 *
 * Finally, you must make sure to free() any intermediate malloced()
 * memory before you return the result.  You should also return a
 * char* that is malloced to be as small as necessary (the script
 * checks for this).  For example, if you returned "forty five", you
 * should put this into a malloced space of 12 bytes (11 + '\0').
 *
 */

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define TRILLION  1000000000000
#define BILLION   1000000000
#define MILLION   1000000

char * ONES_A[10] = { "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
char * TEN_TO_TWENTY_A[10] = { "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen" };
char * TENS_A[10] = { "zero", "ten", "twenty", "thirty", "fourty", "fifty", "sixty", "seventy", "eighty", "ninety" };

char *itoaaa(int i);

char * ones(unsigned int n);
char * ten_to_twenty(unsigned int n);
char * tens(unsigned int n);
char * hundreds(unsigned int n);
char * thousands(unsigned int n);
char * millions(unsigned int n);
char * billions(unsigned int n);

char * allocate_one_string(char * s1);
char * combine_and_allocate_two_strings(char * s1, char * s2);
char * combine_and_allocate_three_strings(char * s1, char * s2, char * s3);

int main(int argc, char **argv) {
    // check for the right number of arguments
    if (argc != 2) {
        printf("Error: Usage: ./cp3-problem1 [int]\n");
        return 0;
    }

    // create the time structure
    long long arg = atoll(argv[1]);
    if (arg == (int) arg) {
        // call the function
        char *result = itoaaa((int) arg);

        // print out the result
        printf("%d is: %s\n", (int) arg, result);

        // CLEAN UP THE RESULT
        free(result);
        result = NULL;

    } else {
        printf("Error: Number out of range.\n");
    }

    return 0;
}

/**
 * This function should print out the English full representation
 * of the passed-in argument.  See above for more details.
 */
char *itoaaa(int arg) {

    char * result = billions(arg);

    if(arg < 0) {
        arg = arg * -1;
        char * temp = billions(arg);
        result = combine_and_allocate_two_strings("negative", temp);
        free(temp);
        return result;
    } else {
        char * result = billions(arg);
        return result;
    }
}

char * allocate_one_string(char * s1) {

    // Accounts for NULL escape sequence
    size_t size = strlen(s1) + 1;
    char * result;
    if ((result = (char *) calloc(size, sizeof(char))) == NULL)  {
        printf("Error: Processing Calloc in Function Combine");
    }

    result = strcpy(result, s1);

    // RETURNING MALLOC'ED VARIABLE
    return result;

}

char * combine_and_allocate_two_strings(char * s1, char * s2) {

    // Accounts for NULL escape sequence and one space
    size_t size = strlen(s1) + strlen(s2) + 2;
    char * result;
    if ((result = (char *) calloc(size, sizeof(char))) == NULL)  {
        printf("Error: Processing Calloc in Function Combine");
    }

    result = strcpy(result, s1);
    result = strcat(result, " ");
    result = strcat(result, s2);

    // RETURNING MALLOC'ED VARIABLE
    return result;
}

char * combine_and_allocate_three_strings(char * s1, char * s2, char * s3) {

    // Accounts for NULL escape sequence and one space
    size_t size = strlen(s1) + strlen(s2) + strlen(s3) + 3;
    char * result;
    if ((result = (char *) calloc(size, sizeof(char))) == NULL)  {
        printf("Error: Processing Calloc in Function Combine");
    }

    result = strcpy(result, s1);
    result = strcat(result, " ");
    result = strcat(result, s2);
    result = strcat(result, " ");
    result = strcat(result, s3);

    // RETURNING MALLOC'ED VARIABLE
    return result;
}

char * billions(unsigned int n) {

    if (n >= TRILLION || n < 0) {
        printf("Error: Entered a value not in the range of 0 - BILLION");
        return NULL;
    }

    if (n >= 0 && n < BILLION) {
        // MALLOC'ED VARIABLE
        return millions(n);
    } else {
        int billions_i = n / BILLION;
        int millions_i = n % BILLION;
        char *s1 = hundreds(billions_i);
        char *s2 = "billion";
        char *result;

        if (millions_i == 0 ) {
            result = combine_and_allocate_two_strings(s1, s2);

            // MALLOC'ED VARIABLE
            return result;
        } else {
            char *s3 = millions(millions_i);
            result = combine_and_allocate_three_strings(s1, s2, s3);
            free(s3);

        }

        free(s1);
        // MALLOC'ED VARIABLE
        return result;
    }
}

char * millions(unsigned int n) {

    if (n >= BILLION || n < 0) {
        printf("Error: Entered a value not in the range of 0 - BILLION");
        return NULL;
    }

    if (n >= 0 && n < MILLION) {
        // MALLOC'ED VARIABLE
        return thousands(n);
    } else {
        int millions_i = n / MILLION;
        int thousands_i = n % MILLION;
        char *s1 = hundreds(millions_i);
        char *s2 = "million";
        char *result;

        if (thousands_i == 0 ) {
            result = combine_and_allocate_two_strings(s1, s2);

            // MALLOC'ED VARIABLE
            return result;
        } else {
            char *s3 = thousands(thousands_i);
            result = combine_and_allocate_three_strings(s1, s2, s3);
            free(s3);

        }

        free(s1);
        // MALLOC'ED VARIABLE
        return result;
    }
}

char * thousands(unsigned int n) {

    if (n >= MILLION || n < 0) {
        printf("Error: Entered a value not in the range of 0 - 1000");
        return NULL;
    }

    if (n >= 0 && n < 1000) {
        // MALLOC'ED VARIABLE
        return hundreds(n);
    } else {
        int thousands_i = n / 1000;
        int hundreds_i = n % 1000;
        char *s1 = hundreds(thousands_i);
        char *s2 = "thousand";
        char *result;

        if (hundreds_i == 0 ) {
            result = combine_and_allocate_two_strings(s1, s2);
        } else {
            char *s3 = hundreds(hundreds_i);
            result = combine_and_allocate_three_strings(s1, s2, s3);
            free(s3);
        }

        free(s1);
        // MALLOC'ED VARIABLE
        return result;
    }
}

char * hundreds(unsigned int n) {

    if (n >= 1000 || n < 0) {
        printf("Error: Entered a value not in the range of 0 - 1000");
        return NULL;
    }

    if (n >= 0 && n < 100) {
        // MALLOC'ED VARIABLE
        return tens(n);
    } else {
        int hundreds_i = n / 100;
        int tens_i = n % 100;
        char *s1 = ones(hundreds_i);
        char *s2 = "hundred";
        char *result;

        if (tens_i == 0 ) {
            result = combine_and_allocate_two_strings(s1, s2);
        } else {
            char *s3 = tens(tens_i);
            result = combine_and_allocate_three_strings(s1, s2, s3);
            free(s3);
        }

        free(s1);
        // MALLOC'ED VARIABLE
        return result;
    }
}

char * tens(unsigned int n) {

    if (n >= 100 || n < 0) {
        printf("Error: Entered a value not in the range of 0 - 100");
        return NULL;
    }

    if (n >= 0 && n < 10) {
        return ones(n);
    } else if (n >= 10 && n < 20) {
        return ten_to_twenty(n);
    } else {
        int ones_i = n % 10;
        char *s1 = TENS_A[n / 10];
        if (ones_i == 0) {
            // MALLOC'ED VARIABLE
            return allocate_one_string(s1);
        } else {
            char *s2 = ones(ones_i);
            char *result = combine_and_allocate_two_strings(s1, s2);
            free(s2);

            // MALLOC'ED VARIABLE
            return result;
        }
    }
}

char * ten_to_twenty(unsigned int n) {

    if (n >= 20 || n < 10) {
        printf("Error: Entered a value not in the range of 10 - 20");
        return NULL;
    }

    // To find the correct location in the array
    n = n % 10;

    // MALLOC'ED VARIABLE
    return allocate_one_string(TEN_TO_TWENTY_A[n]);
}

char * ones(unsigned int n) {

    if (n >= 10 || n < 0) {
        printf("Error: Entered a value not in the range of 0 - 10");
        return NULL;
    }

    // MALLOC'ED VARIABLE
    return allocate_one_string(ONES_A[n]);
}
