#pragma once
#include<stdio.h>
#include <stdlib.h>
#include <string.h>


struct Result {
	double time;
	int score;
};


typedef struct {
	int allocated_size;
	int count;
	Result* ptr;
} vector_t;


void init(vector_t* v);


// Reallocate the *v vector so that the buffer has reallocate_size capacity.
void reallocate(vector_t* v, int reallocate_size);


// Placing 'val' as the new (final) element of the vector *v.
void push_back(vector_t* v, Result val);


// Getting and removing the value of the final element of the vector *v.
Result pop_back(vector_t* v);
