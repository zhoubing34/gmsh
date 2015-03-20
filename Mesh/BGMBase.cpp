#include "BGMBase.h"

#include <iostream>

#include "OS.h"
#include "GPoint.h"
#include "GFace.h"
#include "GmshDefines.h"
#include "MElementOctree.h"

void BGMBase::export_scalar(const string &filename, const DoubleStorageType &_whatToPrint) const{
//  cout << "BGMBase::size of sizeField: " << sizeField.size()  << endl;
  FILE *f = Fopen (filename.c_str(),"w");
  fprintf(f,"View \"Background Mesh\"{\n");

  const MElement *elem;
  int nvertex;
  int type;

  for(unsigned int i=0;i<getNumMeshElements();i++){
    elem = getElement(i);
    nvertex = elem->getNumVertices();
    type = elem->getType();
    const char *s = 0;
    switch(type){
      case TYPE_PNT: s = "SP"; break;
      case TYPE_LIN: s = "SL"; break;
      case TYPE_TRI: s = "ST"; break;
      case TYPE_QUA: s = "SQ"; break;
      case TYPE_TET: s = "SS"; break;
      case TYPE_HEX: s = "SH"; break;
      case TYPE_PRI: s = "SI"; break;
      case TYPE_PYR: s = "SY"; break;
      default: throw;
    }

    fprintf(f,"%s(",s);
    const MVertex *v;
    vector<double> values(nvertex);
    for (int iv=0;iv<nvertex;iv++){
      v = elem->getVertex(iv);
      values[iv] = get_nodal_value(v,_whatToPrint);
      //GPoint p = gf->point(SPoint2(v->x(),v->y()));
      GPoint p = get_GPoint_from_MVertex(v);
      fprintf(f,"%g,%g,%g",p.x(),p.y(),p.z());
      if (iv!=nvertex-1) fprintf(f,",");
      else fprintf(f,"){");
    }
    for (int iv=0;iv<nvertex;iv++){
      fprintf(f,"%g",values[iv]);
      if (iv!=nvertex-1) fprintf(f,",");
      else fprintf(f,"};\n");
    }
  }
  fprintf(f,"};\n");
  fclose(f);
//  cout << "export_scalar DONE " << endl;
}

//------------------------------------------------------------------------

void BGMBase::export_vector(const string &filename, const VectorStorageType &_whatToPrint)const{
  FILE *f = Fopen (filename.c_str(),"w");
  fprintf(f,"View \"Background Mesh\"{\n");

  const MElement *elem;
  int nvertex;
  int type;

  for(unsigned int i=0;i<getNumMeshElements();i++){
    elem = getElement(i);
    nvertex = elem->getNumVertices();
    type = elem->getType();
    const char *s = 0;
    switch(type){
      case TYPE_PNT: s = "VP"; break;
      case TYPE_LIN: s = "VL"; break;
      case TYPE_TRI: s = "VT"; break;
      case TYPE_QUA: s = "VQ"; break;
      case TYPE_TET: s = "VS"; break;
      case TYPE_HEX: s = "VH"; break;
      case TYPE_PRI: s = "VI"; break;
      case TYPE_PYR: s = "VY"; break;
      default: throw;
    }

    fprintf(f,"%s(",s);
    const MVertex *v;
    vector<double> values(nvertex*3);
    for (int iv=0;iv<nvertex;iv++){
      v = elem->getVertex(iv);
      vector<double> temp = get_nodal_value(v,_whatToPrint);
      for (int j=0;j<3;j++)
        values[iv*3+j] = temp[j];
      GPoint p = get_GPoint_from_MVertex(v);
      fprintf(f,"%g,%g,%g",p.x(),p.y(),p.z());
      if (iv!=nvertex-1) fprintf(f,",");
      else fprintf(f,"){");
    }
    for (int iv=0;iv<nvertex;iv++){
      for (int j=0;j<3;j++){
        fprintf(f,"%g",values[iv*3+j]);
        if (!((iv==nvertex-1)&&(j==2))) fprintf(f,",");
        else fprintf(f,"};\n");
      }
    }
  }
  fprintf(f,"};\n");
  fclose(f);
}

//------------------------------------------------------------------------

void BGMBase::export_tensor_as_vectors(const string &filename, const TensorStorageType &_whatToPrint)const{
  FILE *f = Fopen (filename.c_str(),"w");
  fprintf(f,"View \"Background Mesh\"{\n");

  TensorStorageType::const_iterator it = _whatToPrint.begin();
  const char *s = "VP";
  MVertex *v;
  for (;it!=_whatToPrint.end();it++){// for all vertices
    v = it->first;
    GPoint p = get_GPoint_from_MVertex(v);
    for (int i=0;i<3;i++){
      fprintf(f,"%s(%g,%g,%g){%g,%g,%g};\n",s,p.x(),p.y(),p.z(),(it->second)(0,i),(it->second)(1,i),(it->second)(2,i));
      fprintf(f,"%s(%g,%g,%g){%g,%g,%g};\n",s,p.x(),p.y(),p.z(),-(it->second)(0,i),-(it->second)(1,i),-(it->second)(2,i));
    }
  }
  fprintf(f,"};\n");
  fclose(f);
}

//------------------------------------------------------------------------

BGMBase::BGMBase(int dim,GEntity *_gf):octree(NULL),gf(_gf), DIM(dim), order(1),debug(false){
  if(debug) cout << "BGMBase::constructor " << endl;
}

//------------------------------------------------------------------------

BGMBase::~BGMBase(){
}

//------------------------------------------------------------------------

bool BGMBase::inDomain (double u, double v, double w){
  return (findElement(u, v, w) != NULL);
}

//------------------------------------------------------------------------

const MElement* BGMBase::findElement(double u, double v, double w, bool strict){
  return (getOctree()->find(u, v, w, DIM, strict));
}

//------------------------------------------------------------------------

vector<double> BGMBase::get_field_value(double u, double v, double w, const VectorStorageType &data){
  MElement *e = const_cast<MElement*>(findElement(u, v, w ));
  if (!e) return vector<double>(3,-1000.);
  vector<vector<double> > val = get_nodal_values(e,data);
  vector<double> element_uvw = get_element_uvw_from_xyz(e,u,v,w);

  vector<double> res(3);
  for (int j=0;j<3;j++){
    double values[e->getNumVertices()];
    for (int i=0;i<e->getNumVertices();i++) values[i]=val[i][j];
    res[j] = e->interpolate(values, element_uvw[0], element_uvw[1], element_uvw[2], 1, order);
  }
  return res;
}

//------------------------------------------------------------------------

double BGMBase::get_field_value(double u, double v, double w, const DoubleStorageType &data){
  MElement *e = const_cast<MElement*>(findElement(u, v, w));
  if (!e) return -1000.;
  vector<double> val = get_nodal_values(e,data);
  vector<double> element_uvw = get_element_uvw_from_xyz(e,u,v,w);
  double values[e->getNumVertices()];
  for (int i=0;i<e->getNumVertices();i++)
    values[i]=val[i];
  
  return e->interpolate(values, element_uvw[0], element_uvw[1], element_uvw[2], 1, order);
}

//------------------------------------------------------------------------

double BGMBase::size(double u, double v, double w){
  return get_field_value(u,v,w,sizeField);
}

//------------------------------------------------------------------------

double BGMBase::size(const MVertex *v){
  return get_nodal_value(v,sizeField);
}

//------------------------------------------------------------------------

vector<double> BGMBase::get_nodal_value(const MVertex *v,const VectorStorageType &data)const{
  if(debug) cout << "BGMBase::get_nodal_value(const MVertex *v,const map<MVertex*,vector<double> > &data)" << endl;
  VectorStorageType::const_iterator itfind = data.find(const_cast<MVertex*>(v));
  if (itfind==data.end()){
    cout << "WARNING: BGMBase::get_nodal_value (vector): unknown vertex ! " << v << endl;
    throw;
    return vector<double>(3,0.);
  }
  return itfind->second;
}

//------------------------------------------------------------------------

double BGMBase::get_nodal_value(const MVertex *v,const DoubleStorageType &data)const{
  if(debug) cout << "BGMBase::get_nodal_value(const MVertex *v,const map<MVertex*,double> &data)" << endl;
  DoubleStorageType::const_iterator itfind = data.find(const_cast<MVertex*>(v));
  if (itfind==data.end()){
    cout << "WARNING: BGMBase::get_nodal_value: unknown vertex ! " << endl;
    throw;
    return 0.;
  }
  return itfind->second;
}

//------------------------------------------------------------------------

vector<vector<double> > BGMBase::get_nodal_values(const MElement *e,const VectorStorageType &data)const{
  if(debug) cout << "BGMBase::get_nodal_values(const MElement *e,const map<MVertex*,vector<double> > &data)" << endl;
  vector<vector<double> > res(e->getNumVertices());

  for (int i=0;i<e->getNumVertices();i++){
    VectorStorageType::const_iterator itfind = data.find(const_cast<MVertex*>(e->getVertex(i)));
    for (int j=0;j<3;j++)
      res[i].push_back((itfind->second)[j]);
  }
  return res;
}

//------------------------------------------------------------------------

vector<double> BGMBase::get_nodal_values(const MElement *e,const DoubleStorageType &data)const{
  if(debug) cout << "BGMBase::get_nodal_values(const MElement *e,const map<MVertex*,double> &data)" << endl;
  vector<double> res(e->getNumVertices(),0.);

  for (int i=0;i<e->getNumVertices();i++)
    res[i] = (data.find(const_cast<MVertex*>(e->getVertex(i))))->second;
  return res;
}

//------------------------------------------------------------------------

vector<double> BGMBase::get_element_uvw_from_xyz (const MElement *e, double x, double y,double z) const{
  double element_uvw[3];
  double xyz[3] = {x, y, z};
  e->xyz2uvw(xyz, element_uvw);
  vector<double> res(3,0.);
  for (int i=0;i<3;i++) {
    res[i] = element_uvw[i];
  }
  return res;
}

//------------------------------------------------------------------------

set<MVertex*> BGMBase::get_vertices_of_maximum_dim(int dim){
  set<MVertex*> bnd_vertices;
  for(unsigned int i=0;i<gf->getNumMeshElements();i++){
    MElement* element = gf->getMeshElement(i);
    for(int j=0;j<element->getNumVertices();j++){
      MVertex *vertex = element->getVertex(j);
      if (vertex->onWhat()->dim() <= dim)bnd_vertices.insert(vertex);
    }
  }
  return bnd_vertices;
}

//------------------------------------------------------------------------

GEntity* BGMBase::getBackgroundGEntity(){
  return gf;
}

//------------------------------------------------------------------------



