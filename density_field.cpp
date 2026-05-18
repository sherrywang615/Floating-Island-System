#include "density_field.h"
#include "vec.h"
#include <vector>

DensityField::DensityField(int nx, int ny, int nz, float voxel_size)
	: nx(nx), ny(ny), nz(nz), voxel_size(voxel_size) {
	data.resize(nx * ny * nz, 0.0f);
}

void DensityField::setDensity(int i, int j, int k, float value) {
	data[i + nx * (j + ny * k)] = value;
}

float DensityField::getDensity(int i, int j, int k) const {
	return data[i + nx * (j + ny * k)];
}

int DensityField::getNx() const {
	return nx;
}

int DensityField::getNy() const {
	return ny;
}

int DensityField::getNz() const {
	return nz;
}

float DensityField::getVoxelSize() const {
	return voxel_size;
}