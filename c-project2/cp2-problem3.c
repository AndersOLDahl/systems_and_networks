/*
 * CS3600, Spring 2014
 * C Bootcamp, Homework 2, Problem 3
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to learn about strings (character arrays).
 * You should fill in the functions below, as described, and be sure to
 * not touch anything in the main() function.
 *
 * Also note that you may not use any of the functions provided by <string.h>;
 * you must do all of the string manipulation yourself.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void str_cat(char *str1, char *str2, char *dst);
int count_lowercase(char *str);
void reverse(char *str);
int count(char * str);

int main(int argc, const char **argv) {
  // check for the right number of arguments
  if (argc != 3) {
    printf("Error: Usage: ./cp2-problem3 [str1] [str2]\n");
    return 0;
  }

  char *str1 = (char *) argv[1];
  char *str2 = (char *) argv[2];
  char *str3 = (char *) malloc(strlen(argv[1])+strlen(argv[2])+1);

  str_cat(str1, str2, str3);
  printf("The concatenation is: %s\n", str3);

  int t = count_lowercase(str3);
  printf("There are a total of %d lowercase characters.\n", t);

  reverse(str3);
  printf("The reverse is: %s\n", str3);
}

/**
 * This function takes in three char*s, and should concatenate the
 * first two and store the result in the third.  For example, if the
 * value of str1 is "foo" and str2 is "bar", you should store the
 * string "foobar" into dst.  Be careful about the trailing \0s.
 * You can assume that dst has sufficient space for the result (but
 * no more).
 */
void str_cat(char *str1, char *str2, char *dst) {

    int count_str1 = count(str1);

    for(int i = 0; i < count_str1; i++) {
        dst[i] = str1[i];
    }

    for(int j = 0; j < count(str2) + 1; j++) {
        dst[count_str1 + j] = str2[j];

    }

    return;
}

/**
 * This function should count the number of lowercase letters ('a' .. 'z')
 * that occur in the argument and return the result.
 */
int count_lowercase(char *str) {

    int count = 0;

    while(*str != '\0') {
        if (*str >= 'a' && *str <= 'z') {
            count++;
        }

        str++;
    }

    return count;
}

/**
 * This function should reverse the string argument *in-place*.  In other
 * words, you should change the characters in str so that they are in the
 * reverse order.
 */
void reverse(char *str) {

    int size = count(str);
    int i = 0;

    for(; i < (size / 2); i++) {
        char tmp = str[i];
        str[i] = str[size - i - 1];
        str[size - i - 1] = tmp;
    }

}

int count(char * str) {
    int count = 0;

    do  {

        count++;
        str++;

    } while(*str != '\0');

    return count;

}
