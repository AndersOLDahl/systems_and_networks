/*
 * CS3600, Spring 2014
 * C Bootcamp, Homework 1, Problem 3
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to fill in the great_circle_distance() function located at the
 * bottom of this file.  Do not touch anything outside of this function.
 *
 * For the latitudes, the values should range between -90 (representing S) and 90 (representing N).
 * For the longitudes, the values should range between -180 (representing E) and 180 (representing W).
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// the prototype of the function
double great_circle_distance(double lat1, double lon1, double lat2, double lon2);

int main(int argc, char *argv[]) {
  // check for the right number of arguments
  if (argc != 5) {
    printf("Error: Usage: ./cp1-problem3 [lat1] [lon1] [lat2] [lon2]\n");
    return 0;
  }

  // interpret the variables
  double lat1 = atof(argv[1]);
  double lon1 = atof(argv[2]);
  double lat2 = atof(argv[3]);
  double lon2 = atof(argv[4]);

  // calculate the result
  double result = great_circle_distance(lat1, lon1, lat2, lon2);

  // print it out
  if (result >= 0) {
    printf("Success: The great circle distance between those two points is: %f\n", result);
    return 0;
  } else if (result == -1) {
    printf("Error: Invalid latitude or longitudes passed in.\n");
    return -1;
  } else {
    printf("Error: Unknown error.\n");
    return -2;
  }
}

/**
 * This function returns the great circle distance in miles between the two latitude/longitude
 * points, using the Haversine formula.  You should treat all variables as doubles, and
 * you should use the formula
 *
 * dlon = lon2 - lon1
 * dlat = lat2 - lat1
 * a = (sin(dlat/2))^2 + cos(lat1) * cos(lat2) * (sin(dlon/2))^2
 * c = 2 * atan2( sqrt(a), sqrt(1-a) )
 * distance = R * c (where R is the radius of the Earth)
 *
 * You should use 3961 miles as the radius of the earth.  You should also use the
 * sin, cos, sqrt, and atan2 functions from math.h.  For more information, look up
 *
 * man 3 atan2
 *
 * If invalid latitudes or longitudes are passed in (e.g., the latitude is greater than
 * 90 or less that -90, or the longitude is greater than 180 or less than -180), you should
 * return -1).
 *
 * Finally, the latitudes and longitudes will be given to you as degrees; you MUST convert
 * these to radians to use the above formula.  You can do so by using the formula
 *
 * randians = degrees * PI/180;
 *
 * For these calculations, use PI = 3.14159.

 */

#define PI 3.14159
#define RADIUS_OF_EARTH 3961

double great_circle_distance(double lat1, double lon1, double lat2, double lon2) {

  if (abs(lat1) > 90.0 || abs(lat2) > 90.0 || abs(lon1) > 180.0 || abs(lon2) > 180.0) {
    return -1;
  }

  double lat1_r = lat1 * (PI / 180);
  double lat2_r = lat2 * (PI / 180);
  double lon1_r = lon1 * (PI / 180);
  double lon2_r = lon2 * (PI / 180);

  double dlon = lon2_r - lon1_r;
  double dlat = lat2_r - lat1_r;
  double a = pow((sin(dlat/2)), 2) + cos(lat1_r) * cos(lat2_r) * pow((sin(dlon/2)), 2);
  double c = 2 * atan2( sqrt(a), sqrt(1-a) );
  double distance = RADIUS_OF_EARTH * c;

  return distance;
}
