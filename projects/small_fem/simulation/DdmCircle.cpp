#include "SmallFem.h"
#include "SolverDDM.h"
#include "MPIOStream.h"

#include "DDMContextEMDA.h"
#include "DDMContextOO2.h"
#include "DDMContextJFLee.h"
#include "DDMContextOSRCScalar.h"
#include "DDMContextOSRCVector.h"

#include "System.h"
#include "SystemHelper.h"
#include "Interpolator.h"
#include "NodeSolution.h"
#include "FormulationHelper.h"

#include "FormulationOO2.h"
#include "FormulationEMDA.h"
#include "FormulationJFLee.h"
#include "FormulationOSRCScalar.h"
#include "FormulationOSRCVector.h"

#include "FormulationDummy.h"
#include "FormulationSilverMuller.h"
#include "FormulationSteadyWave.h"

#include "FormulationUpdateEMDA.h"
#include "FormulationUpdateOO2.h"
#include "FormulationUpdateJFLee.h"
#include "FormulationUpdateOSRCScalar.h"
#include "FormulationUpdateOSRCVector.h"

#include <cmath>
#include <iostream>

using namespace std;

static const int    scal  = 0;
static const int    vect  = 1;
static       double k;

Complex fSourceScal(fullVector<double>& xyz){
  return Complex(1, 0) * Complex(cos(k * xyz(0)), sin(k * xyz(0)));
}

fullVector<Complex> fSourceVect(fullVector<double>& xyz){
  fullVector<Complex> tmp(3);

  tmp(0) = Complex(0, 0);
  tmp(1) = Complex(1, 0) * Complex(cos(k * xyz(0)), sin(k * xyz(0)));
  tmp(2) = Complex(0, 0);

  return tmp;
}

void compute(const Options& option){
  // MPI //
  int nProcs;
  int myProc;
  MPIOStream cout(0, std::cout);

  MPI_Comm_size(MPI_COMM_WORLD,&nProcs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myProc);

 // Get Type //
  int type;
  if(option.getValue("-type")[1].compare("scalar") == 0)
    type = scal;

  else if(option.getValue("-type")[1].compare("vector") == 0)
    type = vect;

  else
    throw Exception("Bad -type: %s", option.getValue("-type")[1].c_str());

  // Get Parameters //
  const string ddmType  = option.getValue("-ddm")[1];
               k        = atof(option.getValue("-k")[1].c_str());
  const size_t orderVol = atoi(option.getValue("-ov")[1].c_str());
  const size_t orderSur = atoi(option.getValue("-ob")[1].c_str());
  const size_t maxIt    = atoi(option.getValue("-max")[1].c_str());

  // DDM Formulations //
  const string emdaType("emda");
  const string oo2Type("oo2");
  const string osrcType("osrc");
  const string jflType("jfl");

  // Variables
  const double Pi = atan(1.0) * 4;
  double lc       = 0;
  double chi      = 0;
  Complex ooA     = 0;
  Complex ooB     = 0;
  int NPade       = 0;
  Complex keps    = 0;

  // EMDA Stuff
  if(ddmType == emdaType)
    chi = atof(option.getValue("-chi")[1].c_str()) * k;

  // OO2 Stuff
  else if(ddmType == oo2Type){
    lc = atof(option.getValue("-lc")[1].c_str());

    double ooXsiMin = 0;
    double ooXsiMax = Pi / lc;
    double ooDeltaK = Pi / .06;

    double tmp0 =
      (k * k - ooXsiMin * ooXsiMin) * (k * k - (k - ooDeltaK) * (k - ooDeltaK));

    double tmp1 =
      (ooXsiMax * ooXsiMax - k * k) * ((k + ooDeltaK) * (k + ooDeltaK) - k * k);

    Complex ooAlpha = pow(Complex(tmp0, 0), 0.25) * Complex(0, 1);
    Complex ooBeta  = pow(Complex(tmp1, 0), 0.25);

    ooA = -(ooAlpha * ooBeta - k * k) / (ooAlpha + ooBeta);
    ooB = Complex(-1, 0) / (ooAlpha + ooBeta);
  }

  // OSRC Stuff
  else if(ddmType == osrcType){
    double ck = atof(option.getValue("-ck")[1].c_str());
    NPade     = atoi(option.getValue("-pade")[1].c_str());
    keps      = k + Complex(0, k * ck);
  }

  // Jin Fa Lee Stuff
  else if(ddmType == jflType){
    lc = atof(option.getValue("-lc")[1].c_str());
  }

  // Unknown Stuff
  else
    throw Exception("DDM Circle: Formulation %s is not known", ddmType.c_str());

  // Get Domains //
  Mesh msh(option.getValue("-msh")[1]);
  GroupOfElement volume(msh);
  GroupOfElement source(msh);
  GroupOfElement infinity(msh);
  GroupOfElement ddmBorder(msh);

  // Source
  if(myProc == 0)
    source.add(msh.getFromPhysical(1000));

  // Infinity
  if(myProc == nProcs - 1)
    infinity.add(msh.getFromPhysical(2000 + nProcs - 1));

  // Volume
  volume.add(msh.getFromPhysical(100 + myProc));

  // DDM border
  if(myProc > 0)
    ddmBorder.add(msh.getFromPhysical(4000 + myProc - 1));

  if(myProc < nProcs - 1)
    ddmBorder.add(msh.getFromPhysical(4000 + myProc));

  // Full Domain //
  vector<const GroupOfElement*> domain(4);
  domain[0] = &volume;
  domain[1] = &source;
  domain[2] = &infinity;
  domain[3] = &ddmBorder;

  // Dirichlet Border //
  vector<const GroupOfElement*> dirichlet(1);
  dirichlet[0] = &source;

  // Function Space //
  FunctionSpace* fs = NULL;
  FunctionSpace* fG = NULL;

  if(type == scal){
    fs = new FunctionSpaceScalar(domain,    orderVol);
    fG = new FunctionSpaceScalar(ddmBorder, orderSur);
  }

  else{
    fs = new FunctionSpaceVector(domain,    orderVol);
    fG = new FunctionSpaceVector(ddmBorder, orderSur);
  }

  // OSRC
  vector<const FunctionSpaceScalar*> OSRCScalPhi;
  vector<const FunctionSpaceVector*> OSRCVectPhi;
  vector<const FunctionSpaceScalar*> OSRCVectRho;
  FunctionSpaceVector*               OSRCVectR = NULL;

  if(ddmType == osrcType && type == scal){
    OSRCScalPhi.resize(NPade);

    for(int j = 0; j < NPade; j++)
      OSRCScalPhi[j] = new FunctionSpaceScalar(ddmBorder, orderVol);
  }

  if(ddmType == osrcType && type == vect){
    OSRCVectPhi.resize(NPade);
    OSRCVectRho.resize(NPade);

    for(int j = 0; j < NPade; j++)
      OSRCVectPhi[j] = new FunctionSpaceVector(ddmBorder, orderVol);

    if(orderVol == 0)
      for(int j = 0; j < NPade; j++)
        OSRCVectRho[j] = new FunctionSpaceScalar(ddmBorder, 1);
    else
      for(int j = 0; j < NPade; j++)
        OSRCVectRho[j] = new FunctionSpaceScalar(ddmBorder, orderVol);

    OSRCVectR = new FunctionSpaceVector(ddmBorder, orderVol);
  }

  // Jin Fa Lee
  FunctionSpaceVector* JFPhi = NULL;
  FunctionSpaceScalar* JFRho = NULL;

  if(ddmType == jflType){
    JFPhi = new FunctionSpaceVector(ddmBorder, orderVol);

    if(orderVol == 0)
      JFRho = new FunctionSpaceScalar(ddmBorder, 1);
    else
      JFRho = new FunctionSpaceScalar(ddmBorder, orderVol);
  }

  // Steady Wave Formulation //
  FormulationSteadyWave<Complex> wave(volume, *fs, k);
  Formulation<Complex>*          silverMuller;

  if(myProc == nProcs - 1)
    silverMuller = new FormulationSilverMuller(infinity, *fs, k);
  else
    silverMuller = new FormulationDummy<Complex>;

  // DDM Solution Map //
  map<Dof, Complex> ddmG;
  map<Dof, Complex> rhsG;
  FormulationHelper::initDofMap(*fG, ddmBorder, ddmG);
  FormulationHelper::initDofMap(*fG, ddmBorder, rhsG);

  // Ddm Formulation //
  DDMContext*         context = NULL;
  Formulation<Complex>*   ddm = NULL;
  Formulation<Complex>* upDdm = NULL;

  if(ddmType == emdaType){
    context = new DDMContextEMDA(ddmBorder, dirichlet, *fs, *fG, k, chi);
    context->setDDMDofs(ddmG);

    ddm     = new FormulationEMDA(static_cast<DDMContextEMDA&>(*context));
    upDdm   = new FormulationUpdateEMDA(static_cast<DDMContextEMDA&>(*context));
  }

  else if(ddmType == oo2Type){
    context = new DDMContextOO2(ddmBorder, dirichlet, *fs, *fG, ooA, ooB);
    context->setDDMDofs(ddmG);

    ddm     = new FormulationOO2(static_cast<DDMContextOO2&>(*context));
    upDdm   = new FormulationUpdateOO2(static_cast<DDMContextOO2&>(*context));
  }

  else if(ddmType == osrcType && type == scal){
    context = new DDMContextOSRCScalar
                                  (ddmBorder, dirichlet, *fs, *fG,
                                   OSRCScalPhi, k, keps, NPade, M_PI / 4.);
    context->setDDMDofs(ddmG);

    ddm     = new FormulationOSRCScalar
                                 (static_cast<DDMContextOSRCScalar&>(*context));
    upDdm   = new FormulationUpdateOSRCScalar
                                 (static_cast<DDMContextOSRCScalar&>(*context));
  }

  else if(ddmType == osrcType && type == vect){
    context = new DDMContextOSRCVector
                                  (ddmBorder, dirichlet, *fs, *fG,
                                   OSRCVectPhi, OSRCVectRho, *OSRCVectR,
                                   k, keps, NPade, M_PI / 2.);
    context->setDDMDofs(ddmG);

    ddm   = new FormulationOSRCVector
                                 (static_cast<DDMContextOSRCVector&>(*context));
    upDdm = new FormulationUpdateOSRCVector
                                 (static_cast<DDMContextOSRCVector&>(*context));
  }

  else if(ddmType == jflType){
    context = new DDMContextJFLee(ddmBorder, dirichlet, *fs, *fG,
                                  *JFPhi, *JFRho, k, lc);
    context->setDDMDofs(ddmG);

    ddm   = new FormulationJFLee(static_cast<DDMContextJFLee&>(*context));
    upDdm = new FormulationUpdateJFLee(static_cast<DDMContextJFLee&>(*context));
  }

  else
    throw Exception("Unknown %s DDM border term", ddmType.c_str());

  // Solve Non homogenous problem //
  cout << "Solving non homogenous problem" << endl << flush;

  System<Complex>* nonHomogenous = new System<Complex>;
  nonHomogenous->addFormulation(wave);
  nonHomogenous->addFormulation(*silverMuller);
  nonHomogenous->addFormulation(*ddm);

  // Constraint
  if(fs->isScalar())
    SystemHelper<Complex>::dirichlet(*nonHomogenous, *fs, source, fSourceScal);
  else
    SystemHelper<Complex>::dirichlet(*nonHomogenous, *fs, source, fSourceVect);

  // Assemble & Solve
  nonHomogenous->assemble();
  nonHomogenous->solve();

  // Solve Non homogenous DDM problem //
  cout << "Computing right hand side" << endl << flush;

  context->setSystem(*nonHomogenous);
  upDdm->update(); // update volume solution (at DDM border)

  System<Complex>* nonHomogenousDDM = new System<Complex>;
  nonHomogenousDDM->addFormulation(*upDdm);

  nonHomogenousDDM->assemble();
  nonHomogenousDDM->solve();
  nonHomogenousDDM->getSolution(rhsG, 0);

  // Clear Systems //
  delete nonHomogenous;
  delete nonHomogenousDDM;

  // DDM Solver //
  cout << "Solving DDM problem" << endl << flush;

  SolverDDM* solver =
    new SolverDDM(wave, *silverMuller, *context, *ddm, *upDdm, rhsG);

  // Solve
  solver->setMaximumIteration(maxIt);
  solver->setRestart(maxIt); // No restart!
  cout << " ! Warning: no restart ! " << endl;
  solver->solve();

  // Get Solution
  solver->getSolution(ddmG);
  context->setDDMDofs(ddmG);

  // Get history
  vector<double> history;
  solver->getHistory(history);

  // Clear DDM //
  delete solver;

  // Full Problem //
  cout << "Solving full problem" << endl << flush;
  ddm->update();

  System<Complex> full;
  full.addFormulation(wave);
  full.addFormulation(*silverMuller);
  full.addFormulation(*ddm);

  // Constraint
  if(fs->isScalar())
    SystemHelper<Complex>::dirichlet(full, *fs, source, fSourceScal);
  else
    SystemHelper<Complex>::dirichlet(full, *fs, source, fSourceVect);

  full.assemble();
  full.solve();

  // Draw Solution //
  try{
    option.getValue("-nopos");
  }
  catch(...){
    cout << "Writing full problem" << endl << flush;

    stringstream stream;
    try{
      vector<string> name = option.getValue("-name");
      stream << name[1] << myProc;
    }
    catch(...){
      stream << "circle" << myProc;
    }

    try{
      // Get Visu Mesh //
      vector<string> visuStr = option.getValue("-interp");
      Mesh           visuMsh(visuStr[1]);
      GroupOfElement visuGoe(visuMsh.getFromPhysical(100 + myProc));

      // Solution //
      map<Dof, Complex> sol;

      FormulationHelper::initDofMap(*fs, volume, sol);
      full.getSolution(sol, 0);

      // Vertex, Value Map //
      map<const MVertex*, vector<Complex> > map;
      Interpolator<Complex>::interpolate(volume, visuGoe, *fs, sol, map);

      // Print //
      stringstream name;
      name << stream.str() << ".dat";

      Interpolator<Complex>::write(name.str(), map);
      /*
      NodeSolution<Complex> nodeSol;
      nodeSol.addNodeValue(0, 0, visuMsh, map);
      nodeSol.write(stream.str());
      */
    }

    catch(...){
      FEMSolution<Complex> feSol;
      full.getSolution(feSol, *fs, volume);

      feSol.setSaveMesh(false);
      feSol.setBinaryFormat(true);
      feSol.setParition(myProc + 1);
      feSol.write("ddmCircle");
    }
  }

  // Dump history //
  if(myProc == 0){
    try{
      const size_t nHist      = history.size();
      vector<string> dumpName = option.getValue("-hist");

      // If no name given, dumb on cout
      if(dumpName.size() == 1){
        for(size_t i = 0; i < nHist; i++)
          cout << std::scientific << i << ": " << history[i] << endl;
      }

      else{
        ofstream file;
        file.open(dumpName[1].c_str(), ofstream::out | ofstream::trunc);

        for(size_t i = 0; i < nHist; i++)
          file << std::scientific << std::setprecision(16)
               << history[i] << endl;

        file.close();
      }

    }
    catch(...){
    }
  }

  // Clean //
  delete ddm;
  delete upDdm;
  delete context;
  delete silverMuller;
  delete fs;
  delete fG;

  if(JFPhi)
    delete JFPhi;

  if(JFRho)
    delete JFRho;

  if((int)(OSRCScalPhi.size()) == NPade)
    for(int j = 0; j < NPade; j++)
      delete OSRCScalPhi[j];

  if((int)(OSRCVectPhi.size()) == NPade)
    for(int j = 0; j < NPade; j++)
      delete OSRCVectPhi[j];

  if((int)(OSRCVectRho.size()) == NPade)
    for(int j = 0; j < NPade; j++)
      delete OSRCVectRho[j];

  if(OSRCVectR)
    delete OSRCVectR;
}

int main(int argc, char** argv){
  // Init SmallFem //
  SmallFem::Keywords("-msh,-ov,-ob,-k,-type,-max,-ddm,-chi,-lc,-ck,-pade,"
                     "-interp,-hist,-name,-nopos");
  SmallFem::Initialize(argc, argv);

  compute(SmallFem::getOptions());

  SmallFem::Finalize();
  return 0;
}
