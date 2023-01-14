#define _CRT_SECURE_NO_WARNINGS
#include "vector.h"


void init(vector_t* v) {
	v->count = 0;
	v->allocated_size = 1;
	v->ptr = (Result*)malloc(v->allocated_size * sizeof(Result));
}


void reallocate(vector_t* v, int reallocate_size) {
	// Realokacja wektora *v, tak aby bufor miał pojemność reallocate_size.
	v->allocated_size = reallocate_size;
	Result* newPtr = (Result*)malloc(v->allocated_size * sizeof(Result));
	if (newPtr != 0) memcpy(newPtr, v->ptr, v->count * sizeof(Result));
	free(v->ptr);
	v->ptr = newPtr;
}


void push_back(vector_t* v, Result val) {
	// Umieszczenie wartości val jako nowego (końcowego) elementu wektora *v.
	if (v->count == v->allocated_size)
		reallocate(v, 2 * v->allocated_size);
	v->ptr[v->count] = val;
	v->count++;
	printf("PUSHED, count = %d\n", v->count);
}


Result pop_back(vector_t* v) {
	// Pobranie i usunięcie wartości końcowego elementu wektora *v.
	v->count--;
	Result retv = v->ptr[v->count];
	if (4 * v->count <= v->allocated_size)
		reallocate(v, v->allocated_size / 2);
	return retv;
}


void clear_vector(vector_t* v) {
	int iter = v->count;
	for (int i = 0; i < iter; i++)
		pop_back(v);
}