#ifndef __MARCHING_CUBES_H__
#define __MARCHING_CUBES_H__

#include "density_field.h"
#include "mesh.h"

class MarchingCubes
{
public:
    // Generate mesh from density field
    static Mesh GenerateMesh(const DensityField& field, double isovalue = 0.5);
    
private:
    // Get cube index (0-255) based on corner densities
    static int GetCubeIndex(const double corners[8], double isovalue);
    
    // Interpolate vertex position on edge
    static vec3 InterpolateVertex(const vec3& v1, const vec3& v2, 
                                  double d1, double d2, double isovalue);
    
    // Process a single cube
    static void ProcessCube(Mesh& mesh, const DensityField& field, 
                           int x, int y, int z, double isovalue);
};

#endif