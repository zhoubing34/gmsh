#include "GroupOfJacobian.h"
#include "Quadrature.h"

#include "Exception.h"
#include "FormulationUpdateOO2.h"

using namespace std;

FormulationUpdateOO2::
FormulationUpdateOO2(const GroupOfElement& domain,
                     const FunctionSpaceScalar& fs,
                     Complex a,
                     Complex b,
                     const std::map<Dof, Complex>& sol,
                     const std::map<Dof, Complex>& oldG){

  // Check GroupOfElement Stats: Uniform Mesh //
  pair<bool, size_t> uniform = domain.isUniform();
  size_t               eType = uniform.second;

  if(!uniform.first)
    throw Exception("FormulationUpdateOO2 needs a uniform mesh");

  // a & b //
  this->a = a;
  this->b = b;

  // Save FunctionSpace & Domain //
  fspace = &fs;
  goe    = &domain;

  // Get Basis //
  const Basis& basis = fs.getBasis(eType);

  // Gaussian Quadrature (Field - Field) //
  Quadrature gaussFF(eType, basis.getOrder(), 2);

  const fullMatrix<double>& gCFF = gaussFF.getPoints();
  const fullVector<double>& gWFF = gaussFF.getWeights();

  // Gaussian Quadrature (Grad - Grad) //
  Quadrature gaussGG(eType, basis.getOrder() - 1, 2);

  const fullMatrix<double>& gCGG = gaussGG.getPoints();
  const fullVector<double>& gWGG = gaussGG.getWeights();

  // Pre-evalution //
  basis.preEvaluateFunctions(gCFF);
  basis.preEvaluateDerivatives(gCGG);

  // Jacobians //
  GroupOfJacobian jacFF(domain, gCFF, "jacobian");
  GroupOfJacobian jacGG(domain, gCGG, "invert");

  // Local Terms //
  lGout = new TermFieldField(jacFF, basis, gWFF);
  lGin  = new TermProjectionField<Complex>(jacFF, basis, gWFF, gCFF, fs, oldG);
  lU    = new TermProjectionField<Complex>(jacFF, basis, gWFF, gCFF, fs, sol);
  lDU   = new  TermProjectionGrad<Complex>(jacGG, basis, gWGG, gCGG, fs, sol);
}

FormulationUpdateOO2::~FormulationUpdateOO2(void){
  delete lGout;
  delete lGin;
  delete lU;
  delete lDU;
}

Complex FormulationUpdateOO2::
weak(size_t dofI, size_t dofJ, size_t elementId) const{
  return Complex(lGout->getTerm(dofI, dofJ, elementId), 0);
}

Complex FormulationUpdateOO2::rhs(size_t equationI, size_t elementId) const{
  return
    Complex(-1, 0)     * lGin->getTerm(0, equationI, elementId) +
    Complex(+2, 0) * a *   lU->getTerm(0, equationI, elementId) +
    Complex(-2, 0) * b *  lDU->getTerm(0, equationI, elementId);
}


const FunctionSpace& FormulationUpdateOO2::field(void) const{
  return *fspace;
}

const FunctionSpace& FormulationUpdateOO2::test(void) const{
  return *fspace;
}

const GroupOfElement& FormulationUpdateOO2::domain(void) const{
  return *goe;
}
