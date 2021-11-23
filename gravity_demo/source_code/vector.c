
//vector.c

#include <stdio.h>
#include <math.h>
#include "vector.h"

struct vector2d result;
float matrix[2][2];
float x, y, ratio, angle, sine, cosine;

void add_vector(struct vector2d* a, struct vector2d* b) {

	a->x += b->x;
	a->y += b->y;
}

struct vector2d add_vector_new(struct vector2d* a, struct vector2d* b) {

	result.x = a->x + b->x;
	result.y = a->y + b->y;

	return result;
}

void multiply_vector(struct vector2d* v, float n) {

	v->x *= n;
	v->y *= n;
}

void divide_vector(struct vector2d* v, float n) {

	v->x /= n;
	v->y /= n;
}

void print_vector(struct vector2d* a) {
	
	printf("x = %f\n y = %f\n", a->x, a->y);
}

float magnitude_vector(struct vector2d* v) {
	
	float c2 = pow(v->x, 2) + pow(v->y, 2); 

	return sqrt(c2);
}

void normalise_vector(struct vector2d* v) {

	float mag = magnitude_vector(v);

	divide_vector(v, mag);
}

void limit_vector(struct vector2d* v, float limit) {
	
	float mag = magnitude_vector(v);

	if (mag > limit) {
		
		ratio = limit / mag;
		v->x *= ratio;
		v->y *= ratio;
	}
}

void rotate_vector(struct vector2d* v, float degrees) {
	
	//calculate radians
	angle = degrees * 3.14159265 / 180;
	sine = sin(angle);
	cosine = cos(angle);
	
	//rotation matix
	matrix[0][0] = cosine;
	matrix[0][1] = -sine;
	matrix[1][0] = sine;
	matrix[1][1] = cosine;
	//matrix[2][2] = {{cosine, -sine}, {sine, cosine}};

	x = v->x;
	y = v->y;

	v->x = matrix[0][0] * x + matrix[0][1] * y;
        v->y = matrix[1][0] * x + matrix[1][1] * y;
}

