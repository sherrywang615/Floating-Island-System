#ifndef __SURFACE_NETS_H__
#define __SURFACE_NETS_H__

#include "density_field.h"
#include "mesh.h"

class SurfaceNets
{
public:
    // Generate mesh from density field using Surface Nets algorithm
    // This produces smoother surfaces than Marching Cubes with fewer vertices
    // Based on: https://bonsairobo.medium.com/smooth-voxel-mapping-a-technical-deep-dive-on-real-time-surface-nets-and-texturing-ef06d0f8ca14
    static Mesh GenerateMesh(const DensityField& field, double isovalue = 0.5);
    
private:
    // Calculate gradient at a grid point (for vertex positioning)
    static vec3 CalculateGradient(const DensityField& field, int x, int y, int z);
    
    // Check if a cube crosses the isosurface
    static bool CubeCrossesSurface(const DensityField& field, int x, int y, int z, double isovalue);
    
    // Get vertex position using gradient-weighted interpolation
    static vec3 GetVertexPosition(const DensityField& field, int x, int y, int z, double isovalue);
};

#endif



