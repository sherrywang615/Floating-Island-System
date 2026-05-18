#include "lerped_spheres.h"
#include "ray.h"
#include "box.h"
#include "surface_nets.h"
#include "noise.h"
#include <iostream>
#include <limits>
#include <algorithm>

LerpedSpheres::LerpedSpheres(const std::vector<SphereData>& spheres_input, 
                             float voxel_size, 
                             float blend_factor,
                             bool smooth,
                             float noise_amplitude,
                             float noise_scale)
    : spheres(spheres_input), blend_factor(blend_factor), 
      noise_amplitude(noise_amplitude), noise_scale(noise_scale),
      d(1, 1, 1, voxel_size)
{
    if (spheres.empty()) {
        std::cerr << "Warning: LerpedSpheres created with no spheres" << std::endl;
        return;
    }

    // Calculate the bounding box that contains all spheres
    bounding_box.Make_Empty();
    for (const SphereData& sphere : spheres) {
        vec3 expansion(sphere.radius, sphere.radius, sphere.radius);
        bounding_box.Include_Point(sphere.center - expansion);
        bounding_box.Include_Point(sphere.center + expansion);
    }
    
    vec3 size = bounding_box.hi - bounding_box.lo;
    
    // Re-initialize the density field to cover all spheres with proper dimensions
    d = DensityField(
        static_cast<int>(std::ceil(size[0] / voxel_size)) + 1,
        static_cast<int>(std::ceil(size[1] / voxel_size)) + 1,
        static_cast<int>(std::ceil(size[2] / voxel_size)) + 1,
        voxel_size
    );

    // Fill the density field with blended sphere distances
    for (int i = 0; i < d.getNx(); i++) {
        for (int j = 0; j < d.getNy(); j++) {
            for (int k = 0; k < d.getNz(); k++) {
                // Compute the position of the center of the voxel
                vec3 pos;
                pos[0] = bounding_box.lo[0] + i * voxel_size;
                pos[1] = bounding_box.lo[1] + j * voxel_size;
                pos[2] = bounding_box.lo[2] + k * voxel_size;
                
                // Calculate blended signed distance to all spheres
                float min_dist = std::numeric_limits<float>::max();
                
                if (blend_factor < 0.01f) {
                    // No blending - use simple minimum distance
                    for (const SphereData& sphere : spheres) {
                        float dist = (pos - sphere.center).magnitude();
                        float signed_dist = dist - sphere.radius;
                        min_dist = std::min(min_dist, signed_dist);
                    }
                } else {
                    // Use smooth minimum for blending
                    bool first = true;
                    for (const SphereData& sphere : spheres) {
                        float dist = (pos - sphere.center).magnitude();
                        float signed_dist = dist - sphere.radius;
                        
                        if (first) {
                            min_dist = signed_dist;
                            first = false;
                        } else {
                            min_dist = SmoothUnion(min_dist, signed_dist, blend_factor);
                        }
                    }
                }
                
                // Apply Perlin noise to erode the surface (only decrease, never increase)
                if (noise_amplitude > 0.0f) {
                    // Sample 3D Perlin noise at the current position
                    float noise_value = noise::perlin3D_fbm(
                        pos[0] * noise_scale,
                        pos[1] * noise_scale,
                        pos[2] * noise_scale,
                        4,      // octaves
                        2.0f,   // lacunarity
                        0.5f    // gain
                    );
                    // noise_value is in range [-1, 1]
                    // Only apply when noise is negative (erosion)
                    if (noise_value < 0.0f) {
                        // Scale by amplitude and add to distance
                        // now some voxels that would have been negative (inside) become positive (outside), causing erosion
                        min_dist += -noise_value * noise_amplitude;
                    }
                }
                
                // Convert signed distance to density
                // Positive distance (outside) = 0 density
                // Negative distance (inside) = 1 density
                float density = 0.0f;
                if (min_dist < 0) {
                    density = 1.0f;
                }
                
                d.setDensity(i, j, k, density);
            }
        }
    }

    // Generate the mesh from the density field
    if (smooth) {
        // Use Surface Nets for smooth mesh
        m = SurfaceNets::GenerateMesh(d, 0.5);
        
        // Translate mesh vertices to world space (Surface Nets generates relative to origin)
        for (size_t i = 0; i < m.vertices.size(); i++) {
            m.vertices[i] = m.vertices[i] + bounding_box.lo;
        }
    } else {
        // Use blocky mesh generation (original method)
        m.Generate_Mesh_From_Density_Field(d, 0.5f, bounding_box.lo);
    }
}

Hit LerpedSpheres::Intersection(const Ray& ray, int part) const {
    Hit result = m.Intersection(ray, part);
    // Override the object pointer to point to this LerpedSpheres, not the internal mesh
    if (result.object != nullptr) {
        result.object = this;
    }
    return result;
}

vec3 LerpedSpheres::Normal(const vec3& point, int part) const {
    return m.Normal(point, part);
}

Box LerpedSpheres::Bounding_Box(int part) const {
    Box box = bounding_box;
    float voxel_size = d.getVoxelSize();
    // Expand by half voxel size since voxels extend in all directions
    vec3 expansion(0.5f * voxel_size, 0.5f * voxel_size, 0.5f * voxel_size);
    box.lo = box.lo - expansion;
    box.hi = box.hi + expansion;
    return box;
}

float LerpedSpheres::SmoothUnion(float d1, float d2, float k) const {
    // Pulled from IQ's article on distance functions, smooth minimum functions
	// https://iquilezles.org/articles/distfunctions/
	// This is the quadratic polynomial smoothmin from https://iquilezles.org/articles/smin/
    // k controls the smoothness: larger k = more blending
	k *= 4.0f;
    float h = std::max(k - std::abs(d1 - d2), 0.0f);
    return std::min(d1, d2) - h * h * k * 0.25f/k;
}
