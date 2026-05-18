#include "mesh.h"
#include <fstream>
#include <string>
#include <limits>
#include <iostream>

// Consider a triangle to intersect a ray if the ray intersects the plane of the
// triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
static const double weight_tolerance = 1e-4;

// Read in a mesh from an obj file.  Populates the bounding box and registers
// one part per triangle (by setting number_parts). Given.
void Mesh::Read_Obj(const char* file)
{
    std::ifstream fin(file);
    if(!fin)
    {
        exit(EXIT_FAILURE);
    }
    std::string line;
    ivec3 e;
    vec3 v;
    box.Make_Empty();
    while(fin)
    {
        getline(fin,line);

        if(sscanf(line.c_str(), "v %lg %lg %lg", &v[0], &v[1], &v[2]) == 3)
        {
            vertices.push_back(v);
            box.Include_Point(v);
        }

        if(sscanf(line.c_str(), "f %d %d %d", &e[0], &e[1], &e[2]) == 3)
        {
            for(int i=0;i<3;i++) e[i]--;
            triangles.push_back(e);
        }
    }
    number_parts=triangles.size();
}

// spacing is size of voxels
DensityField Mesh::Generate_Density_Field_From_Mesh(float voxel_size) {
    Box box = this->Bounding_Box(-1);
    vec3 size = box.hi - box.lo;

    // make sure density field dimensions are multiples of the spacing and large enough to contain whole mesh
    // nx, ny, nz are the number of grid points in each dimension
    DensityField d(
        static_cast<int>(std::ceil(size[0] / voxel_size)) + 1,
        static_cast<int>(std::ceil(size[1] / voxel_size)) + 1,
        static_cast<int>(std::ceil(size[2] / voxel_size)) + 1,
        voxel_size
    );

    const int nx = d.getNx();
    const int ny = d.getNy();
    const int nz = d.getNz();

    for (const ivec3 tri : triangles) {
        vec3 v0 = vertices[tri[0]];
        vec3 v1 = vertices[tri[1]];
        vec3 v2 = vertices[tri[2]];

        // Compute the bounding box of the triangle
        Box triBox;
        triBox.Make_Empty();
        triBox.Include_Point(v0);
        triBox.Include_Point(v1);
        triBox.Include_Point(v2);

        // Determine the grid cells that the triangle's bounding box overlaps
        ivec3 minCell, maxCell;
        for (int i = 0; i < 3; i++) {
            minCell[i] = std::max(0, static_cast<int>(std::floor((triBox.lo[i] - box.lo[i]) / voxel_size)));
            maxCell[i] = std::min((i == 0 ? nx : (i == 1 ? ny : nz)) - 1,
                                  static_cast<int>(std::floor((triBox.hi[i] - box.lo[i]) / voxel_size)));
        }

        // For each voxel in the bounding box, check if the triangle actually intersects it
        for (int i = minCell[0]; i <= maxCell[0]; i++) {
            for (int j = minCell[1]; j <= maxCell[1]; j++) {
                for (int k = minCell[2]; k <= maxCell[2]; k++) {
                    // Compute voxel center
                    vec3 voxelCenter;
                    voxelCenter[0] = box.lo[0] + (i + 0.5f) * voxel_size;
                    voxelCenter[1] = box.lo[1] + (j + 0.5f) * voxel_size;
                    voxelCenter[2] = box.lo[2] + (k + 0.5f) * voxel_size;
                    
                    // Check if voxel center is close to triangle plane
                    vec3 normal = cross(v1 - v0, v2 - v0).normalized();
                    double distToPlane = std::abs(dot(normal, voxelCenter - v0));
                    
                    if (distToPlane < voxel_size * 0.87) { // sqrt(3)/2 * voxel_size
                        // Project voxel center onto triangle plane
                        vec3 projected = voxelCenter - normal * dot(normal, voxelCenter - v0);
                        
                        // Compute barycentric coordinates
                        vec3 v0v1 = v1 - v0;
                        vec3 v0v2 = v2 - v0;
                        vec3 v0p = projected - v0;
                        
                        float d00 = dot(v0v1, v0v1);
                        float d01 = dot(v0v1, v0v2);
                        float d02 = dot(v0v1, v0p);
                        float d11 = dot(v0v2, v0v2);
                        float d12 = dot(v0v2, v0p);
                        
                        float invDenom = 1.0f / (d00 * d11 - d01 * d01);
                        float u = (d11 * d02 - d01 * d12) * invDenom;
                        float v = (d00 * d12 - d01 * d02) * invDenom;
                        
                        // Check if point is in or near triangle
                        if (u >= -0.5f && v >= -0.5f && (u + v) <= 1.5f) {
                            d.setDensity(i, j, k, 1.0f);
                        }
                    }
                }
            }
        }
    }
    return d;
}

void Mesh::Generate_Mesh_From_Density_Field(const DensityField& d, float iso_value, vec3 field_start) {

    vertices.clear();
    triangles.clear();
    
    float voxel_size = d.getVoxelSize();
    
    for (int i = 0; i < d.getNx(); i++) {
        for (int j = 0; j < d.getNy(); j++) {
            for (int k = 0; k < d.getNz(); k++) {
                float density = d.getDensity(i, j, k);
                if (density >= iso_value) {
                    // Grid point (i,j,k) represents the CENTER of the voxel
                    vec3 voxelCenter;
                    voxelCenter[0] = field_start[0] + i * voxel_size;
                    voxelCenter[1] = field_start[1] + j * voxel_size;
                    voxelCenter[2] = field_start[2] + k * voxel_size;
                    
                    // The voxel corner is offset by -0.5 * voxel_size from the center
                    vec3 topLeftVertex = voxelCenter - vec3(0.5 * voxel_size, 0.5 * voxel_size, 0.5 * voxel_size);
                    
                    // Create a cube at this position
                    // if i == 0 or d.getDensity(i-1,j,k) < iso_value, create left face
                    if (i == 0 || d.getDensity(i-1,j,k) < iso_value) {
                        int v0 = vertices.size();
                        vertices.push_back(topLeftVertex);
                        vertices.push_back(topLeftVertex + vec3(0, voxel_size, 0));
                        vertices.push_back(topLeftVertex + vec3(0, 0, voxel_size));
                        vertices.push_back(topLeftVertex + vec3(0, voxel_size, voxel_size));
                        triangles.push_back(ivec3(v0, v0+2, v0+1));
                        triangles.push_back(ivec3(v0+1, v0+2, v0+3));
                    }
                    // if j == 0 or d.getDensity(i,j-1,k) < iso_value, create bottom face
                    if (j == 0 || d.getDensity(i,j-1,k) < iso_value) {
                        int v0 = vertices.size();
                        vertices.push_back(topLeftVertex);
                        vertices.push_back(topLeftVertex + vec3(voxel_size, 0, 0));
                        vertices.push_back(topLeftVertex + vec3(0, 0, voxel_size));
                        vertices.push_back(topLeftVertex + vec3(voxel_size, 0, voxel_size));
                        triangles.push_back(ivec3(v0, v0+2, v0+1));
                        triangles.push_back(ivec3(v0+1, v0+2, v0+3));
                    }
                    // if k == 0 or d.getDensity(i,j,k-1) < iso_value, create back face
                    if (k == 0 || d.getDensity(i,j,k-1) < iso_value) {
                        int v0 = vertices.size();
                        vertices.push_back(topLeftVertex);
                        vertices.push_back(topLeftVertex + vec3(voxel_size, 0, 0));
                        vertices.push_back(topLeftVertex + vec3(0, voxel_size, 0));
                        vertices.push_back(topLeftVertex + vec3(voxel_size, voxel_size, 0));
                        triangles.push_back(ivec3(v0, v0+2, v0+1));
                        triangles.push_back(ivec3(v0+1, v0+2, v0+3));
                    }
                    // if i == d.getNx()-1 or d.getDensity(i+1,j,k) < iso_value, create right face
                    if (i == d.getNx()-1 || (i < d.getNx()-1 && d.getDensity(i+1,j,k) < iso_value)) {
                        int v0 = vertices.size();
                        vertices.push_back(topLeftVertex + vec3(voxel_size, 0, 0));
                        vertices.push_back(topLeftVertex + vec3(voxel_size, voxel_size, 0));
                        vertices.push_back(topLeftVertex + vec3(voxel_size, 0, voxel_size));
                        vertices.push_back(topLeftVertex + vec3(voxel_size, voxel_size, voxel_size));
                        triangles.push_back(ivec3(v0, v0+1, v0+2));
                        triangles.push_back(ivec3(v0+1, v0+3, v0+2));
                    }
                    // if j == d.getNy()-1 or d.getDensity(i,j+1,k) < iso_value, create top face
                    if (j == d.getNy()-1 || (j < d.getNy()-1 && d.getDensity(i,j+1,k) < iso_value)) {
                        int v0 = vertices.size();
                        vertices.push_back(topLeftVertex + vec3(0, voxel_size, 0));
                        vertices.push_back(topLeftVertex + vec3(voxel_size, voxel_size, 0));
                        vertices.push_back(topLeftVertex + vec3(0, voxel_size, voxel_size));
                        vertices.push_back(topLeftVertex + vec3(voxel_size, voxel_size, voxel_size));
                        triangles.push_back(ivec3(v0, v0+1, v0+2));
                        triangles.push_back(ivec3(v0+1, v0+3, v0+2));
                    }
                    // if k == d.getNz()-1 or d.getDensity(i,j,k+1) < iso_value, create front face
                    if (k == d.getNz()-1 || (k < d.getNz()-1 && d.getDensity(i,j,k+1) < iso_value)) {
                        int v0 = vertices.size();
                        vertices.push_back(topLeftVertex + vec3(0, 0, voxel_size));
                        vertices.push_back(topLeftVertex + vec3(voxel_size, 0, voxel_size));
                        vertices.push_back(topLeftVertex + vec3(0, voxel_size, voxel_size));
                        vertices.push_back(topLeftVertex + vec3(voxel_size, voxel_size, voxel_size));
                        triangles.push_back(ivec3(v0, v0+1, v0+2));
                        triangles.push_back(ivec3(v0+1, v0+3, v0+2));
                    }
                }
            }
        }
    }
}

// Check for an intersection against the ray.  See the base class for details.
Hit Mesh::Intersection(const Ray& ray, int part) const
{
    Hit closest_hit = {0,0, -1};
    double closest_dist = std::numeric_limits<double>::max();
    double dist;
    if (part >= 0) {
        if (Intersect_Triangle(ray, part, dist)) {
            closest_hit.dist = dist;
            closest_hit.part = part;
            closest_hit.object = this;
            return closest_hit;
        }
        return closest_hit;
    }
    else {
        for (int i=0; i<triangles.size(); i++) {
            if (Intersect_Triangle(ray, i, dist) && dist < closest_dist && dist > small_t) {
                closest_dist = dist;
                closest_hit.dist = dist;
                closest_hit.part = i;
                closest_hit.object = this;
            }
        }
        return closest_hit;
    }

    TODO; //implement Mesh+ray Intersection

    return closest_hit;
}

// Compute the normal direction for the triangle with index part.
vec3 Mesh::Normal(const vec3& point, int part) const
{
    assert(part>=0);
    
    if (part >= (int)triangles.size()) {
        return vec3(0, 1, 0); // Return default normal
    }

    ivec3 current_triangle = triangles[part];

    vec3 v0 = vertices[current_triangle[0]];
    vec3 v1 = vertices[current_triangle[1]];
    vec3 v2 = vertices[current_triangle[2]];
    vec3 tri_normal = cross(v1 - v0, v2 - v0).normalized();

    // TODO; //implement tri normal calculation

    return tri_normal;
}

// This is a helper routine whose purpose is to simplify the implementation
// of the Intersection routine.  It should test for an intersection between
// the ray and the triangle with index tri.  If an intersection exists,
// record the distance and return true.  Otherwise, return false.
// This intersection should be computed by determining the intersection of
// the ray and the plane of the triangle.  From this, determine (1) where
// along the ray the intersection point occurs (dist) and (2) the barycentric
// coordinates within the triangle where the intersection occurs.  The
// triangle intersects the ray if dist>small_t and the barycentric weights are
// larger than -weight_tolerance.  The use of small_t avoid the self-shadowing
// bug, and the use of weight_tolerance prevents rays from passing in between
// two triangles.
bool Mesh::Intersect_Triangle(const Ray& ray, int tri, double& dist) const
{
    if (tri < 0 || tri >= (int)triangles.size()) {
        dist = 0.0;
        return false;
    }
    ivec3 points = triangles[tri];
    vec3 v0 = vertices[points[0]];
    vec3 v1 = vertices[points[1]];
    vec3 v2 = vertices[points[2]];

    vec3 normal = cross(v1 - v0, v2 - v0).normalized();

    // check if ray intersects the plane of the triangle
    float numerator = dot(normal, v0 - ray.endpoint);
    float denominator = dot(normal, ray.direction);
    // if ray and plane are parallel
    if (denominator < small_t && denominator > -small_t) {
        dist = 0.0;
        return false;
    }
    double new_dist = numerator / denominator;
    // if intersection is basically at zero or negative (behind the ray)
    if (new_dist < small_t) {
        dist = 0.0;
        return false;
    }
    // ray intersects plane
    vec3 intersection_point = ray.endpoint + new_dist * ray.direction;
    // calculate barycentric coordinates
    // P = alpha * v0 + beta * v1 + gamma * v2
    // alpha + beta + gamma = 1
    // if any of the coordinates are < -weight_tolerance or > 1 + weight_tolerance, no intersection
    float area_v0v1v2 = dot(normal, cross(v1 - v0, v2 - v0));
    if (fabs(area_v0v1v2) < small_t) {
        dist = 0.0;
        return false; // Degenerate triangle
    }
    float area_pv0v1 = dot(normal, cross(v0 - intersection_point, v1 - intersection_point));
    float area_pv1v2 = dot(normal, cross(v1 - intersection_point, v2 - intersection_point));
    float area_pv2v0 = dot(normal, cross(v2 - intersection_point, v0 - intersection_point));
    float alpha = area_pv0v1 / area_v0v1v2;
    float beta = area_pv1v2 / area_v0v1v2;
    float gamma = area_pv2v0 / area_v0v1v2;
    if (alpha >= -weight_tolerance && beta >= -weight_tolerance && gamma >= -weight_tolerance &&
        alpha <= 1 + weight_tolerance && beta <= 1 + weight_tolerance && gamma <= 1 + weight_tolerance
        && (fabs(alpha + beta + gamma - 1) <= weight_tolerance)) {
        // intersection point is inside the triangle
        dist = new_dist;
        return true;
    }

    // TODO; //implement tri+ray intersection

    dist = 0.0;
    return false;
}

// Compute the bounding box.  Return the bounding box of only the triangle whose
// index is part.
Box Mesh::Bounding_Box(int part) const
{
    Box b;
    assert(part>=-1);
    if (part == -1) {
        for (const ivec3& tri : triangles) {
            vec3 v0 = vertices[tri[0]];
            vec3 v1 = vertices[tri[1]];
            vec3 v2 = vertices[tri[2]];
            b.Include_Point(v0);
            b.Include_Point(v1);
            b.Include_Point(v2);
        }
    }
    else if (part <= (int)triangles.size()) {
        ivec3 current_triangle = triangles[part];
        vec3 v0 = vertices[current_triangle[0]];
        vec3 v1 = vertices[current_triangle[1]];
        vec3 v2 = vertices[current_triangle[2]];
        b.Include_Point(v0);
        b.Include_Point(v1);
        b.Include_Point(v2);
    }
    return b;
}
