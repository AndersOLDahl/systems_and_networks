/*
 * CS3600, Spring 2014
 * C Bootcamp, Homework 2, Problem 1
 * (c) 2014 Alan Mislove
 *
 * In this problem, your goal is to fill in the string_to_time function.
 * Do not touch anything outside of the string_to_time function.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

char *string_to_time(struct tm time);

int main(int argc, char **argv) {
  // check for the right number of arguments
  if (argc != 10) {
    printf("Error: Usage: ./cp2-problem1 [seconds] [minutes] [hours] [mday] [mon] [year] [dow] [doy] [isdst]\n");
    return 0;
  }

  // create the time structure
  struct tm myTm;
  myTm.tm_sec=atoi(argv[1]);
  myTm.tm_min=atoi(argv[2]);
  myTm.tm_hour=atoi(argv[3]);
  myTm.tm_mday=atoi(argv[4]);
  myTm.tm_mon=atoi(argv[5]);
  myTm.tm_year=atoi(argv[6]);
  myTm.tm_wday=atoi(argv[7]);
  myTm.tm_yday=atoi(argv[8]);
  myTm.tm_isdst=atoi(argv[9]);

  char *result = string_to_time(myTm);

  // print out the result
  printf("Time was: %s", result);

  return 0;
}

/**
 * This function should print out the passed-in time variable in
 * C-standard format followed by a \n, e.g.,
 *
 * Mon Jan  9 04:33:20 2012\n
 *
 * There's a very easy way and a very annoying way to do this problem.  I would
 * strongly hint to look at the man pages to find the easy way.
 */
char *string_to_time(struct tm time) {
  return asctime(&time);
}

