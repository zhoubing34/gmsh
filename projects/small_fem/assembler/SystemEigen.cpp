#include "slepceps.h"
#include "SystemEigen.h"

using namespace std;

SystemEigen::SystemEigen(const Formulation<std::complex<double> >& formulation){
  // Get Formulation //
  this->formulation = &formulation;

  // Get Formulation Dofs //
  set<Dof> dof;
  formulation.fsField().getKeys(formulation.domain(), dof);

  // Get Dof Manager //
  dofM = new DofManager<std::complex<double> >();
  dofM->addToDofManager(dof);

  // Is the Problem a General EigenValue Problem ? //
  general = formulation.isGeneral();

  // Init //
  A           = NULL;
  B           = NULL;
  eigenValue  = NULL;
  eigenVector = NULL;

  // The SystemEigen is not assembled and not solved//
  nEigenValues = 0;
  assembled    = false;
  solved       = false;
}

SystemEigen::~SystemEigen(void){
  if(eigenVector)
    delete eigenVector;

  if(eigenValue)
    delete eigenValue;

  if(A){
    MatDestroy(A);
    delete A;
  }

  if(B){
    MatDestroy(B);
    delete B;
  }

  delete dofM;
}

bool SystemEigen::isGeneral(void) const{
  return general;
}

size_t SystemEigen::getNComputedSolution(void) const{
  return nEigenValues;
}

void SystemEigen::getSolution(fullVector<std::complex<double> >& sol,
                              size_t nSol) const{
  sol.setAsProxy((*eigenVector)[nSol], 0, (*eigenVector)[nSol].size());
}

void SystemEigen::getSolution(std::map<Dof, std::complex<double> >& sol,
                              size_t nSol) const{
  // Get All Dofs
  std::map<Dof, std::complex<double> >::iterator it  = sol.begin();
  std::map<Dof, std::complex<double> >::iterator end = sol.end();

  // Loop on Dofs and set Values
  for(; it != end; it++)
    it->second = (*eigenVector)[nSol](dofM->getGlobalId(it->first));
}

void SystemEigen::getSolution(FEMSolution<std::complex<double> >& feSol) const{
  // Solved ?
  if(!solved)
    throw Exception("System: addSolution -- System not solved");

  // Coefficients //
  // FunctionSpace & Domain
  const FunctionSpace&  fs  = formulation->fsField();
  const GroupOfElement& goe = formulation->domain();

  // Get Dofs
  set<Dof> dof;
  fs.getKeys(goe, dof);

  // Get Coefficient
  const set<Dof>::iterator   end = dof.end();
  set<Dof>::iterator         it  = dof.begin();
  map<Dof, complex<double> > coef;

  for(; it != end; it++)
    coef.insert(pair<Dof, complex<double> >(*it, 0));

  // Iterate on Solutions //
  for(int i = 0; i < nEigenValues; i++){
    // Populate Map
    getSolution(coef, i);

    // FEMSolution
    feSol.addCoefficients(i, 0, goe, fs, coef);
  }
}

void SystemEigen::getEigenValues(fullVector<std::complex<double> >& eig) const{
  eig.setAsProxy(*eigenValue, 0, eigenValue->size());
}

void SystemEigen::
setNumberOfEigenValues(size_t nEigenValues){
  const size_t nDof = dofM->getUnfixedDofNumber();

  if(nEigenValues > nDof)
    throw
      Exception
      ("I can't compute more Eigenvalues (%d) than the number of unknowns (%d)",
       nEigenValues, nDof);

  else
    this->nEigenValues = nEigenValues;
}

void SystemEigen::assemble(void){
  // Enumerate //
  dofM->generateGlobalIdSpace();

  // Get All Field & Test Dofs per Element //
  vector<vector<Dof> > dofField;
  vector<vector<Dof> > dofTest;
  formulation->fsField().getKeys(formulation->domain(), dofField);
  formulation->fsTest().getKeys(formulation->domain(), dofTest);

  // Get Formulation Terms //
  formulationPtr termA = &Formulation<std::complex<double> >::weak;
  formulationPtr termB = &Formulation<std::complex<double> >::weakB;

  // Alloc Temp Sparse Matrices (not with PETSc) //
  const size_t size = dofM->getUnfixedDofNumber();

  SolverVector<complex<double> > tmpRHS(size);
  SolverMatrix<complex<double> > tmpA(size, size);
  SolverMatrix<complex<double> > tmpB(size, size);

  // Assemble Systems (tmpA and tmpB) //
  const size_t E = dofField.size();   // Should be equal to dofTest.size().?.

  #pragma omp parallel for
  for(size_t i = 0; i < E; i++)
    SystemAbstract::assemble
      (tmpA, tmpRHS, i, dofField[i], dofTest[i], termA, *formulation);

  if(general)
    #pragma omp parallel for
    for(size_t i = 0; i < E; i++)
      SystemAbstract::assemble
        (tmpB, tmpRHS, i, dofField[i], dofTest[i], termB, *formulation);

  // Copy tmpA into Assembled PETSc matrix //
  // Data
  vector<int>                   row;
  vector<int>                   col;
  vector<std::complex<double> > value;
  int                           nNZ;

  // Serialize (CStyle) tmpA & Copy
  nNZ = tmpA.serializeCStyle(row, col, value);
  A   = new Mat;

  MatCreateSeqAIJFromTriple(MPI_COMM_SELF, size, size,
                            row.data(), col.data(), value.data(),
                            A, nNZ, PETSC_FALSE);

  // Copy tmpB (CStyle) into Assembled PETSc matrix (if needed) //
  if(general){
    nNZ = tmpB.serializeCStyle(row, col, value);
    B   = new Mat;

    MatCreateSeqAIJFromTriple(MPI_COMM_SELF, size, size,
                              row.data(), col.data(), value.data(),
                              B, nNZ, PETSC_FALSE);
  }

  // The SystemEigen is assembled //
  assembled = true;
}

void SystemEigen::solve(void){
  // Check nEigenValues
  if(!nEigenValues)
    throw
      Exception("The number of eigenvalues to compute is zero");

  // Is the SystemEigen assembled ? //
  if(!assembled)
    assemble();

  // Build Solver //
  EPS solver;
  EPSCreate(MPI_COMM_SELF, &solver);

  if(general)
    EPSSetOperators(solver, *A, *B);
  else
    EPSSetOperators(solver, *A, NULL);

  if(general)
    EPSSetProblemType(solver, EPS_GNHEP);
  else
    EPSSetProblemType(solver, EPS_NHEP);

  // Set Options //
  EPSSetDimensions(solver, nEigenValues, PETSC_DECIDE, PETSC_DECIDE);
  EPSSetTolerances(solver, 1E-12, 1E6);
  EPSSetWhichEigenpairs(solver, EPS_SMALLEST_MAGNITUDE);

  // Use Krylov Schur //
  EPSSetType(solver, EPSKRYLOVSCHUR);
  /*
  // Use Generalized Davidson Solver and LU (MUMPS) preconditioning //
  KSP linSolver;
  PC  precond;
  ST  specT;

  EPSSetType(solver, "gd");

  EPSGetST(solver, &specT);
  STSetType(specT, "precond");
  STGetKSP(specT, &linSolver);

  KSPSetType(linSolver, "preonly");
  KSPGetPC(linSolver, &precond);
  PCSetType(precond, "lu");
  PCFactorSetMatSolverPackage(precond, "mumps");
  */

  // Override with PETSc Database //
  EPSSetFromOptions(solver);
  //STSetFromOptions(specT);

  // Solve //
  EPSSolve(solver);

  // Get Solution //
  const size_t size = dofM->getUnfixedDofNumber();

  PetscScalar  lambda;
  PetscScalar* x;
  Vec          xPetsc;

  MatGetVecs(*A, PETSC_NULL, &xPetsc);

  EPSGetConverged(solver, &nEigenValues);

  eigenValue  = new fullVector<complex<double> >(nEigenValues);
  eigenVector = new vector<fullVector<complex<double> > >(nEigenValues);

  for(PetscInt i = 0; i < nEigenValues; i++){
    EPSGetEigenpair(solver, i, &lambda, NULL, xPetsc, NULL);

    VecGetArray(xPetsc, &x);

    (*eigenVector)[i].resize(size);
    for(size_t j = 0; j < size; j++)
      (*eigenVector)[i](j) = x[j];

    (*eigenValue)(i) = lambda;
  }

  VecDestroy(&xPetsc);
  EPSDestroy(&solver);

  // System solved ! //
  solved = true;
}
