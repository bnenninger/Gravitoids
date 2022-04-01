// COEN 4720
// Project
// Gravitoids: Asteroids with Extra Physics and Multiplayer
// Brendan Nenninger, Kassie Povinelli, Carl Sustar
//
// vector.c
// provided 2d vector math utilities for the game engine

#include <stdio.h>
#include <math.h>
#include "vector.h"

struct vector2d result;
float matrix[2][2];
float x, y, ratio, angle, sine, cosine;

// adds the two passed vectors
// the result is stored in vector a
void add_vector(struct vector2d *a, struct vector2d *b)
{

	a->x += b->x;
	a->y += b->y;
}

// adds the two passed vectors and returns the result without modifying the passed vectors
struct vector2d add_vector_new(struct vector2d *a, struct vector2d *b)
{

	result.x = a->x + b->x;
	result.y = a->y + b->y;

	return result;
}

// multiplies the magnitude of the passed vector by the passed float
void multiply_vector(struct vector2d *v, float n)
{

	v->x *= n;
	v->y *= n;
}

// divides the magnitude of the passed vector by the passed float
void divide_vector(struct vector2d *v, float n)
{

	v->x /= n;
	v->y /= n;
}

// prints the x and y values of the passed vector
void print_vector(struct vector2d *a)
{
	printf("x = %f\n y = %f\n", a->x, a->y);
}

// returns the magnitude of the passed vector
float magnitude_vector(struct vector2d *v)
{

	float c2 = pow(v->x, 2) + pow(v->y, 2);

	return sqrt(c2);
}

// normalizes the passed vector (divides by its magnitude to make its
// magnitude 1 while still retaining the direction)
void normalise_vector(struct vector2d *v)
{
	float mag = magnitude_vector(v);
	divide_vector(v, mag);
}

// limits the magnitude of the passed vector to the passed limit
// if the magnitude is under the limit, it is unaffected
// if the magnitude if over the limit, it is brought down to be equal to the limit
void limit_vector(struct vector2d *v, float limit)
{

	float mag = magnitude_vector(v);

	if (mag > limit)
	{

		ratio = limit / mag;
		v->x *= ratio;
		v->y *= ratio;
	}
}

// rotates the passed vector by the passed number of degrees
void rotate_vector(struct vector2d *v, float degrees)
{

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
