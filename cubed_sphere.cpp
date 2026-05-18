#include "cubed_sphere.h"
#include "ray.h"
#include "box.h"
#include "surface_nets.h"
#include <iostream>

CubedSphere::CubedSphere(const vec3& center_input, double radius_input, float voxel_size, bool smooth)
    : center(center_input), radius(radius_input), d(
        static_cast<int>(std::ceil((2 * radius_input) / voxel_size)) + 1,
        static_cast<int>(std::ceil((2 * radius_input) / voxel_size)) + 1,
        static_cast<int>(std::ceil((2 * radius_input) / voxel_size)) + 1,
        voxel_size
    )
{
	// Initialize the density field to represent a sphere
	for (int i = 0; i < d.getNx(); i++) {
		for (int j = 0; j < d.getNy(); j++) {
			for (int k = 0; k < d.getNz(); k++) {
				// Compute the position of the center of the cube
				vec3 pos;
				pos[0] = center[0] - radius + i * d.getVoxelSize() + 0.5 * d.getVoxelSize();
				pos[1] = center[1] - radius + j * d.getVoxelSize() + 0.5 * d.getVoxelSize();
				pos[2] = center[2] - radius + k * d.getVoxelSize() + 0.5 * d.getVoxelSize();
				// Compute the density value based on distance from center of sphere
				float dist = (pos - center).magnitude();
				float density = radius - dist; // Inside sphere: positive, outside: negative
				if (density < 0) density = 0; // zero outside sphere
				else { density = 1.0f; } // one inside sphere
				d.setDensity(i, j, k, density);
			}
		}
	}
	// Generate the mesh from the density field
	vec3 field_origin = center - vec3(radius, radius, radius);
	if (smooth) {
		// Use Surface Nets for smooth mesh
		// Based on: https://bonsairobo.medium.com/smooth-voxel-mapping-a-technical-deep-dive-on-real-time-surface-nets-and-texturing-ef06d0f8ca14
		m = SurfaceNets::GenerateMesh(d, 0.5);
		
		// Translate mesh vertices to world space (Surface Nets generates relative to origin)
		for (size_t i = 0; i < m.vertices.size(); i++) {
			m.vertices[i] = m.vertices[i] + field_origin;
		}
	} else {
		// Use blocky mesh generation (original method)
		m.Generate_Mesh_From_Density_Field(d, 0.5f, field_origin);
	}
}

Hit CubedSphere::Intersection(const Ray& ray, int part) const {
	Hit result = m.Intersection(ray, part);
	// Override the object pointer to point to this CubedSphere, not the internal mesh
	if (result.object != nullptr) {
		result.object = this;
	}
	return result;
}

vec3 CubedSphere::Normal(const vec3& point, int part) const {
	return m.Normal(point, part);
}

Box CubedSphere::Bounding_Box(int part) const {
	Box box;
	float voxel_size = d.getVoxelSize();
	// Since voxels are centered on grid points and extend 0.5*voxel_size in each direction,
	// we need to expand the bounding box by 0.5*voxel_size
	box.lo = center - vec3(radius + 0.5 * voxel_size, radius + 0.5 * voxel_size, radius + 0.5 * voxel_size);
	box.hi = center + vec3(radius + 0.5 * voxel_size, radius + 0.5 * voxel_size, radius + 0.5 * voxel_size);
	return box;
}

