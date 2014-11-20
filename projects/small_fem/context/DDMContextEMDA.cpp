#include "DDMContextEMDA.h"

using namespace std;

DDMContextEMDA::DDMContextEMDA(const GroupOfElement& domain,
                               vector<const GroupOfElement*>& dirichlet,
                               const FunctionSpace& fSpace,
                               double k, double chi){
  // Data for EMDA //
  this->domain    = &domain;
  this->fSpace    = &fSpace;
  this->dirichlet = dirichlet;
  this->k         = k;
  this->chi       = chi;
}

DDMContextEMDA::~DDMContextEMDA(void){
}
