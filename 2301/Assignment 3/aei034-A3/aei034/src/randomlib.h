#pragma once
#ifndef RANDOMLIB_H
/// <summary>Header inclusion macro.</summary>
#define RANDOMLIB_H 1

void RandomInitialise(int, int);

double RandomUniform(void);

double RandomGaussian(double, double);

int RandomInt(int, int);

double RandomDouble(double, double);


#endif //RANDOMLIB_H