/*
 * CS3600, Spring 2014
 * C Bootcamp, Homework 2, Problem 2
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to learn about C structures.  You will
 * be mananging employee records.  Each employee record consists of
 * the following values:
 *
 *  - A name string (less than 49 chars long)
 *  - A salary (integer, positive, less than $4B)
 *  - A employee ID (integer, positive, less than 4B)
 *  - A location string (less than 49 chars long)
 *
 * You are guaranteed that there will be no more than 10 employees. You
 * should support 10, and exactly 10, employees.
 *
 * You should do the following:
 *
 *  - Define an appropriate structure to represent each employee. Call
 *    this structure "struct employee" (surprise!).
 *
 *  - Define a global location to store the employee records (hint: array).
 *    Be sure to have a way to keep track of which employee records
 *    are in-use and which are empty.
 *
 *  - Fill in the add_employee, remove_employee, get_salary functions
 *
 * Do not touch anything inside the main() function.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int add_employee(char *name, unsigned int id, unsigned int salary, char *location);
int remove_employee(unsigned int id);
int get_salary(unsigned int id);

// Do not touch this function
int main(int argc, char **argv) {
    int i;
    for (i=1; i<argc; i++) {
        char *arg = argv[i];
        switch (arg[0]) {
            case 'a': {
                          char *name = (char*) malloc(100);
                          unsigned int salary;
                          unsigned int id;
                          char *location = (char*) malloc(100);

                          sscanf(arg, "a %[A-Z-a-z] %u %u %[A-Z-a-z]", name, &id, &salary, location);

                          int result = add_employee(name, id, salary, location);
                          if (result == -1) printf("Err: Employee name already exists.\n");
                          if (result == -2) printf("Err: Employee id already exists.\n");
                          if (result == -3) printf("Err: Too many employees!\n");
                      }
                      break;
            case 'r': {
                          unsigned int id;

                          sscanf(arg, "r %u", &id);

                          int result = remove_employee(id);
                          if (result == -1) printf("Err: Employee %d not found.\n", id);
                      }
                      break;
            case 's': {
                          unsigned int id;

                          sscanf(arg, "s %u", &id);

                          int result = get_salary(id);
                          if (result == -1) printf("Err: Employee %d not found.\n", id);
                          else printf("Salary of %d: %d\n", id, result);
                      }
                      break;
            default: {
                         printf("Error: Unknown command '%c'.\n", arg[0]);
                         return -1;
                     }
        }
    }

    return 0;
}

typedef struct employee {
    char * name;
    unsigned int salary;
    unsigned int id;
    char * location;
} employee;

employee employees[10];

int size_of_employee_list = sizeof(employees) / sizeof(employee);
int number_of_employees = 0;

/**
 * This function should add an employee to the list.  The passed-in arguments
 * describe the employee.  You should add the employee and return one of the following
 * values:
 *   0 - success
 *  -1 - if an employee with an identical name already exists
 *  -2 - if an employee with an identical ID aleady exists (but not an identical name)
 *  -3 - if there is no more space
 */
int add_employee(char *name, unsigned int id, unsigned int salary, char *location) {

    int i;
    employee e;

    if (number_of_employees == size_of_employee_list) {
        return -3;
    }

    for(i = 0; i < number_of_employees; i++) {

        if (strcmp(employees[i].name, name) == 0) {
            return -1;
        }

        if (employees[i].id == id) {
            return -2;
        }
    }

    e.name = name;
    e.id = id;
    e.salary = salary;
    e.location = location;

    employees[number_of_employees] = e;
    number_of_employees++;

    return 0;
}

/**
 * This function should remove an existing employee from the list, based on the
 * provided ID.  You should return one of the following values:
 *  0 - success
 * -1 - if employee was not found
 */
int remove_employee(unsigned int id) {

    int i;

    for(i = 0; i < number_of_employees; i++) {
        if(employees[i].id == id) {
            int j;

            for(j = i; j < number_of_employees; j++) {
                employees[j] = employees[j + 1];

            }

            number_of_employees--;
            return 0;

        }
    }

    return -1;
}

/**
 * This function should look up the salary of the employee assigned the given ID,
 * and return the corresponding value. You should return one of the following values:
 * >= 0 - success, the salary value
 *   -1 - if employee was not found
 */
int get_salary(unsigned int id) {

    int i;

    for(i = 0; i < number_of_employees; i++) {
        if (employees[i].id == id) {
            return employees[i].salary;
        }
    }

    return -1;
}
