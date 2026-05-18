#ifndef __CUBED_SPHERE_H__
#define __CUBED_SPHERE_H__

#include "object.h"
#include "density_field.h"
#include "mesh.h"
#include "box.h"

#include <cmath>

class CubedSphere : public Object
{
    vec3 center;
    double radius;
	DensityField d; // 3D grid of density values
	Mesh m; // Mesh generated from the density field

public:
    // smooth: true = use Surface Nets for smooth mesh, false = use blocky mesh
    CubedSphere(const vec3& center_input, double radius_input, float voxel_size = 0.5f, bool smooth = true);

    // Get the mesh generated from the density field (for OpenGL rendering)
    const Mesh& GetMesh() const { return m; }

    virtual Hit Intersection(const Ray& ray, int part) const override;
    virtual vec3 Normal(const vec3& point, int part) const override;
    virtual Box Bounding_Box(int part) const override;
};
#endif