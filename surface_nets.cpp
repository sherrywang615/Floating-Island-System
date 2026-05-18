#include "surface_nets.h"
#include "vec.h"
#include <cmath>
#include <map>
#include <vector>
#include <algorithm>

// Surface Nets implementation based on:
// https://medium.com/@ryandremer/implementing-surface-nets-in-godot-f48ecd5f29ff
//
// Key differences from previous implementation:
// 1. Vertices are placed at edge midpoints where sign changes occur
// 2. Vertices are averaged from all edges that contribute to them
// 3. This produces smoother surfaces without spikes

// Check if an edge crosses the isosurface (sign change)
static bool EdgeCrossesSurface(const DensityField& field, int x1, int y1, int z1, 
                                int x2, int y2, int z2, double isovalue)
{
    float d1 = field.getDensity(x1, y1, z1);
    float d2 = field.getDensity(x2, y2, z2);
    
    // Check if one side is above and the other is below the isovalue
    bool side1 = d1 >= isovalue;
    bool side2 = d2 >= isovalue;
    return side1 != side2;
}

// Get the position along an edge where the surface crosses (interpolated)
static vec3 GetEdgeIntersection(const DensityField& field, 
                                int x1, int y1, int z1,
                                int x2, int y2, int z2,
                                double isovalue)
{
    float d1 = field.getDensity(x1, y1, z1);
    float d2 = field.getDensity(x2, y2, z2);
    
    float voxel_size = field.getVoxelSize();
    
    // Get world positions
    vec3 p1(x1 * voxel_size, y1 * voxel_size, z1 * voxel_size);
    vec3 p2(x2 * voxel_size, y2 * voxel_size, z2 * voxel_size);
    
    // Linear interpolation to find intersection point
    if (std::abs(d2 - d1) < 1e-6) {
        // Densities are equal, use midpoint
        return (p1 + p2) * 0.5;
    }
    
    float t = (isovalue - d1) / (d2 - d1);
    t = std::max(0.0f, std::min(1.0f, t)); // Clamp to [0, 1]
    
    return p1 + (p2 - p1) * t;
}

// Generate mesh using Surface Nets algorithm (edge-based approach)
Mesh SurfaceNets::GenerateMesh(const DensityField& field, double isovalue)
{
    Mesh mesh;
    
    int nx = field.getNx();
    int ny = field.getNy();
    int nz = field.getNz();
    
    // Map from cube position (x,y,z) to vertex index
    // Each cube can have a vertex if any of its edges cross the surface
    std::map<std::vector<int>, int> cubeToVertex;
    
    // Map from cube position to accumulated vertex position and count
    // This allows us to average positions from multiple edges
    std::map<std::vector<int>, vec3> cubeToPosition;
    std::map<std::vector<int>, int> cubeToCount;
    
    // First pass: find all edges that cross the surface and accumulate vertex positions
    for (int x = 0; x < nx - 1; x++) {
        for (int y = 0; y < ny - 1; y++) {
            for (int z = 0; z < nz - 1; z++) {
                // Check all 12 edges of the cube
                // Each edge connects two corners
                
                // Edge 0: (x,y,z) to (x+1,y,z)
                if (EdgeCrossesSurface(field, x, y, z, x+1, y, z, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x, y, z, x+1, y, z, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 1: (x,y,z) to (x,y+1,z)
                if (EdgeCrossesSurface(field, x, y, z, x, y+1, z, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x, y, z, x, y+1, z, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 2: (x,y,z) to (x,y,z+1)
                if (EdgeCrossesSurface(field, x, y, z, x, y, z+1, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x, y, z, x, y, z+1, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 3: (x+1,y,z) to (x+1,y+1,z)
                if (EdgeCrossesSurface(field, x+1, y, z, x+1, y+1, z, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x+1, y, z, x+1, y+1, z, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 4: (x+1,y,z) to (x+1,y,z+1)
                if (EdgeCrossesSurface(field, x+1, y, z, x+1, y, z+1, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x+1, y, z, x+1, y, z+1, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 5: (x,y+1,z) to (x+1,y+1,z)
                if (EdgeCrossesSurface(field, x, y+1, z, x+1, y+1, z, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x, y+1, z, x+1, y+1, z, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 6: (x,y+1,z) to (x,y+1,z+1)
                if (EdgeCrossesSurface(field, x, y+1, z, x, y+1, z+1, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x, y+1, z, x, y+1, z+1, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 7: (x,y,z+1) to (x+1,y,z+1)
                if (EdgeCrossesSurface(field, x, y, z+1, x+1, y, z+1, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x, y, z+1, x+1, y, z+1, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 8: (x,y,z+1) to (x,y+1,z+1)
                if (EdgeCrossesSurface(field, x, y, z+1, x, y+1, z+1, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x, y, z+1, x, y+1, z+1, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 9: (x+1,y+1,z) to (x+1,y+1,z+1)
                if (EdgeCrossesSurface(field, x+1, y+1, z, x+1, y+1, z+1, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x+1, y+1, z, x+1, y+1, z+1, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 10: (x+1,y,z+1) to (x+1,y+1,z+1)
                if (EdgeCrossesSurface(field, x+1, y, z+1, x+1, y+1, z+1, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x+1, y, z+1, x+1, y+1, z+1, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
                
                // Edge 11: (x,y+1,z+1) to (x+1,y+1,z+1)
                if (EdgeCrossesSurface(field, x, y+1, z+1, x+1, y+1, z+1, isovalue)) {
                    vec3 pos = GetEdgeIntersection(field, x, y+1, z+1, x+1, y+1, z+1, isovalue);
                    cubeToPosition[{x, y, z}] += pos;
                    cubeToCount[{x, y, z}]++;
                }
            }
        }
    }
    
    // Average the positions and create vertices
    for (auto& pair : cubeToPosition) {
        int count = cubeToCount[pair.first];
        if (count > 0) {
            vec3 avgPos = pair.second / static_cast<float>(count);
            int vertexIdx = mesh.vertices.size();
            mesh.vertices.push_back(avgPos);
            cubeToVertex[pair.first] = vertexIdx;
        }
    }
    
    // Second pass: connect vertices to form quads/triangles
    // For each cube that has a vertex, check its 6 faces and connect to neighbors
    for (int x = 0; x < nx - 1; x++) {
        for (int y = 0; y < ny - 1; y++) {
            for (int z = 0; z < nz - 1; z++) {
                auto it = cubeToVertex.find({x, y, z});
                if (it == cubeToVertex.end()) continue;
                
                int v0 = it->second;
                
                // Check +X face (right)
                if (x < nx - 2) {
                    auto itRight = cubeToVertex.find({x + 1, y, z});
                    if (itRight != cubeToVertex.end()) {
                        int v1 = itRight->second;
                        
                        // Check +Y neighbor (top)
                        auto itUp = cubeToVertex.find({x, y + 1, z});
                        auto itUpRight = cubeToVertex.find({x + 1, y + 1, z});
                        if (itUp != cubeToVertex.end() && itUpRight != cubeToVertex.end()) {
                            int v2 = itUp->second;
                            int v3 = itUpRight->second;
                            // Create quad as two triangles
                            mesh.triangles.push_back(ivec3(v0, v1, v2));
                            mesh.triangles.push_back(ivec3(v1, v3, v2));
                        }
                    }
                }
                
                // Check +Y face (top)
                if (y < ny - 2) {
                    auto itTop = cubeToVertex.find({x, y + 1, z});
                    if (itTop != cubeToVertex.end()) {
                        int v1 = itTop->second;
                        
                        // Check +Z neighbor (back)
                        auto itBack = cubeToVertex.find({x, y, z + 1});
                        auto itTopBack = cubeToVertex.find({x, y + 1, z + 1});
                        if (itBack != cubeToVertex.end() && itTopBack != cubeToVertex.end()) {
                            int v2 = itBack->second;
                            int v3 = itTopBack->second;
                            // Create quad as two triangles
                            mesh.triangles.push_back(ivec3(v0, v2, v1));
                            mesh.triangles.push_back(ivec3(v2, v3, v1));
                        }
                    }
                }
                
                // Check +Z face (back)
                if (z < nz - 2) {
                    auto itBack = cubeToVertex.find({x, y, z + 1});
                    if (itBack != cubeToVertex.end()) {
                        int v1 = itBack->second;
                        
                        // Check +X neighbor (right)
                        auto itRight = cubeToVertex.find({x + 1, y, z});
                        auto itRightBack = cubeToVertex.find({x + 1, y, z + 1});
                        if (itRight != cubeToVertex.end() && itRightBack != cubeToVertex.end()) {
                            int v2 = itRight->second;
                            int v3 = itRightBack->second;
                            // Create quad as two triangles
                            mesh.triangles.push_back(ivec3(v0, v1, v2));
                            mesh.triangles.push_back(ivec3(v1, v3, v2));
                        }
                    }
                }
            }
        }
    }
    
    return mesh;
}

// These methods are no longer used but kept for interface compatibility
vec3 SurfaceNets::CalculateGradient(const DensityField& field, int x, int y, int z)
{
    // Not used in edge-based approach
    return vec3(0, 0, 0);
}

bool SurfaceNets::CubeCrossesSurface(const DensityField& field, int x, int y, int z, double isovalue)
{
    // Check if any corner is above and any is below the isovalue
    float corners[8];
    corners[0] = field.getDensity(x,     y,     z);
    corners[1] = field.getDensity(x + 1, y,     z);
    corners[2] = field.getDensity(x + 1, y,     z + 1);
    corners[3] = field.getDensity(x,     y,     z + 1);
    corners[4] = field.getDensity(x,     y + 1, z);
    corners[5] = field.getDensity(x + 1, y + 1, z);
    corners[6] = field.getDensity(x + 1, y + 1, z + 1);
    corners[7] = field.getDensity(x,     y + 1, z + 1);
    
    bool hasAbove = false;
    bool hasBelow = false;
    
    for (int i = 0; i < 8; i++) {
        if (corners[i] >= isovalue) {
            hasAbove = true;
        } else {
            hasBelow = true;
        }
    }
    
    return hasAbove && hasBelow;
}

vec3 SurfaceNets::GetVertexPosition(const DensityField& field, int x, int y, int z, double isovalue)
{
    // Not used in edge-based approach
    float voxel_size = field.getVoxelSize();
    return vec3(
        x * voxel_size + voxel_size * 0.5,
        y * voxel_size + voxel_size * 0.5,
        z * voxel_size + voxel_size * 0.5
    );
}

