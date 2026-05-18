#ifndef __ISLAND_GENERATOR_H__
#define __ISLAND_GENERATOR_H__

#include "density_field.h"
#include "mesh.h"

class IslandGenerator
{
public:
    // Generate a floating island
    static DensityField GenerateIsland(int size = 64, const vec3& center = vec3(0,0,0), double radius = 10.0);
    
    // Add terrain features using noise
    static void AddTerrainFeatures(DensityField& field);
    
    // Blend multiple primitives to form island base
    static void CreateIslandBase(DensityField& field);
};
#endif
