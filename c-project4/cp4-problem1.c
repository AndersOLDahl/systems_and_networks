/*
 * CS3600, Spring 2014
 * C Bootcamp, Homework 4, Problem 1
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to learn about debugging C programs.
 * Each of the following functions below has an error (often a
 * segfault).  You should debug and fix each of the functions.  Note
 * that there is likely to be multiple errors in each function.
 *
 * Be sure to not touch anything in the main() function
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *str_find(char *s, char c);
char *str_append(char *s1, const char *s2);
int str_compare(char *s1, char *s2, size_t n);

int main(int argc, const char **argv) {
  setbuf(stdout, NULL);

  // check for the right number of arguments
  if (argc != 3) {
    printf("Error: Usage: ./cp4-problem1 [str1] [str2]\n");
    return 0;
  }

  char *str1 = (char *) argv[1];
  char *str2 = (char *) argv[2];
  char *str3 = (char *) malloc(strlen(argv[1])+strlen(argv[2])+1);
  strcpy(str3, str1);

  int cmp = str_compare(str1, str2, (strlen(str1) < strlen(str2) ? strlen(str1) : strlen(str2)));
  printf("Comparing '%s' and '%s' is: %d\n", str1, str2, cmp);

  char *loc = str_find(str1, str2[0]);
  printf("Finding '%c' in '%s' is: %s\n", str2[0], str1, loc);

  str_append(str3, str2);
  printf("The concatenation is: %s\n", str3);
}

/**
 * The str_find() function shall locate the first occurrence of c
 * in the string pointed to by s. The terminating null byte is
 * considered to be part of the string. The function returns
 * the location of the found character, or a null pointer if
 * the character was not found.
 */
char *str_find(char *s, char c) {
  /* Scan s for the character.  When this loop is finished,
     s will either point to the end of the string or the
     character we were looking for.  */
    while ((*s != '\0') && (*s != c))
        s++;

    return ((*s == c) ? s : NULL );
}

/**
 * The str_append() function shall append a copy of the string
 * pointed to by s2 (including the terminating null byte)
 * to the end of the string pointed to by s1. The initial
 * byte of s2 overwrites the null byte at the end of s1.
 * If copying takes place between objects that overlap,
 * the behavior is undefined. The function returns s1.
 */
char *str_append(char *s1, const char *s2) {
    /* Copy the contents of s2 into the space at the end of s1.  */
    strcat(s1, s2);

    return s1;
}

/**
 * The str_compare() function shall compare not more than
 * n bytes (bytes that follow a null byte are not compared)
 * from the array pointed to by s1 to the array pointed to
 * by s2. The sign of a non-zero return value is determined
 * by the sign of the difference between the values of the
 * first pair of bytes (both interpreted as type unsigned
 * char) that differ in the strings being compared. The
 * function should return -1 if s1 < s2, 0 if equal, 1 if
 * s1 > s2.
 */
int str_compare(char *s1, char *s2, size_t n) {
    unsigned char uc1, uc2;

    /* Nothing to compare?  Return zero.  */
    if (n == 0)
        return 0;

    /* Loop, comparing bytes.  */
    while (n > 0 && s1 == s2) {

        n--;

        /* If we've run out of bytes or hit a null, return zero
           since we already know *s1 == *s2.  */

        if (n == 0 || (*s1 = '\0'))
            return 0;

        s1++;
        s2++;
    }

    uc1 = *s1;
    uc2 = *s2;

    return ((uc1 < uc2) ? -1 : (uc1 > uc2));
}
