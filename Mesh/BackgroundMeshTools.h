

#ifndef _BACKGROUND_MESH_TOOLS_H_
#define _BACKGROUND_MESH_TOOLS_H_

#include "STensor3.h"

class GFace;
class GVertex;
class GEdge;
class GEntity;


SMetric3 buildMetricTangentToCurve (SVector3 &t, double l_t, double l_n);
SMetric3 buildMetricTangentToSurface (SVector3 &t1, SVector3 &t2, double l_t1, double l_t2, double l_n);
double BGM_MeshSize(GEntity *ge, double U, double V, double X, double Y, double Z);
SMetric3 BGM_MeshMetric(GEntity *ge, double U, double V, double X, double Y, double Z);
bool Extend1dMeshIn2dSurfaces();
bool Extend2dMeshIn3dVolumes();
SMetric3 max_edge_curvature_metric(const GVertex *gv);
SMetric3 max_edge_curvature_metric(const GEdge *ge, double u, double &l);
SMetric3 metric_based_on_surface_curvature(const GFace *gf, double u, double v, bool surface_isotropic = false,double d_normal = 1.e12,double d_tangent_max = 1.e12);

//
//static double LC_MVertex_CURV(GEntity *ge, double U, double V);
//SMetric3 LC_MVertex_CURV_ANISO(GEntity *ge, double U, double V);
//static double LC_MVertex_PNTS(GEntity *ge, double U, double V);
//static double max_edge_curvature(const GVertex *gv);
//static double max_surf_curvature(const GEdge *ge, double u);
//static SMetric3 metric_based_on_surface_curvature(const GEdge *ge, double u, bool iso_surf);
//static SMetric3 metric_based_on_surface_curvature(const GVertex *gv, bool iso_surf);

#endif