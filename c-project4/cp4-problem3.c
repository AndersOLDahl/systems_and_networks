/*
 * CS3600, Spring 2014
 * C Bootcamp, Homework 4, Problem 3
 * (c) 2012 Alan Mislove
 *
 * In this problem, we have implemented a library that provides a
 * singly-linked list of ints.  The list elements are kept in order.
 *
 * However, there are a number of bugs.  Find and fix them.
 *
 * Do not touch anything in the main() function, although you may
 * modify the linked_list structures.
 */

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

struct linked_list_element_s;

typedef struct linked_list_element_s {
    int i;
    struct linked_list_element_s *next;
} linked_list_element;

typedef struct linked_list_s {
    int elements;
    long id;
    linked_list_element *head;
} linked_list;

linked_list *make_list();
int size(linked_list *list);
void add_element(linked_list *list, int i);

/**
 * A utility function that will print out the list.
 * I promise that no bugs do exist here.
 */
void dump(linked_list *list) {
    printf("%d elements: [", list->elements);

    linked_list_element *e = list->head;

    while (e != NULL) {
        printf(" %d", e->i);
        e = e->next;
    }

    printf(" ]\n");
}


int main(int argc, char **argv) {
    linked_list *list = make_list();

    int i;
    for (i=1; i<argc; i++) {
        add_element(list, atoi(argv[i]));
        dump(list);
    }

    return 0;
}

/**
 * This function should allocate a new list structure (via malloc)
 * and return it.
 */
linked_list *make_list() {

    linked_list *list = (linked_list *) malloc(sizeof(linked_list));
    list->head = NULL;
    list->elements = 0;
    list->id = (long) &list;

    // Returning Malloc'ED Variable
    return list;
}

/**
 * This function should return the number of elements present in
 * the list.
 */
int size(linked_list *list) {
    return list->elements;
}

/**
 * This function should add the given element to the provided
 * list.  The list should be stored in ascending order, and
 * duplicates are allowed.
 */
void add_element(linked_list *list, int i) {
    linked_list_element * e = (linked_list_element *) malloc(sizeof(linked_list_element));
    e -> i = i;

    linked_list_element * f = list -> head;

    if (f == NULL) {
        list -> head = e;
    } else {
        if (f->i > i) {
            e->next = f;
            list->head = e;
        } else {
            while ((f->next != NULL) && (f->next->i < i))
                f = f->next;

            e->next = f->next;
            f->next = e;
        }
    }

    list -> elements++;
}

