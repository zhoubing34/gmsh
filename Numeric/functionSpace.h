// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _FUNCTION_SPACE_H_
#define _FUNCTION_SPACE_H_

#include <math.h>
#include <map>
#include <vector>
#include "fullMatrix.h"

// presently thos function spaces are only for simplices
// should be extended to other elements like quads and hexes
struct functionSpace 
{
  typedef  std::vector<std::vector<int> > clCont;
  clCont faceClosure;
  clCont edgeClosure;
  fullMatrix<double> points;
  fullMatrix<double> monomials;
  fullMatrix<double> coefficients;
  // for a given face/edge, with both a sign and a rotation,
  // give an ordered list of nodes on this face/edge
  std::vector<int> & getFaceClosure (int iFace, int iSign, int iRot){
    return faceClosure[iFace+4*(iSign==1?0:1)+8*iRot];
  }
  inline std::vector<int> & getEdgeClosure (int iEdge, int iSign){
    return edgeClosure[iSign == 1 ? iEdge : 3+iEdge];
  }
  inline void evaluateMonomials(double u, double v, double w, double p[]) const 
  {
    for (int j = 0; j < monomials.size1(); j++) {
      p[j] = pow(u, (int)monomials(j, 0));
      if (monomials.size2() > 1) p[j] *= pow(v, (int)monomials(j, 1));
      if (monomials.size2() > 2) p[j] *= pow(w, (int)monomials(j, 2));
    }
  }
  inline void f(double u, double v, double w, double *sf) const
  {
    double p[256];
    evaluateMonomials(u, v, w, p);
    for (int i = 0; i < coefficients.size1(); i++) {
      sf[i] = 0;
      for (int j = 0; j < coefficients.size2(); j++) {
        sf[i] += coefficients(i, j) * p[j];
      }
    }
  }
  inline void df(double u, double v, double w, double grads[][3]) const
  {
    switch (monomials.size2()) {
    case 1:
      for (int i = 0; i < coefficients.size1(); i++){
        grads[i][0] = 0;
        grads[i][1] = 0;
        grads[i][2] = 0;
        for(int j = 0; j < coefficients.size2(); j++){
          if ((monomials)(j, 0) > 0)
            grads[i][0] += (coefficients)(i, j) * 
              pow(u, (monomials)(j, 0) - 1) * (monomials)(j, 0);
        }
      }
      break;
    case 2:
      for (int i = 0; i < coefficients.size1(); i++){
        grads[i][0] = 0;
        grads[i][1] = 0;
        grads[i][2] = 0;
        for(int j = 0; j < coefficients.size2(); j++){
          if ((monomials)(j, 0) > 0)
            grads[i][0] += (coefficients)(i, j) *
              pow(u, (monomials)(j, 0) - 1) * (monomials)(j, 0) *
              pow(v, (monomials)(j, 1));
          if ((monomials)(j, 1) > 0)
            grads[i][1] += (coefficients)(i, j) *
              pow(u, (monomials)(j, 0)) *
              pow(v, (monomials)(j, 1) - 1) * (monomials)(j, 1);
        }
      }
      break;
    case 3:
      for (int i = 0; i < coefficients.size1(); i++){
        grads[i][0] = 0;
        grads[i][1] = 0;
        grads[i][2] = 0;
        for(int j = 0; j < coefficients.size2(); j++){
          if ((monomials)(j, 0) > 0)
            grads[i][0] += (coefficients)(i, j) *
              pow(u, (monomials)(j, 0) - 1) * (monomials)(j, 0) *
              pow(v, (monomials)(j, 1)) *
              pow(w, (monomials)(j, 2));
          if ((monomials)(j, 1) > 0)
            grads[i][1] += (coefficients)(i, j) *
              pow(u, (monomials)(j, 0)) *
              pow(v, (monomials)(j, 1) - 1) * (monomials)(j, 1) *
              pow(w, (monomials)(j, 2));
          if ((monomials)(j, 2) > 0)
            grads[i][2] += (coefficients)(i, j) *
              pow(u, (monomials)(j, 0)) *
              pow(v, (monomials)(j, 1)) *
              pow(w, (monomials)(j, 2) - 1) * (monomials)(j, 2);
        }
      }
      break;
    }
  }
};

class functionSpaces 
{
 private:
  static std::map<int, functionSpace> fs;
  static std::map<std::pair<int, int>, fullMatrix<double> > injector;
 public :
  static const functionSpace &find(int);
  static const fullMatrix<double> &findInjector(int, int);
};

#endif
