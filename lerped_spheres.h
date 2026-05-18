#ifndef __LERPED_SPHERES_H__
#define __LERPED_SPHERES_H__

#include "object.h"
#include "density_field.h"
#include "mesh.h"
#include "box.h"
#include <vector>
#include <cmath>

struct SphereData {
    vec3 center;
    double radius;
    
    SphereData(const vec3& c, double r) : center(c), radius(r) {}
};

class LerpedSpheres : public Object
{
    std::vector<SphereData> spheres;
    float blend_factor; // Controls how much spheres meld together (0.0 = sharp, 1.0 = smooth)
    float noise_amplitude; // Controls the strength of erosion (only decreases surface)
    float noise_scale; // Controls the frequency of noise
    DensityField d; // 3D grid of density values
    Mesh m; // Mesh generated from the density field
    Box bounding_box;

public:
    // Constructor takes a list of sphere centers and radii, voxel size, blend factor, and smooth option
    // blend_factor: 0.0 = no blending (sharp edges), 1.0+ = smooth melding
    // smooth: true = use Surface Nets for smooth mesh, false = use blocky mesh
    // noise_amplitude: strength of erosion (only decreases surface, 0.0 = no noise)
    // noise_scale: frequency of noise (higher = more detail)
    LerpedSpheres(const std::vector<SphereData>& spheres_input, 
                  float voxel_size = 0.5f, 
                  float blend_factor = 0.5f,
                  bool smooth = true,
                  float noise_amplitude = 4.0f,
                  float noise_scale = 0.3f);

    // Get the mesh generated from the density field (for OpenGL rendering)
    const Mesh& GetMesh() const { return m; }

    virtual Hit Intersection(const Ray& ray, int part) const override;
    virtual vec3 Normal(const vec3& point, int part) const override;
    virtual Box Bounding_Box(int part) const override;

private:
    // Smooth min function for blending distances
    float SmoothUnion(float a, float b, float k) const;
};

#endif
