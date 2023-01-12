#pragma once
#include<stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	int allocated_size;
	int count;
	int* ptr;
} vector_t;


void init(vector_t* v);


// Realokacja wektora *v, tak aby bufor miał pojemność reallocate_size.
void reallocate(vector_t* v, int reallocate_size);


// Umieszczenie wartości val jako nowego (końcowego) elementu wektora *v.
void push_back(vector_t* v, int val);


// Pobranie i usunięcie wartości końcowego elementu wektora *v.
int pop_back(vector_t* v);


void clear_vector(vector_t* v);