#pragma once
#include "armadillo"

#ifndef constants
#define PI 3.141592653589793
#define ec 1.6021766E-19
#define eps0 8.8541878E-12
#endif

void writeVectorToFile(arma::vec, FILE*);
void writeVectorToFile(arma::rowvec, FILE*);
void writeVectorsToFile(const arma::mat&, FILE*, std::string mode = "row");
arma::vec readVectorFromFile(std::string);

/* Routines for DoS calculation */
std::complex<double> rGreenF(double, double, double);
double densityOfStates(double, double, const arma::mat&);
void writeDensityOfStates(const arma::mat&, double, FILE*);