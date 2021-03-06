#include <cmath>
#include <sstream>
#include <stack>

#include "Polynomial.h"

using namespace std;

const char Polynomial::coefName[3] = {'x', 'y', 'z'};

Polynomial::Polynomial(double coef,
                       int powerX,
                       int powerY,
                       int powerZ){
  nMon = 1;
  mon  = new monomial_t[1];

  mon[0].coef     = coef;
  mon[0].power[0] = powerX;
  mon[0].power[1] = powerY;
  mon[0].power[2] = powerZ;
}

Polynomial::Polynomial(const Polynomial& other){
  nMon = other.nMon;
  mon  = copyMonomial(other.mon, nMon);
}

Polynomial::Polynomial(void){
  nMon = 0;
  mon  = NULL;
}

Polynomial::~Polynomial(void){
  if(mon)
    delete[] mon;
}

void Polynomial::derivative(int dim){
  // Take derivative //
  for(int i = 0; i < nMon; i++){
    mon[i].coef *= mon[i].power[dim];
    mon[i].power[dim] -= 1;
  }

  // Remove zero monomials //
  int N = 0;
  stack<monomial_t*> s;

  for(int i = 0; i < nMon; i++){
    if(mon[i].coef != 0.0){
      s.push(&mon[i]);
      N++;
    }
  }

  // If no monomial any more ---> return zero polynomial
  if(!N){
    delete[] mon;

    mon  = zeroPolynomial();
    nMon = 1;
    return;
  }

  // If no zero found ---> return;
  if(N == nMon)
    return;

  // Else, remove them //
  monomial_t* tmp = mon;

  mon  = new monomial_t[N];
  nMon = N;

  for(int i = N - 1; i >= 0; i--){
    mon[i] = *(s.top());
    s.pop();
  }

  delete[] tmp;

  // Sort resulting monomial and return //
  sort(mon, nMon);

  return;
}

vector<Polynomial> Polynomial::gradient(void) const{
  vector<Polynomial> grad(3);

  // Copy Polynomial //
  grad[0] = *this;
  grad[1] = *this;
  grad[2] = *this;

  // Derivative with respect to each direction //
  grad[0].derivative(0);
  grad[1].derivative(1);
  grad[2].derivative(2);

  return grad;
}

vector<Polynomial> Polynomial::curl(const vector<Polynomial>& p){
  vector<Polynomial> rot(3);

  // Partial Derivatives //
  Polynomial dP0d1 = p[0];
  Polynomial dP0d2 = p[0];
  Polynomial dP1d0 = p[1];
  Polynomial dP1d2 = p[1];
  Polynomial dP2d0 = p[2];
  Polynomial dP2d1 = p[2];

  dP0d1.derivative(1);
  dP0d2.derivative(2);
  dP1d0.derivative(0);
  dP1d2.derivative(2);
  dP2d0.derivative(0);
  dP2d1.derivative(1);

  // Curl //
  rot[0] = dP2d1 - dP1d2;
  rot[1] = dP0d2 - dP2d0;
  rot[2] = dP1d0 - dP0d1;

  // Return //
  return rot;
}

Polynomial Polynomial::divergence(const vector<Polynomial>& p){
  // Partial Derivatives //
  Polynomial dP0d0 = p[0];
  Polynomial dP1d1 = p[1];
  Polynomial dP2d2 = p[2];

  dP0d0.derivative(0);
  dP1d1.derivative(1);
  dP2d2.derivative(2);

  // Return Div //
  return dP0d0 + dP1d1 + dP2d2;
}

double Polynomial::at
  (double x, double y, double z) const{

  double val = 0;
  for(int i = 0; i < nMon; i++){
    val += mon[i].coef * pow(x, mon[i].power[0])
                       * pow(y, mon[i].power[1])
                       * pow(z, mon[i].power[2]);
  }

  return val;
}

fullVector<double> Polynomial::at(const std::vector<Polynomial>& P,
                                  double x,
                                  double y,
                                  double z){
  fullVector<double> val(3);

  val(0) = P[0].at(x, y, z);
  val(1) = P[1].at(x, y, z);
  val(2) = P[2].at(x, y, z);

  return val;
}


Polynomial Polynomial::operator+(const Polynomial& other) const{
  Polynomial newP;

  newP.nMon = mergeMon(mon, nMon, other.mon, other.nMon, &newP.mon);

  return newP;
}

Polynomial Polynomial::operator-(const Polynomial& other) const{
  const int   otherNMon  = other.nMon;
  monomial_t* otherMinus = copyMonomial(other.mon, otherNMon);

  Polynomial  newP;

  mult(otherMinus, otherNMon, -1);
  newP.nMon = mergeMon(mon, nMon, otherMinus, otherNMon, &newP.mon);

  delete[] otherMinus;
  return newP;
}

Polynomial Polynomial::operator*(const Polynomial& other) const{
  Polynomial newP;

  newP.nMon = mult(mon, nMon, other.mon, other.nMon, &newP.mon);

  return newP;
}

Polynomial Polynomial::operator*(double alpha) const{
  Polynomial newP;

  newP.mon  = copyMonomial(mon, nMon);
  newP.nMon = nMon;

  newP.mul(alpha);

  return newP;
}


void Polynomial::add(const Polynomial& other){
  monomial_t* tmp = mon;

  nMon = mergeMon(mon, nMon, other.mon, other.nMon, &mon);

  delete[] tmp;
}

void Polynomial::sub(const Polynomial& other){
  const int   otherNMon  = other.nMon;
  monomial_t* otherMinus = copyMonomial(other.mon, otherNMon);
  monomial_t* tmp        = mon;

  mult(otherMinus, otherNMon, -1);
  nMon = mergeMon(mon, nMon, otherMinus, otherNMon, &mon);

  delete[] otherMinus;
  delete[] tmp;
}

void Polynomial::mul(const Polynomial& other){
  monomial_t* tmp = mon;

  nMon = mult(mon, nMon, other.mon, other.nMon, &mon);

  delete[] tmp;
}

void Polynomial::mul(double alpha){
  for(int i = 0; i < nMon; i++)
    mon[i].coef *= alpha;
}

void Polynomial::power(int n){
  if (n < 0)
    return;

  switch(n){
  case 0:
    delete[] mon;

    mon  = unitPolynomial();
    nMon = 1;

    break;

  case 1:
    break;

  default:
    Polynomial old = *this;

    for(int i = 1; i < n; i++)
      mul(old);

    break;
  }
}

Polynomial Polynomial::compose(const Polynomial& other) const{
  stack<monomial_t> stk;

  for(int i = 0; i < nMon; i++)
    compose(&mon[i], other, &stk);

  return polynomialFromStack(stk);
}

Polynomial Polynomial::compose(const Polynomial& otherA,
                               const Polynomial& otherB) const{
  stack<monomial_t> stk;

  for(int i = 0; i < nMon; i++)
    compose(&mon[i], otherA, otherB, &stk);

  return polynomialFromStack(stk);
}

Polynomial Polynomial::compose(const Polynomial& otherA,
                               const Polynomial& otherB,
                               const Polynomial& otherC) const{
  stack<monomial_t> stk;

  for(int i = 0; i < nMon; i++)
    compose(&mon[i], otherA, otherB, otherC, &stk);

  return polynomialFromStack(stk);
}

void Polynomial::operator=(const Polynomial& other){
  if(mon)
    delete[] mon;

  nMon = other.nMon;
  mon  = copyMonomial(other.mon, nMon);
}

string Polynomial::
toString(const Polynomial::monomial_t* mon, const bool isAbs){
  stringstream stream;
  const bool minusOne    = mon->coef == -1.0;
  const bool notUnitCoef = mon->coef !=  1.0 && !minusOne;

  // If we have a constant term
  if(!mon->power[0] && !mon->power[1] && !mon->power[2]){
    stream << mon->coef;
    return stream.str();
  }

  // If we're here, we do not have a constant term

  // If we have a coefficient of '1', we don't display it
  if(notUnitCoef && isAbs)
    stream << abs(mon->coef);

  if(notUnitCoef && !isAbs)
    stream << mon->coef;

  if(minusOne && !isAbs)
    stream << "-";

  // We look for each power
  bool notOnce = false;
  for(int i = 0; i < 3; i++){
    // If we have a non zero power, we display it
    if(mon->power[i]){
      if(notUnitCoef || notOnce)
        stream << " * ";

      stream << coefName[i];

      if(mon->power[i] != 1)
        stream << "^" << mon->power[i];

      notOnce = true;
    }
  }

  return stream.str();
}

string Polynomial::toString(void) const{
  stringstream stream;
  bool isAbs = false;

  stream << toString(&mon[0], isAbs);

  for(int i = 1; i < nMon; i++){
    if(mon[i].coef < 0.0){
      stream << " - ";
      isAbs = true;
    }

    else{
      stream << " + ";
      isAbs = false;
    }

    stream << toString(&mon[i], isAbs);
  }

  return stream.str();
}

bool Polynomial::isSmaller(const Polynomial::monomial_t* a,
                           const Polynomial::monomial_t* b){
  // GRevLex order:
  // http://www.math.uiuc.edu/Macaulay2/doc/Macaulay2-1.4/share/doc/Macaulay2/Macaulay2Doc/html/___G__Rev__Lex.html

  int dif[3];
  int last = 0;

  if(isSmallerPower(a, b))
    return true;

  if(isEqualPower(a, b)){
    for(int i = 0, j = 2; i < 3; i++, j--)
      dif[i] = b->power[j] - a->power[j];

    for(int i = 0; i < 3; i++)
      if(dif[i])
        last = dif[i];

    if(last < 0)
      return true;

    else
      return false;
  }

  return false;
}

void Polynomial::sort(monomial_t* mon, int size){
  for(int i = 0; i < size; i++)
    for(int j = i; j < size; j++)
      if(isSmaller(&mon[j], &mon[i]))
        swap(mon, i, j);
}

void Polynomial::swap(monomial_t* mon, int i, int j){
  monomial_t tmp = mon[i];
  mon[i] = mon[j];
  mon[j] = tmp;
}


int Polynomial::mergeMon(monomial_t* sourceA, int sizeA,
                         monomial_t* sourceB, int sizeB,
                         monomial_t** dest){
  stack<monomial_t> s;
  monomial_t tmp;

  int i = 0;
  int j = 0;
  int N = 0;

  while(i < sizeA && j < sizeB){
    if(sourceA[i].coef == 0.0)
      i++;

    else if(sourceB[j].coef == 0.0)
      j++;

    else if(isEqual(&sourceA[i], &sourceB[j])){
      tmp       = sourceA[i];
      tmp.coef += sourceB[j].coef;

      if(tmp.coef != 0.0){
        s.push(tmp);
        N++;
      }

      i++;
      j++;
    }

    else if(isSmaller(&sourceA[i], &sourceB[j])){
      s.push(sourceA[i]);
      i++;
      N++;
    }

    else{
      s.push(sourceB[j]);
      j++;
      N++;
    }
  }

  while(i == sizeA && j < sizeB){
    s.push(sourceB[j]);
    j++;
    N++;
  }

  while(i < sizeA && j == sizeB){
    s.push(sourceA[i]);
    i++;
    N++;
  }

  if(!N){
    *dest = zeroPolynomial();
    N++;
  }

  else{
    *dest = new monomial_t[N];

    for(int k = N - 1; k >= 0; k--){
      (*dest)[k] = s.top();
      s.pop();
    }
  }

  return N;
}

int Polynomial::mult(const monomial_t* sourceA, int sizeA,
                     const monomial_t* sourceB, int sizeB,
                     monomial_t** dest){

  const monomial_t* a; // smaller polynomial
  const monomial_t* b; // bigger polynomial
  int nDist;
  int size;

  if(sizeA < sizeB){
    a     = sourceA;
    b     = sourceB;
    nDist = sizeA;
    size  = sizeB;
  }

  else{
    a     = sourceB;
    b     = sourceA;
    nDist = sizeB;
    size  = sizeA;
  }

  // Check if zero //
  if(a[0].coef == 0 || b[0].coef == 0){
    *dest = zeroPolynomial();
    return 1;
  }

  // Distrubute all monomials //
  monomial_t** dist = new monomial_t*[nDist];

  for(int i = 0; i < nDist; i++){
    dist[i] = copyMonomial(b, size);

    distribute(dist[i], size, &a[i]);
  }

  // Merge //
  int finalSize    = size;
  int nDistMinus   = nDist - 1;
  monomial_t** tmp = new monomial_t*[nDistMinus]; // Temp array for all dist[0];

  for(int i = 1, j = 0; i < nDist; i++, j++){
    tmp[j] = dist[0];

    finalSize = mergeMon(dist[0], finalSize,
                         dist[i], size,
                         &dist[0]);
  }

  // Keep distributed polynomial //
  *dest = dist[0];

  // Free Temporary Resources and Return //
  for(int i = 1, j = 0; i < nDist; i++, j++){
    delete[] dist[i];
    delete[] tmp[j];
  }

  delete[] dist;
  delete[] tmp;

  return finalSize;
}

void Polynomial::mult(monomial_t* source, int size, double alpha){
  for(int i = 0; i < size; i++)
    source[i].coef *= alpha;
}


void Polynomial::distribute(monomial_t* src, int size, const monomial_t* m){
  for(int i = 0; i < size; i++){
    src[i].coef *= m->coef;

    src[i].power[0] += m->power[0];
    src[i].power[1] += m->power[1];
    src[i].power[2] += m->power[2];
  }
}

void Polynomial::compose(const monomial_t* src,
                         Polynomial comp,
                         std::stack<monomial_t>* stk){

  comp.power(src->power[0]);
  comp.mul(src->coef);

  const int size = comp.nMon;

  for(int i = 0; i < size; i++){
    if(comp.mon[i].coef != 0){

      comp.mon[i].power[1] += src->power[1];
      comp.mon[i].power[2] += src->power[2];

      stk->push(comp.mon[i]);
    }
  }
}

void Polynomial::compose(const monomial_t* src,
                         Polynomial compA, Polynomial compB,
                         std::stack<monomial_t>* stk){

  compA.power(src->power[0]);
  compB.power(src->power[1]);

  compA.mul(compB);
  compA.mul(src->coef);

  const int size = compA.nMon;

  for(int i = 0; i < size; i++){
    if(compA.mon[i].coef != 0){

      compA.mon[i].power[2] += src->power[2];

      stk->push(compA.mon[i]);
    }
  }
}

void Polynomial::compose(const monomial_t* src,
                         Polynomial compA, Polynomial compB, Polynomial compC,
                         std::stack<monomial_t>* stk){

  compA.power(src->power[0]);
  compB.power(src->power[1]);
  compC.power(src->power[2]);

  compA.mul(compB);
  compA.mul(compC);
  compA.mul(src->coef);

  const int size = compA.nMon;

  for(int i = 0; i < size; i++){
    if(compA.mon[i].coef != 0)
      stk->push(compA.mon[i]);
  }
}

Polynomial Polynomial::
polynomialFromStack(std::stack<Polynomial::monomial_t>& stk){
  Polynomial  newP;
  monomial_t* tmp;
  monomial_t* newMon;
  int         newNMon;

  if(!stk.size()){
    newMon  = zeroPolynomial();
    newNMon = 1;
  }

  else{
    newMon    = new monomial_t[1];
    newMon[0] = stk.top();
    newNMon   = 1;
    stk.pop();

    while(!stk.empty()){
      tmp     = newMon;
      newNMon = mergeMon(newMon, newNMon, &stk.top(), 1, &newMon);
      stk.pop();

      delete[] tmp;
    }
  }

  newP.nMon = newNMon;
  newP.mon  = newMon;

  return newP;
}

Polynomial::monomial_t* Polynomial::copyMonomial(const monomial_t* src,
                                                 int size){
  monomial_t* dest = new monomial_t[size];

  for(int i = 0; i < size; i++){
    dest[i].coef     = src[i].coef;
    dest[i].power[0] = src[i].power[0];
    dest[i].power[1] = src[i].power[1];
    dest[i].power[2] = src[i].power[2];
  }

  return dest;
}

Polynomial::monomial_t* Polynomial::zeroPolynomial(void){
  monomial_t* zero = new monomial_t[1];

  zero->coef     = 0;
  zero->power[0] = 0;
  zero->power[1] = 0;
  zero->power[2] = 0;

  return zero;
}

Polynomial::monomial_t* Polynomial::unitPolynomial(void){
  monomial_t* unit = new monomial_t[1];

  unit->coef     = 1;
  unit->power[0] = 0;
  unit->power[1] = 0;
  unit->power[2] = 0;

  return unit;
}
