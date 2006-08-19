// $Id: GModelIO.cpp,v 1.26 2006-08-19 04:24:03 geuzaine Exp $
//
// Copyright (C) 1997-2006 C. Geuzaine, J.-F. Remacle
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
// 
// Please report all bugs and problems to <gmsh@geuz.org>.

#include <map>
#include <string>

#include "Message.h"
#include "GmshDefines.h"
#include "gmshRegion.h"
#include "gmshFace.h"
#include "gmshEdge.h"
#include "MElement.h"
#include "SBoundingBox3d.h"

template<class T>
void addElements(std::vector<T*> &dst, const std::vector<MElement*> &src)
{
  for(unsigned int i = 0; i < src.size(); i++) dst.push_back((T*)src[i]);
}

static void storeElementsInEntities(GModel *m, 
				    std::map<int, std::vector<MElement*> > &map)
{
  std::map<int, std::vector<MElement*> >::const_iterator it = map.begin();
  std::map<int, std::vector<MElement*> >::const_iterator ite = map.end();
  for(; it != ite; ++it){
    if(!it->second.size()) continue;
    int numEdges = it->second[0]->getNumEdges();
    switch(numEdges){
    case 1: 
      {
	GEdge *e = m->edgeByTag(it->first);
	if(!e){
	  e = new gmshEdge(m, it->first);
	  m->add(e);
	}
	addElements(e->lines, it->second);
      }
      break;
    case 3: case 4: 
      {
	GFace *f = m->faceByTag(it->first);
	if(!f){
	  f = new gmshFace(m, it->first);
	  m->add(f);
	}
	if(numEdges == 3) addElements(f->triangles, it->second);
	else addElements(f->quadrangles, it->second);
      }
      break;
    case 6: case 12: case 9: case 8:
      {
	GRegion *r = m->regionByTag(it->first);
	if(!r){
	  r = new gmshRegion(m, it->first);
	  m->add(r);
	}
	if(numEdges == 6) addElements(r->tetrahedra, it->second);
	else if(numEdges == 12) addElements(r->hexahedra, it->second);
	else if(numEdges == 9) addElements(r->prisms, it->second);
	else addElements(r->pyramids, it->second);
      }
      break;
    }
  }
}

template<class T>
static void associateEntityWithVertices(GEntity *ge, std::vector<T*> &elements)
{
  for(unsigned int i = 0; i < elements.size(); i++)
    for(int j = 0; j < elements[i]->getNumVertices(); j++)
      elements[i]->getVertex(j)->setEntity(ge);
}

static void associateEntityWithVertices(GModel *m)
{
  // loop on regions, then on faces, edges and vertices and store the
  // entity pointer in the the elements' vertices (this way we
  // associate the entity of lowest geometrical degree with each
  // vertex)
  for(GModel::riter it = m->firstRegion(); it != m->lastRegion(); ++it){
    associateEntityWithVertices(*it, (*it)->tetrahedra);
    associateEntityWithVertices(*it, (*it)->hexahedra);
    associateEntityWithVertices(*it, (*it)->prisms);
    associateEntityWithVertices(*it, (*it)->pyramids);
  }
  for(GModel::fiter it = m->firstFace(); it != m->lastFace(); ++it){
    associateEntityWithVertices(*it, (*it)->triangles);
    associateEntityWithVertices(*it, (*it)->quadrangles);
  }
  for(GModel::eiter it = m->firstEdge(); it != m->lastEdge(); ++it){
    associateEntityWithVertices(*it, (*it)->lines);
  }
  for(GModel::viter it = m->firstVertex(); it != m->lastVertex(); ++it){
    (*it)->mesh_vertices[0]->setEntity(*it);
  }
}

static void storeVerticesInEntities(std::map<int, MVertex*> &vertices)
{
  std::map<int, MVertex*>::const_iterator it = vertices.begin();
  std::map<int, MVertex*>::const_iterator ite = vertices.end();
  for(; it != ite; ++it){
    MVertex *v = it->second;
    GEntity *ge = v->onWhat();
    if(ge) 
      ge->mesh_vertices.push_back(v);
    else
      delete v; // we delete all unused vertices
  }
}

static void storeVerticesInEntities(std::vector<MVertex*> &vertices)
{
  for(unsigned int i = 0; i < vertices.size(); i++){
    MVertex *v = vertices[i];
    GEntity *ge = v->onWhat();
    if(ge) 
      ge->mesh_vertices.push_back(v);
    else
      delete v; // we delete all unused vertices
  }
}

static void storePhysicalTagsInEntities(GModel *m, int dim,
					std::map<int, std::map<int, std::string> > &map)
{
  std::map<int, std::map<int, std::string> >::const_iterator it = map.begin();
  std::map<int, std::map<int, std::string> >::const_iterator ite = map.end();
  for(; it != ite; ++it){
    GEntity *ge = 0;
    switch(dim){
    case 0: ge = m->vertexByTag(it->first); break;
    case 1: ge = m->edgeByTag(it->first); break;
    case 2: ge = m->faceByTag(it->first); break;
    case 3: ge = m->regionByTag(it->first); break;
    }
    if(ge){
      std::map<int, std::string>::const_iterator it2 = it->second.begin();
      std::map<int, std::string>::const_iterator ite2 = it->second.end();
      for(; it2 != ite2; ++it2)
	ge->physicals.push_back(it2->first);
    }
  }
}

static int getNumVerticesForElementTypeMSH(int type)
{
  switch (type) {
  case PNT : return 1;
  case LGN1: return 2;
  case LGN2: return 2 + 1;
  case TRI1: return 3;
  case TRI2: return 3 + 3;
  case QUA1: return 4;
  case QUA2: return 4 + 4 + 1;
  case TET1: return 4;
  case TET2: return 4 + 6;
  case HEX1: return 8;
  case HEX2: return 8 + 12 + 6 + 1;
  case PRI1: return 6;
  case PRI2: return 6 + 9 + 3;
  case PYR1: return 5;
  case PYR2: return 5 + 8 + 1;
  default: return 0;
  }
}

int GModel::readMSH(const std::string &name)
{
  FILE *fp = fopen(name.c_str(), "r");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  double version = 1.0;
  char str[256];
  std::map<int, MVertex*> vertices;
  std::map<int, std::vector<MVertex*> > points;
  std::map<int, std::vector<MElement*> > elements[7];
  std::map<int, std::map<int, std::string> > physicals[4];

  while(1) {

    do {
      if(!fgets(str, sizeof(str), fp) || feof(fp))
        break;
    } while(str[0] != '$');

    if(feof(fp))
      break;

    if(!strncmp(&str[1], "MeshFormat", 10)) {

      int format, size;
      fscanf(fp, "%lf %d %d\n", &version, &format, &size);

    }
    else if(!strncmp(&str[1], "NO", 2) || !strncmp(&str[1], "Nodes", 5)) {

      int numVertices;
      fscanf(fp, "%d", &numVertices);
      Msg(INFO, "%d vertices", numVertices);

      int progress = (numVertices > 100000) ? numVertices / 25 : 0;
      for(int i = 0; i < numVertices; i++) {
	int num;
	double x, y, z;
        fscanf(fp, "%d %lf %lf %lf", &num, &x, &y, &z);
	if(vertices.count(num))
	  Msg(WARNING, "Skipping duplicate vertex %d", num);
	else
	  vertices[num] = new MVertex(x, y, z);
	if(progress && (i % progress == progress - 1))
	  Msg(PROGRESS, "Read %d vertices", i + 1);
      }
      Msg(PROGRESS, "");

    }
    else if(!strncmp(&str[1], "ELM", 3) || !strncmp(&str[1], "Elements", 8)) {

      int numElements;
      fscanf(fp, "%d", &numElements);
      Msg(INFO, "%d elements", numElements);

      int progress = (numElements > 100000) ? numElements / 25 : 0;
      for(int i = 0; i < numElements; i++) {
	int num, type, physical = 0, elementary = 0, partition = 0, numVertices;
	if(version <= 1.0){
	  fscanf(fp, "%d %d %d %d %d", &num, &type, &physical, &elementary, &numVertices);
	  int check = getNumVerticesForElementTypeMSH(type);
	  if(!check){
	    Msg(GERROR, "Unknown type for element %d", num); 
	    continue;
	  }
	  if(numVertices != check){
	    Msg(GERROR, "Wrong number of vertices (%d) for element %d", numVertices, num);
	    continue;
	  }
	}
	else{
	  int numTags;
	  fscanf(fp, "%d %d %d", &num, &type, &numTags);
	  for(int j = 0; j < numTags; j++){
	    int tag;
	    fscanf(fp, "%d", &tag);	    
	    if(j == 0)      physical = tag;
	    else if(j == 1) elementary = tag;
	    else if(j == 2) partition = tag;
	    // ignore any other tags for now
	  }
	  numVertices = getNumVerticesForElementTypeMSH(type);
	  if(!numVertices){
	    Msg(GERROR, "Unknown type (%d) for element %d", type, num); 
	    continue;
	  }
	}
	int n[30];
        for(int j = 0; j < numVertices; j++) fscanf(fp, "%d", &n[j]);
	int dim = 0;
	std::map<int, MVertex*> &v(vertices);
        switch (type) {
        case PNT:
	  points[elementary].push_back(v[n[0]]);
	  dim = 0;
          break;
        case LGN1:
	  elements[0][elementary].push_back
	    (new MLine(v[n[0]], v[n[1]], num, partition));
	  dim = 1;
          break;
        case LGN2:
	  elements[0][elementary].push_back
	    (new MLine2(v[n[0]], v[n[1]], v[n[2]], num, partition));
	  dim = 1;
          break;
        case TRI1:
	  elements[1][elementary].push_back
	    (new MTriangle(v[n[0]], v[n[1]], v[n[2]], num, partition));
	  dim = 2;
          break;
        case TRI2:
	  elements[1][elementary].push_back
	    (new MTriangle2(v[n[0]], v[n[1]], v[n[2]], v[n[3]], v[n[4]], v[n[5]], 
			    num, partition));
	  dim = 2;
          break;
        case QUA1:
	  elements[2][elementary].push_back
	    (new MQuadrangle(v[n[0]], v[n[1]], v[n[2]], v[n[3]], num, partition));
	  dim = 2;
          break;
        case QUA2:
	  elements[2][elementary].push_back
	    (new MQuadrangle2(v[n[0]], v[n[1]], v[n[2]], v[n[3]], v[n[4]], v[n[5]], 
			      v[n[6]], v[n[7]], v[n[8]], num, partition));
	  dim = 2;
          break;
	case TET1:
	  elements[3][elementary].push_back
	    (new MTetrahedron(v[n[0]], v[n[1]], v[n[2]], v[n[3]], num, partition));
	  dim = 3; 
	  break;
	case TET2:
	  elements[3][elementary].push_back
	    (new MTetrahedron2(v[n[0]], v[n[1]], v[n[2]], v[n[3]], v[n[4]], v[n[5]], 
			       v[n[6]], v[n[7]], v[n[8]], v[n[9]], num, partition));
	  dim = 3; 
	  break;
	case HEX1:
	  elements[4][elementary].push_back
	    (new MHexahedron(v[n[0]], v[n[1]], v[n[2]], v[n[3]], v[n[4]], v[n[5]], 
			     v[n[6]], v[n[7]], num, partition));
	  dim = 3; 
	  break;
	case HEX2:
	  elements[4][elementary].push_back
	    (new MHexahedron2(v[n[0]], v[n[1]], v[n[2]], v[n[3]], v[n[4]], v[n[5]], 
			      v[n[6]], v[n[7]], v[n[8]], v[n[9]], v[n[10]], v[n[11]], 
			      v[n[12]], v[n[13]], v[n[14]], v[n[15]], v[n[16]], v[n[17]], 
			      v[n[18]], v[n[19]], v[n[20]], v[n[21]], v[n[22]], v[n[23]], 
			      v[n[24]], v[n[25]], v[n[26]], num, partition));
	  dim = 3; 
	  break;
	case PRI1: 
	  elements[5][elementary].push_back
	    (new MPrism(v[n[0]], v[n[1]], v[n[2]], v[n[3]], v[n[4]], v[n[5]], 
			num, partition));
	  dim = 3; 
	  break;
	case PRI2: 
	  elements[5][elementary].push_back
	    (new MPrism2(v[n[0]], v[n[1]], v[n[2]], v[n[3]], v[n[4]], v[n[5]], 
			 v[n[6]], v[n[7]], v[n[8]], v[n[9]], v[n[10]], v[n[11]], 
			 v[n[12]], v[n[13]], v[n[14]], v[n[15]], v[n[16]], v[n[17]], 
			 num, partition));
	  dim = 3; 
	  break;
	case PYR1: 
	  elements[6][elementary].push_back
	    (new MPyramid(v[n[0]], v[n[1]], v[n[2]], v[n[3]], v[n[4]], num, partition));
	  dim = 3; 
	  break;
	case PYR2: 
	  elements[6][elementary].push_back
	    (new MPyramid2(v[n[0]], v[n[1]], v[n[2]], v[n[3]], v[n[4]], v[n[5]], 
			   v[n[6]], v[n[7]], v[n[8]], v[n[9]], v[n[10]], v[n[11]], 
			   v[n[12]], v[n[13]], num, partition));
	  dim = 3; 
	  break;
        default:
	  Msg(GERROR, "Unknown type (%d) for element %d", type, num); 
          break;
        }

	if(physical && (!physicals[dim].count(elementary) || 
			!physicals[dim][elementary].count(physical)))
	    physicals[dim][elementary][physical] = "unnamed";
	
	if(partition) meshPartitions.insert(partition);
	
	if(progress && (i % progress == progress - 1))
	  Msg(PROGRESS, "Read %d elements", i + 1);
      }
      Msg(PROGRESS, "");

    }

    do {
      if(!fgets(str, sizeof(str), fp) || feof(fp))
	Msg(GERROR, "Prematured end of mesh file");
    } while(str[0] != '$');

  }

  // store the elements in their associated elementary entity. If the
  // entity does not exist, create a new one.
  for(int i = 0; i < 7; i++) storeElementsInEntities(this, elements[i]);

  // treat points separately
  {
    std::map<int, std::vector<MVertex*> >::const_iterator it = points.begin();
    std::map<int, std::vector<MVertex*> >::const_iterator ite = points.end();
    for(; it != ite; ++it){
      GVertex *v = vertexByTag(it->first);
      if(!v){
	v = new gmshVertex(this, it->first);
	add(v);
      }
      for(unsigned int i = 0; i < it->second.size(); i++) 
	v->mesh_vertices.push_back(it->second[i]);
    }
  }

  // associate the correct geometrical entity with each mesh vertex
  associateEntityWithVertices(this);

  // special case for geometry vertices: now that the correct
  // geometrical entity has been associated with the vertices, we
  // reset mesh_vertices so that it can be filled again below
  for(viter it = firstVertex(); it != lastVertex(); ++it)
    (*it)->mesh_vertices.clear();

  // store the vertices in their associated geometrical entity
  storeVerticesInEntities(vertices);

  // store the physical tags
  for(int i = 0; i < 4; i++)  
    storePhysicalTagsInEntities(this, i, physicals[i]);

  fclose(fp);
  return 1;
}

template<class T>
static void writeElementsMSH(FILE *fp, const std::vector<T*> &ele, int saveAll, 
			     double version, int &num, int elementary, 
			     std::vector<int> &physicals)
{
  for(unsigned int i = 0; i < ele.size(); i++)
    if(saveAll)
      ele[i]->writeMSH(fp, version, ++num, elementary, 0);
    else
      for(unsigned int j = 0; j < physicals.size(); j++)
	ele[i]->writeMSH(fp, version, ++num, elementary, physicals[j]);
}

int GModel::writeMSH(const std::string &name, double version, bool saveAll, 
		     double scalingFactor)
{
  FILE *fp = fopen(name.c_str(), "w");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  // if there are no physicals we save all the elements
  if(noPhysicalGroups()) saveAll = true;

  // get the number of vertices and renumber the vertices in a
  // continuous sequence
  int numVertices = renumberMeshVertices();
  
  // get the number of elements
  int numElements = 0;
  for(viter it = firstVertex(); it != lastVertex(); ++it)
    numElements += (saveAll ? 1 : (*it)->physicals.size()) * 
      (*it)->mesh_vertices.size();
  for(eiter it = firstEdge(); it != lastEdge(); ++it)
    numElements += (saveAll ? 1 : (*it)->physicals.size()) * 
      (*it)->lines.size();
  for(fiter it = firstFace(); it != lastFace(); ++it)
    numElements += (saveAll ? 1 : (*it)->physicals.size()) * 
      ((*it)->triangles.size() + (*it)->quadrangles.size());
  for(riter it = firstRegion(); it != lastRegion(); ++it)
    numElements += (saveAll ? 1 : (*it)->physicals.size()) * 
      ((*it)->tetrahedra.size() + (*it)->hexahedra.size() +
       (*it)->prisms.size() + (*it)->pyramids.size());

  if(version > 2.0){
    fprintf(fp, "$MeshFormat\n");
    fprintf(fp, "%g %d %d\n", version, 0, (int)sizeof(double));
    fprintf(fp, "$EndMeshFormat\n");
    fprintf(fp, "$Nodes\n");
  }
  else
    fprintf(fp, "$NOD\n");
 
  fprintf(fp, "%d\n", numVertices);
  for(viter it = firstVertex(); it != lastVertex(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeMSH(fp, scalingFactor);
  for(eiter it = firstEdge(); it != lastEdge(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++)
      (*it)->mesh_vertices[i]->writeMSH(fp, scalingFactor);
  for(fiter it = firstFace(); it != lastFace(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeMSH(fp, scalingFactor);
  for(riter it = firstRegion(); it != lastRegion(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeMSH(fp, scalingFactor);

  if(version > 2.0){
    fprintf(fp, "$EndNodes\n");
    fprintf(fp, "$Elements\n");
  }
  else{
    fprintf(fp, "$ENDNOD\n");
    fprintf(fp, "$ELM\n");
  }

  fprintf(fp, "%d\n", numElements);
  int num = 0;

  for(viter it = firstVertex(); it != lastVertex(); ++it){
    writeElementsMSH(fp, (*it)->mesh_vertices, saveAll, version, num,
		     (*it)->tag(), (*it)->physicals);
  }
  for(eiter it = firstEdge(); it != lastEdge(); ++it){
    writeElementsMSH(fp, (*it)->lines, saveAll, version, num,
		     (*it)->tag(), (*it)->physicals);
  }
  for(fiter it = firstFace(); it != lastFace(); ++it){
    writeElementsMSH(fp, (*it)->triangles, saveAll, version, num,
		     (*it)->tag(), (*it)->physicals);
    writeElementsMSH(fp, (*it)->quadrangles, saveAll, version, num,
		     (*it)->tag(), (*it)->physicals);
  }
  for(riter it = firstRegion(); it != lastRegion(); ++it){
    writeElementsMSH(fp, (*it)->tetrahedra, saveAll, version, num,
		     (*it)->tag(), (*it)->physicals);
    writeElementsMSH(fp, (*it)->hexahedra, saveAll, version, num,
		     (*it)->tag(), (*it)->physicals);
    writeElementsMSH(fp, (*it)->prisms, saveAll, version, num,
		     (*it)->tag(), (*it)->physicals);
    writeElementsMSH(fp, (*it)->pyramids, saveAll, version, num,
		     (*it)->tag(), (*it)->physicals);
  }
  
  if(version > 2.0){
    fprintf(fp, "$EndElements\n");
  }
  else{
    fprintf(fp, "$ENDELM\n");
  }

  fclose(fp);
  return 1;
}

int GModel::writePOS(const std::string &name, double scalingFactor)
{
  FILE *fp = fopen(name.c_str(), "w");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  if(numRegion()){
    fprintf(fp, "View \"Volumes\" {\n");
    fprintf(fp, "T2(1.e5,30,%d){\"Elementary Entity\", \"Element Number\", "
	    "\"Gamma\", \"Eta\", \"Rho\"};\n", (1<<16)|(4<<8));
    for(riter it = firstRegion(); it != lastRegion(); ++it) {
      for(unsigned int i = 0; i < (*it)->tetrahedra.size(); i++)
	(*it)->tetrahedra[i]->writePOS(fp, scalingFactor, (*it)->tag());
      for(unsigned int i = 0; i < (*it)->hexahedra.size(); i++)
	(*it)->hexahedra[i]->writePOS(fp, scalingFactor, (*it)->tag());
      for(unsigned int i = 0; i < (*it)->prisms.size(); i++)
	(*it)->prisms[i]->writePOS(fp, scalingFactor, (*it)->tag());
      for(unsigned int i = 0; i < (*it)->pyramids.size(); i++)
	(*it)->pyramids[i]->writePOS(fp, scalingFactor, (*it)->tag());
    }
    fprintf(fp, "};\n");
  }
  
  if(numFace()){
    fprintf(fp, "View \"Surfaces\" {\n");
    fprintf(fp, "T2(1.e5,30,%d){\"Elementary Entity\", \"Element Number\", "
	    "\"Gamma\", \"Eta\", \"Rho\"};\n", (1<<16)|(4<<8));
    for(fiter it = firstFace(); it != lastFace(); ++it) {
      for(unsigned int i = 0; i < (*it)->triangles.size(); i++)
	(*it)->triangles[i]->writePOS(fp, scalingFactor, (*it)->tag());
      for(unsigned int i = 0; i < (*it)->quadrangles.size(); i++)
	(*it)->quadrangles[i]->writePOS(fp, scalingFactor, (*it)->tag());
    }
    fprintf(fp, "};\n");
  }

  if(numEdge()){
    fprintf(fp, "View \"Lines\" {\n");
    fprintf(fp, "T2(1.e5,30,%d){\"Elementary Entity\", \"Element Number\", "
	    "\"Gamma\", \"Eta\", \"Rho\"};\n", (1<<16)|(4<<8));
    for(eiter it = firstEdge(); it != lastEdge(); ++it) {
      for(unsigned int i = 0; i < (*it)->lines.size(); i++)
 	(*it)->lines[i]->writePOS(fp, scalingFactor, (*it)->tag());
    }
    fprintf(fp, "};\n");
  }

  fclose(fp);
  return 1;
}

static void swapBytes(char *array, int size, int n)
{
  char *x = new char[size];
  for(int i = 0; i < n; i++) {
    char *a = &array[i * size];
    memcpy(x, a, size);
    for(int c = 0; c < size; c++)
      a[size - 1 - c] = x[c];
  }
  delete [] x;
}

int GModel::readSTL(const std::string &name, double tolerance)
{
  FILE *fp = fopen(name.c_str(), "rb");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  // read all facets and store triplets of points
  std::vector<SPoint3> points;
  SBoundingBox3d bbox;

  // "solid", or binary data header
  char buffer[256];
  fgets(buffer, sizeof(buffer), fp);

  if(!strncmp(buffer, "solid", 5)) { 
    // ASCII STL
    while(!feof(fp)) {
      // "facet normal x y z", or "endsolid"
      if(!fgets(buffer, sizeof(buffer), fp)) break;
      if(!strncmp(buffer, "endsolid", 8)) break;
      char s1[256], s2[256];
      float x, y, z;
      sscanf(buffer, "%s %s %f %f %f", s1, s2, &x, &y, &z);
      // "outer loop"
      if(!fgets(buffer, sizeof(buffer), fp)) break;
      // "vertex x y z"
      for(int i = 0; i < 3; i++){
	if(!fgets(buffer, sizeof(buffer), fp)) break;
	sscanf(buffer, "%s %f %f %f", s1, &x, &y, &z);
	SPoint3 p(x, y, z);
	points.push_back(p);
	bbox += p;
      }
      // "endloop"
      if(!fgets(buffer, sizeof(buffer), fp)) break;
      // "endfacet"
      if(!fgets(buffer, sizeof(buffer), fp)) break;
    }
  }
  else{
    // Binary STL
    rewind(fp);
    char header[80];
    if(fread(header, sizeof(char), 80, fp)){
      unsigned int nfacets = 0;
      size_t ret = fread(&nfacets, sizeof(unsigned int), 1, fp);
      bool swap = false;
      if(nfacets > 10000000){
	Msg(INFO, "Swapping bytes from binary file");
	swap = true;
	swapBytes((char*)&nfacets, sizeof(unsigned int), 1);
      }
      if(ret && nfacets){
	char *data = new char[nfacets * 50 * sizeof(char)];
	ret = fread(data, sizeof(char), nfacets * 50, fp);
	if(ret == nfacets * 50){
	  for(unsigned int i = 0; i < nfacets; i++) {
	    float *xyz = (float *)&data[i * 50 * sizeof(char)];
	    if(swap) swapBytes((char*)xyz, sizeof(float), 12);
	    for(int j = 0; j < 3; j++){
	      SPoint3 p(xyz[3 + 3 * j], xyz[3 + 3 * j + 1], xyz[3 + 3 * j + 2]);
	      points.push_back(p);
	      bbox += p;
	    }
	  }
	}
	delete data;
      }
    }
  }

  if(!points.size()){
    Msg(GERROR, "No facets found in STL file");
    return 0;
  }
  
  if(points.size() % 3){
    Msg(GERROR, "Wrong number of points in STL file");
    return 0;
  }

  Msg(INFO, "%d facets", points.size() / 3);

  // create face
  GFace *face = new gmshFace(this, numFace() + 1);
  add(face);

  // create (unique) vertices and triangles
  double lc = norm(SVector3(bbox.max(), bbox.min()));
  MVertexLessThanLexicographic::tolerance = lc * tolerance;
  std::set<MVertex*, MVertexLessThanLexicographic> vertices;
  for(unsigned int i = 0; i < points.size(); i += 3){
    MVertex *v[3];
    for(int j = 0; j < 3; j++){
      double x = points[i + j].x(), y = points[i + j].y(), z = points[i + j].z();
      MVertex w(x, y, z);
      std::set<MVertex*, MVertexLessThanLexicographic>::iterator it = vertices.find(&w);
      if(it != vertices.end()) {
        v[j] = *it;
      }
      else {
        v[j] = new MVertex(x, y, z);
	vertices.insert(v[j]);
	face->mesh_vertices.push_back(v[j]);
      }
    }
    face->triangles.push_back(new MTriangle(v[0], v[1], v[2]));
  }

  fclose(fp);
  return 1;
}

int GModel::writeSTL(const std::string &name, bool binary, double scalingFactor)
{
  FILE *fp = fopen(name.c_str(), binary ? "wb" : "w");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  if(!binary)
    fprintf(fp, "solid Created by Gmsh\n");
  else{
    char header[80];
    strncpy(header, "Created by Gmsh", 80);
    fwrite(header, sizeof(char), 80, fp);
    unsigned int nfacets = 0;
    for(fiter it = firstFace(); it != lastFace(); ++it)
      nfacets += (*it)->triangles.size() + 2 * (*it)->quadrangles.size();
    fwrite(&nfacets, sizeof(unsigned int), 1, fp);
  }
    
  for(fiter it = firstFace(); it != lastFace(); ++it) {
    for(unsigned int i = 0; i < (*it)->triangles.size(); i++)
      (*it)->triangles[i]->writeSTL(fp, binary, scalingFactor);
    for(unsigned int i = 0; i < (*it)->quadrangles.size(); i++)
      (*it)->quadrangles[i]->writeSTL(fp, binary, scalingFactor);
  }

  if(!binary)
    fprintf(fp, "endsolid Created by Gmsh\n");
  
  fclose(fp);
  return 1;
}

static int skipUntil(FILE *fp, char *key)
{
  char str[256];
  while(fscanf(fp, "%s", str)){
    if(!strcmp(str, key)) return 1;
  }
  return 0;
}

static int readVerticesVRML(FILE *fp, std::map<int, MVertex*> &vertices,
			    std::vector<MVertex*> &allvertices)
{
  double x, y, z;
  if(fscanf(fp, " [ %lf %lf %lf", &x, &y, &z) != 3) return 0;
  int num = 0;
  vertices[num++] = new MVertex(x, y, z);
  while(fscanf(fp, " , %lf %lf %lf", &x, &y, &z) == 3)
    vertices[num++] = new MVertex(x, y, z);
  for(unsigned int i = 0; i < vertices.size(); i++)
    allvertices.push_back(vertices[i]);
  Msg(INFO, "%d vertices", vertices.size());
  return 1;
}

static int readElementsVRML(FILE *fp, std::map<int, MVertex*> &v, int region,
			    std::map<int, std::vector<MElement*> > elements[3], 
			    bool strips=false)
{
  int i;
  std::vector<int> idx;
  if(fscanf(fp, " [ %d", &i) != 1) return 0;
  idx.push_back(i);
  while(fscanf(fp, " , %d", &i) == 1){
    if(i != -1){
      idx.push_back(i);
    }
    else{
      for(unsigned int j = 0; j < idx.size(); j++){
	if(!v.count(idx[j])){
	  Msg(GERROR, "Bad vertex index in VRML file");
	  return 0;
	}
      }
      if(idx.size() < 2){
	Msg(INFO, "Skipping %d-vertex element", (int)idx.size());
      }
      else if(idx.size() == 2){
	elements[0][region].push_back(new MLine(v[idx[0]], v[idx[1]]));
      }
      else if(idx.size() == 3){
	elements[1][region].push_back
	  (new MTriangle(v[idx[0]], v[idx[1]], v[idx[2]]));
      }
      else if(!strips && idx.size() == 4){
	elements[2][region].push_back
	  (new MQuadrangle(v[idx[0]], v[idx[1]], v[idx[2]], v[idx[3]]));
      }
      else if(strips){ // triangle strip
	for(unsigned int j = 2; j < idx.size(); j++){
	  if(j % 2)
	    elements[1][region].push_back
	      (new MTriangle(v[idx[j]], v[idx[j - 1]], v[idx[j - 2]]));
	  else
	    elements[1][region].push_back
	      (new MTriangle(v[idx[j - 2]], v[idx[j - 1]], v[idx[j]]));
	}
      }
      else{ // import polygon as triangle fan
	for(unsigned int j = 2; j < idx.size(); j++){
	  elements[1][region].push_back
	    (new MTriangle(v[idx[0]], v[idx[j-1]], v[idx[j]]));
	}
      }
      idx.clear();
    }
  }
  if(idx.size()){
    Msg(GERROR, "Prematured end of VRML file");
    return 0;
  }
  Msg(INFO, "%d elements", elements[0][region].size() + 
      elements[1][region].size() + elements[2][region].size());
  return 1;
}

int GModel::readVRML(const std::string &name)
{
  FILE *fp = fopen(name.c_str(), "r");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  // This is by NO means a complete VRML/Inventor parser (but it's
  // sufficient for reading simple Inventor files... which is all I
  // need)
  std::map<int, MVertex*> vertices;
  std::vector<MVertex*> allvertices;
  std::map<int, std::vector<MElement*> > elements[3];
  int region = 0;
  char buffer[256], str[256];
  while(!feof(fp)) {
    if(!fgets(buffer, sizeof(buffer), fp)) break;
    if(buffer[0] != '#'){ // skip comments
      sscanf(buffer, "%s", str);
      if(!strcmp(str, "IndexedTriangleStripSet")){
	region++;
	vertices.clear();
	if(!skipUntil(fp, "vertex")) break;
	if(!readVerticesVRML(fp, vertices, allvertices)) break;
	if(!skipUntil(fp, "coordIndex")) break;
	if(!readElementsVRML(fp, vertices, region, elements, true)) break;
      }
      else if(!strcmp(str, "Coordinate3")){
	vertices.clear();
	if(!skipUntil(fp, "point")) break;
	if(!readVerticesVRML(fp, vertices, allvertices)) break;
      }
      else if(!strcmp(str, "IndexedFaceSet") || !strcmp(str, "IndexedLineSet")){
	region++;
	if(!skipUntil(fp, "coordIndex")) break;
	if(!readElementsVRML(fp, vertices, region, elements)) break;
      }
      else if(!strcmp(str, "DEF")){
	char str1[256], str2[256];
	if(!sscanf(buffer, "%s %s %s", str1, str2, str)) break;
	if(!strcmp(str, "IndexedFaceSet") || !strcmp(str, "IndexedLineSet")){
	  region++;
	  if(!skipUntil(fp, "coordIndex")) break;
	  if(!readElementsVRML(fp, vertices, region, elements)) break;
	}
      }
    }
  }

  for(int i = 0; i < 3; i++) storeElementsInEntities(this, elements[i]);
  associateEntityWithVertices(this);
  storeVerticesInEntities(allvertices);

  fclose(fp);
  return 1;
}

int GModel::writeVRML(const std::string &name, double scalingFactor)
{
  FILE *fp = fopen(name.c_str(), "w");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  renumberMeshVertices();

  fprintf(fp, "#VRML V1.0 ascii\n");
  fprintf(fp, "#created by Gmsh\n");
  fprintf(fp, "Coordinate3 {\n");
  fprintf(fp, "  point [\n");

  for(viter it = firstVertex(); it != lastVertex(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeVRML(fp, scalingFactor);
  for(eiter it = firstEdge(); it != lastEdge(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++)
      (*it)->mesh_vertices[i]->writeVRML(fp, scalingFactor);
  for(fiter it = firstFace(); it != lastFace(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeVRML(fp, scalingFactor);

  fprintf(fp, "  ]\n");
  fprintf(fp, "}\n");

  for(eiter it = firstEdge(); it != lastEdge(); ++it){
    fprintf(fp, "DEF Curve%d IndexedLineSet {\n", (*it)->tag());
    fprintf(fp, "  coordIndex [\n");
    for(unsigned int i = 0; i < (*it)->lines.size(); i++)
      (*it)->lines[i]->writeVRML(fp);
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
  }

  for(fiter it = firstFace(); it != lastFace(); ++it){
    fprintf(fp, "DEF Surface%d IndexedFaceSet {\n", (*it)->tag());
    fprintf(fp, "  coordIndex [\n");
    for(unsigned int i = 0; i < (*it)->triangles.size(); i++)
      (*it)->triangles[i]->writeVRML(fp);
    for(unsigned int i = 0; i < (*it)->quadrangles.size(); i++)
      (*it)->quadrangles[i]->writeVRML(fp);
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
  }
  
  fclose(fp);
  return 1;
}

static void addInGroupOfNodesUNV(FILE *fp, std::set<int> &nodes, int num)
{
  std::set<int>::iterator it = nodes.find(num);
  if(it == nodes.end()){
    nodes.insert(num);
    fprintf(fp, "%10d%10d%2d%2d%2d%2d%2d%2d\n", num, 1, 0, 1, 0, 0, 0, 0);
    fprintf(fp, "   0.0000000000000000D+00   1.0000000000000000D+00"
	    "   0.0000000000000000D+00\n");
    fprintf(fp, "   0.0000000000000000D+00   0.0000000000000000D+00"
	    "   0.0000000000000000D+00\n");
    fprintf(fp, "%10d%10d%10d%10d%10d%10d\n", 0, 0, 0, 0, 0, 0);
  }
}

template<class T>
static void addInGroupOfNodesUNV(FILE *fp, std::set<int> &nodes, 
				 std::vector<T*> &elements)
{
  for(unsigned int i = 0; i < elements.size(); i++)
    for(int j = 0; j < elements[i]->getNumVertices(); j++)
      addInGroupOfNodesUNV(fp, nodes, elements[i]->getVertex(j)->getNum());
}

int GModel::writeUNV(const std::string &name, double scalingFactor)
{
  FILE *fp = fopen(name.c_str(), "w");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  renumberMeshVertices();

  // IDEAS NODES record
  fprintf(fp, "%6d\n", -1);
  fprintf(fp, "%6d\n", 2411);
  for(viter it = firstVertex(); it != lastVertex(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeUNV(fp, scalingFactor);
  for(eiter it = firstEdge(); it != lastEdge(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++)
      (*it)->mesh_vertices[i]->writeUNV(fp, scalingFactor);
  for(fiter it = firstFace(); it != lastFace(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeUNV(fp, scalingFactor);
  for(riter it = firstRegion(); it != lastRegion(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeUNV(fp, scalingFactor);
  fprintf(fp, "%6d\n", -1);  

  // IDEAS ELEMENTS record
  fprintf(fp, "%6d\n", -1);
  fprintf(fp, "%6d\n", 2412);
  for(eiter it = firstEdge(); it != lastEdge(); ++it){
    for(unsigned int i = 0; i < (*it)->lines.size(); i++)
      (*it)->lines[i]->writeUNV(fp, (*it)->tag());
  }
  for(fiter it = firstFace(); it != lastFace(); ++it){
    for(unsigned int i = 0; i < (*it)->triangles.size(); i++)
      (*it)->triangles[i]->writeUNV(fp, (*it)->tag());
    for(unsigned int i = 0; i < (*it)->quadrangles.size(); i++)
      (*it)->quadrangles[i]->writeUNV(fp, (*it)->tag());
  }
  for(riter it = firstRegion(); it != lastRegion(); ++it){
    for(unsigned int i = 0; i < (*it)->tetrahedra.size(); i++)
      (*it)->tetrahedra[i]->writeUNV(fp, (*it)->tag());
    for(unsigned int i = 0; i < (*it)->hexahedra.size(); i++)
      (*it)->hexahedra[i]->writeUNV(fp, (*it)->tag());
    for(unsigned int i = 0; i < (*it)->prisms.size(); i++)
      (*it)->prisms[i]->writeUNV(fp, (*it)->tag());
  }
  fprintf(fp, "%6d\n", -1);

  std::map<int, std::vector<GEntity*> > physicals[4];
  getPhysicalGroups(physicals);

  for(int dim = 0; dim < 4; dim++){
    std::map<int, std::vector<GEntity*> >::const_iterator it = physicals[dim].begin();
    std::map<int, std::vector<GEntity*> >::const_iterator ite = physicals[dim].end();
    for(; it != ite; ++it){
      // IDEAS GROUPOFNODES record
      fprintf(fp, "%6d\n", -1);
      fprintf(fp, "%6d\n", 790);
      fprintf(fp, "%10d%10d\n", it->first, 1);
      fprintf(fp, "LOAD SET %2d\n", 1);
      std::set<int> nodes;
      for(unsigned int i = 0; i < it->second.size(); i++){
	// we could also do this using the mesh_vertices of the entity
	// and all the entities in the closure
	GVertex *v;
	switch(dim){
	case 0: 
	  v = (GVertex*)it->second[i];
	  for(unsigned int j = 0; j < v->mesh_vertices.size(); j++)
	    addInGroupOfNodesUNV(fp, nodes, v->mesh_vertices[j]->getNum());
	  break;
	case 1: 
	  addInGroupOfNodesUNV(fp, nodes, ((GEdge*)it->second[i])->lines);
	  break;
	case 2: 
	  addInGroupOfNodesUNV(fp, nodes, ((GFace*)it->second[i])->triangles);
	  addInGroupOfNodesUNV(fp, nodes, ((GFace*)it->second[i])->quadrangles);
	  break;
	case 3: 
	  addInGroupOfNodesUNV(fp, nodes, ((GRegion*)it->second[i])->tetrahedra);
	  addInGroupOfNodesUNV(fp, nodes, ((GRegion*)it->second[i])->hexahedra);
	  addInGroupOfNodesUNV(fp, nodes, ((GRegion*)it->second[i])->prisms);
	  addInGroupOfNodesUNV(fp, nodes, ((GRegion*)it->second[i])->pyramids);
	  break;
	}
      }
      fprintf(fp, "%6d\n", -1);
    }
  }

  fclose(fp);
  return 1;
}

int GModel::readMESH(const std::string &name)
{
  FILE *fp = fopen(name.c_str(), "r");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  char buffer[256];
  fgets(buffer, sizeof(buffer), fp);

  char str[256];
  int format;
  sscanf(buffer, "%s %d", str, &format);

  if(format != 1){
    Msg(GERROR, "Non-ASCII INRIA mesh import is not yet available");
    return 0;
  }

  std::map<int, MVertex*> vertices;
  std::map<int, std::vector<MElement*> > elements[2];

  while(!feof(fp)) {
    if(!fgets(buffer, sizeof(buffer), fp)) break;
    if(buffer[0] != '#'){ // skip comments
      sscanf(buffer, "%s", str);
      if(!strcmp(str, "Dimension")){
	if(!fgets(buffer, sizeof(buffer), fp)) break;
      }
      else if(!strcmp(str, "Vertices")){
	if(!fgets(buffer, sizeof(buffer), fp)) break;
	int nbv;
	sscanf(buffer, "%d", &nbv);
	Msg(INFO, "%d vertices", nbv);
	for(int i = 0; i < nbv; i++) {
	  if(!fgets(buffer, sizeof(buffer), fp)) break;
	  int cl;
	  double x, y, z;
	  sscanf(buffer, "%lf %lf %lf %d", &x, &y, &z, &cl);
	  vertices[i + 1] = new MVertex(x, y, z);
	}
      }
      else if(!strcmp(str, "Triangles")){
	if(!fgets(buffer, sizeof(buffer), fp)) break;
	int nbt;
	sscanf(buffer, "%d", &nbt);
	Msg(INFO, "%d triangles", nbt);
	for(int i = 0; i < nbt; i++) {
	  if(!fgets(buffer, sizeof(buffer), fp)) break;
	  int n1, n2, n3, cl;
	  sscanf(buffer, "%d %d %d %d", &n1, &n2, &n3, &cl);
	  elements[0][cl].push_back
	    (new MTriangle(vertices[n1], vertices[n2], vertices[n3]));
	}
      }
      else if(!strcmp(str, "Quadrilaterals")) {
	if(!fgets(buffer, sizeof(buffer), fp)) break;
	int nbq;
	sscanf(buffer, "%d", &nbq);
	Msg(INFO, "%d quadrangles", nbq);
	for(int i = 0; i < nbq; i++) {
	  if(!fgets(buffer, sizeof(buffer), fp)) break;
	  int n1, n2, n3, n4, cl;
	  sscanf(buffer, "%d %d %d %d %d", &n1, &n2, &n3, &n4, &cl);
	  elements[1][cl].push_back
	    (new MQuadrangle(vertices[n1], vertices[n2], vertices[n3], vertices[n4]));
	}
      }
    }
  }

  // store the elements in their associated elementary entity. If the
  // entity does not exist, create a new one.
  for(int i = 0; i < 2; i++) storeElementsInEntities(this, elements[i]);

  // associate the correct geometrical entity with each mesh vertex
  associateEntityWithVertices(this);

  // store the vertices in their associated geometrical entity
  storeVerticesInEntities(vertices);

  fclose(fp);
  return 1;
}

int GModel::writeMESH(const std::string &name, double scalingFactor)
{
  FILE *fp = fopen(name.c_str(), "w");
  if(!fp){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  fprintf(fp, " MeshVersionFormatted 1\n");
  fprintf(fp, " Dimension\n");
  fprintf(fp, " 3\n");

  int numVertices = renumberMeshVertices();
  fprintf(fp, " Vertices\n");
  fprintf(fp, " %d\n", numVertices);
  for(viter it = firstVertex(); it != lastVertex(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeMESH(fp, scalingFactor);
  for(eiter it = firstEdge(); it != lastEdge(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++)
      (*it)->mesh_vertices[i]->writeMESH(fp, scalingFactor);
  for(fiter it = firstFace(); it != lastFace(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeMESH(fp, scalingFactor);
  for(riter it = firstRegion(); it != lastRegion(); ++it)
    for(unsigned int i = 0; i < (*it)->mesh_vertices.size(); i++) 
      (*it)->mesh_vertices[i]->writeMESH(fp, scalingFactor);
  
  int numTriangles = 0, numQuadrangles = 0;
  for(fiter it = firstFace(); it != lastFace(); ++it){
    numTriangles += (*it)->triangles.size();
    numQuadrangles += (*it)->quadrangles.size();
  }
  if(numTriangles){
    fprintf(fp, " Triangles\n");
    fprintf(fp, " %d\n", numTriangles);
    for(fiter it = firstFace(); it != lastFace(); ++it)
      for(unsigned int i = 0; i < (*it)->triangles.size(); i++)
	(*it)->triangles[i]->writeMESH(fp, (*it)->tag());
  }
  if(numQuadrangles){
    fprintf(fp, " Quadrilaterals\n");
    fprintf(fp, " %d\n", numQuadrangles);
    for(fiter it = firstFace(); it != lastFace(); ++it)
      for(unsigned int i = 0; i < (*it)->triangles.size(); i++)
	(*it)->quadrangles[i]->writeMESH(fp, (*it)->tag());
  }

  fprintf(fp, " End\n");
  
  fclose(fp);
  return 1;
}
