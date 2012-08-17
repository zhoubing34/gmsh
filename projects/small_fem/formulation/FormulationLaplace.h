#ifndef _FORMULATIONLAPLACE_H_
#define _FORMULATIONLAPLACE_H_

#include <vector>

#include "Polynomial.h"
#include "Formulation.h"

/**
   @class FormulationLaplace
   @brief Formulation for the Laplace problem

   Formulation for the @em Laplace problem.

   @todo
   Remove ALL const_cast%S by correcting MElement constness@n
   Allow Hybrid Mesh
 */

class FormulationLaplace: public Formulation{
 private:
  // Gaussian Quadrature Data //
  int G;
  fullMatrix<double>* gC;
  fullVector<double>* gW;

  // Function Space //
  FunctionSpace* fspace;

  // Grad Field //
  std::vector<Polynomial>* gradBasis;

 public:
  FormulationLaplace(const GroupOfElement& goe);

  virtual ~FormulationLaplace(void);

  virtual double weak(int nodeI, int nodeJ, 
		      const GroupOfDof& god) const;

  virtual double rhs(int equationI,
		     const GroupOfDof& god) const;

  virtual const FunctionSpace& fs(void) const;
};

/**
   @fn FormulationLaplace::FormulationLaplace
   @param goe A GroupOfElement

   Instantiates a new FormulationLaplace@n

   The given GroupOfElement will be used as the 
   geomtrical @em domain
   **

   @fn FormulationLaplace::~FormulationLaplace
   Deletes this FormulationLaplace
   **
*/

//////////////////////
// Inline Functions //
//////////////////////

inline double FormulationLaplace::rhs(int equationI,
				      const GroupOfDof& god) const{
  return 0;
}

inline const FunctionSpace& FormulationLaplace::fs(void) const{
  return *fspace;
}

#endif
