#ifndef __DENSITY_FIELD_H__
#define __DENSITY_FIELD_H__

#include "vec.h"
#include <vector>


// density_field.h: Defines the DensityField class for representing 3D scalar fields (e.g., volumetric data)
// used for procedural terrain and volumetric mesh generation (such as with marching cubes).

// The DensityField class stores a 3D grid of scalar "density" values, 
// a bit like a 3D matrix. Think of each grid cell as holding a value
// that says how "solid" or "empty" that spot in space is. High values
// might mean "inside" a surface, low values "outside." 

class DensityField {
public:
	// Constructor to initialize the density field with given dimensions and spacing
	DensityField(int nx, int ny, int nz, float voxel_size);
	// Set the density value at grid position (i, j, k)
	void setDensity(int i, int j, int k, float value);
	// Get the density value at grid position (i, j, k)
	float getDensity(int i, int j, int k) const;
	// Get the dimensions of the density field
	int getNx() const;
	int getNy() const;
	int getNz() const;
	// Get the spacing between grid points
	float getVoxelSize() const;


private:
	// nx represents the number of grid points along the x-axis
	// nx - 1 is the number of cells along that axis
	int nx, ny, nz;
	float voxel_size;
	std::vector<float> data;
};

#endif