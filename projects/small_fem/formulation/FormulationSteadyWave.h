#ifndef _FORMULATIONSTEADYWAVE_H_
#define _FORMULATIONSTEADYWAVE_H_

#include "FormulationBlock.h"
#include "GroupOfElement.h"
#include "FunctionSpace.h"
#include "Term.h"

/**
   @class FormulationSteadyWave
   @brief Formulation for the steady wave problem

   Formulation for the steady wave problem
 */

template<typename scalar>
class FormulationSteadyWave: public FormulationBlock<scalar>{
 private:
  // Function Space & Domain //
  const FunctionSpace*  ffs;
  const GroupOfElement* ddomain;

  // Wavenumber Squared //
  double kSquare;

  // Local Terms //
  Term<scalar>* stif;
  Term<scalar>* mass;
  Term<scalar>* src;

 public:
  FormulationSteadyWave(const GroupOfElement& domain,
                        const FunctionSpace& fs,
                        double k);

  FormulationSteadyWave(const GroupOfElement& domain,
                        const FunctionSpace& fs,
                        double k,
                        void (*nu)(fullVector<double>&, fullMatrix<scalar>&),
                        void (*eps)(fullVector<double>&, fullMatrix<scalar>&),
                        fullVector<scalar> (*source)(fullVector<double>&));

  virtual ~FormulationSteadyWave(void);

  virtual scalar weak(size_t dofI, size_t dofJ, size_t elementId) const;
  virtual scalar rhs(size_t equationI, size_t elementId)          const;

  virtual const FunctionSpace&  field(void)  const;
  virtual const FunctionSpace&  test(void)   const;
  virtual const GroupOfElement& domain(void) const;

  virtual bool isBlock(void) const;
};

/**
   @fn FormulationSteadyWave::FormulationSteadyWave
   @param domain A GroupOfElement
   @param fs A FunctionSpace  for both unknown and test field
   @param k a scalar

   Instantiates a new FormulationSteadyWave with given parametres:
   @li domain for the domain of this Formulation
   @li fs for the function space used for the unknown field
       and the test functions
   @li k for wavenumber
   **

   @fn FormulationSteadyWave::~FormulationSteadyWave
   Deletes this FormulationSteadyWave
*/

//////////////////////////////////////
// Templates Implementations:       //
// Inclusion compilation model      //
//                                  //
// Damn you gcc: we want 'export' ! //
//////////////////////////////////////

#include "FormulationSteadyWaveInclusion.h"

#endif
