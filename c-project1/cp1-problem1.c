/*
 * CS3600, Spring 2014
 * C Bootcamp, Homework 1, Problem 1
 * (c) 2014 Alan Mislove
 *
 * In this problem, your goal is to fill in the factorial() function located at the
 * bottom of this file.  Do not touch anything outside of the factorial function.
 */

#include <stdio.h>
#include <stdlib.h>

// the prototype of the function
int factorial(int n);
int factorial_helper(int n);

int main(int argc, char *argv[]) {
  // check for the right number of arguments
  if (argc != 2) {
    printf("Error: Usage: ./cp1-problem1 [int]\n");
    return 0;
  }

  // interpret the input
  int value = atoi(argv[1]);

  // calculate the result
  int result = factorial(value);

  // print it out
  if (result >= 0) {
    printf("Success: The factorial of that number is: %d\n", result);
    return 0;
  } else if (result == -1) {
    printf("Error: Cannot take the factorial of a negative number.\n");
    return -1;
  } else if (result == -2) {
    printf("Error: Overflow.\n");
    return -2;
  } else {
    printf("Error: Unknown error.\n");
    return -3;
  }
}

/**
 * This function returns the factorial (!) of the input value.  If a negative
 * number is passed in, it should return -1. If overflow occurs, the it should
 * return -2.
 */
int factorial(int n) {

    if ( n < 0 ) {
        return -1;
    } else if ((sizeof(int) == 4 && n > 12) || (sizeof(int) == 8 && n > 20)) {
        return -2;
    } else {
        return factorial_helper(n);
    }
}

int factorial_helper(int n) {

    if (n == 0 || n == 1) {
        return 1;
    } else {
        return n * factorial_helper(n - 1);
    }
}
