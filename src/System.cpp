#include <complex>
#include <math.h>
#include <stdlib.h>
#include <iostream>

#include "System.hpp"


// -------------------- Constructor and destructor --------------------

System::System(SystemConfiguration& configuration) : Crystal(){

	initializeCrystalAttributes(configuration);
	initializeSystemAttributes(configuration);
	
	std::cout << "Correctly initiallized System object" << std::endl;
};

System::~System(){
	std::cout << "Destroying System object..." << std::endl;
};

// ----------------------------- Methods -----------------------------
/* Routine to extract the information contained in the SystemConfiguration object from
parsing the input text file */
void System::initializeSystemAttributes(const SystemConfiguration& configuration){
	
	orbitals_            = configuration.systemInfo.norbitals;
	hamiltonianMatrices  = configuration.systemInfo.hamiltonian;
	overlapMatrices      = configuration.systemInfo.overlap;
	filling_			 = configuration.systemInfo.filling;
	fermiLevel_			 = filling_ - 1;

    int basisdim = 0;
    for(int i = 0; i < natoms; i++){
        int species = this->motif.row(i)(3);
        basisdim += orbitals(species);
    }
	basisdim_   = basisdim;

	std::cout << __LINE__ << std::endl;
}

/* Bloch hamiltonian for posterior diagonalization. Input: arma::vec k (wave number) */
arma::cx_mat System::hamiltonian(arma::rowvec k, bool isTriangular){

	arma::cx_mat h = arma::zeros<arma::cx_mat>(basisdim, basisdim);
	std::complex<double> imag(0, 1);
	for (int i = 0; i < ncells; i++){
		arma::rowvec cell = unitCellList.row(i);
		h += hamiltonianMatrices.slice(i) * std::exp(imag*arma::dot(k, cell));
	};

	if (isTriangular){
		h.diag() -= h.diag()/2;
		h += h.t();
	}

	return h;
};

/* Overlap matrix in reciprocal space to solve generalized eigenvalue problem */
arma::cx_mat System::overlap(arma::rowvec k, bool isTriangular){

	arma::cx_mat s = arma::zeros<arma::cx_mat>(basisdim, basisdim);
	std::complex<double> imag(0, 1);
	for (int i = 0; i < ncells; i++){
		arma::rowvec cell = unitCellList.row(i);
		s += overlapMatrices.slice(i) * std::exp(imag*arma::dot(k, cell));
	};

	if (isTriangular){
		s.diag() -= s.diag()/2;
		s += s.t();
	}

	return s;
}

void System::setFilling(int filling){
	if (filling > 0){
		filling_ = filling;
		fermiLevel_ = filling_;
	}
	else{
		std::cout << "Filling must be a positive integer" << std::endl;
	}
}

void System::solveBands(arma::rowvec& k, arma::vec& eigval, arma::cx_mat& eigvec, bool triangular){
	arma::cx_mat h = hamiltonian(k, triangular);
	double auToEV = 13.6;
	if (!overlapMatrices.empty()){
		arma::cx_vec auxEigval;
		arma::cx_mat s = overlap(k, triangular);
		arma::eig_pair(auxEigval, eigvec, h, s);
		arma::uvec sorted_indices = arma::sort_index(auxEigval);
		eigval = arma::sort(arma::real(auxEigval)*auToEV);
		eigvec = eigvec.cols(sorted_indices);
	}
	else{
		arma::eig_sym(eigval, eigvec, h);
	}
	
}

// /*------------------ Utilities/Additional observables ------------------*/

// BEWARE: Basis order for spin value is NOT correct
/* Routine to calculate the expected value of the spin projection
Sz of a TB eigenstate. */
double System::expectedSpinZValue(const arma::cx_vec& eigvec){

	arma::cx_vec spinEigvalues = {1./2, -1./2};
    arma::cx_vec spinVector = arma::zeros<arma::cx_vec>(basisdim, 1);
    int vecIterator = 0;
    for(int atomIndex = 0; atomIndex < natoms; atomIndex++){
        int species = motif.row(atomIndex)(3);
        spinVector.subvec(vecIterator, vecIterator + orbitals(species)) = 
                    arma::kron(spinEigvalues, arma::ones(orbitals(species)/2, 1));
    }

	arma::cx_vec spinEigvec = eigvec % spinVector;

	return real(arma::cdot(eigvec, spinEigvec));
};

// TODO: ADD CALCULATION OF X, Y SPIN COMPONENTS
/* Routine to calculate the expected value of the spin projection
Sz of a TB eigenstate. */
double System::expectedSpinYValue(const arma::cx_vec& eigvec){

	std::complex<double> i(0,1);
	arma::cx_mat operatorSy = arma::zeros<arma::cx_mat>(basisdim, basisdim);
	operatorSy(0,1) = -i;
	operatorSy(1,0) = i;
	operatorSy.submat(2,5, 4,7) = -i*arma::eye<arma::cx_mat>(3,3);
	operatorSy.submat(5,2, 7,4) = i*arma::eye<arma::cx_mat>(3,3);
	
};

/* Routine to calculate the expected value of the spin projection
Sz of a TB eigenstate. */
double System::expectedSpinXValue(const arma::cx_vec& eigvec){

	arma::cx_mat operatorSx = arma::zeros<arma::cx_mat>(basisdim, basisdim);
	operatorSx(0,1) = 1;
	operatorSx(1,0) = 1;
	operatorSx.submat(2,5, 4,7) = arma::eye<arma::cx_mat>(3,3);
	operatorSx.submat(5,2, 7,4) = arma::eye<arma::cx_mat>(3,3);

};