#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"
#include <iostream>

//#include <iostream>
//using namespace std;

extern bool disable_hierarchy;

Render_World::Render_World()
    :background_shader(0),ambient_intensity(0),enable_shadows(true),
    recursion_depth_limit(3)
{}

Render_World::~Render_World()
{
    delete background_shader;
    for(size_t i=0;i<objects.size();i++) delete objects[i];
    for(size_t i=0;i<lights.size();i++) delete lights[i];
}

// Find and return the Hit structure for the closest intersection.  Be careful
// to ensure that hit.dist>=small_t.
Hit Render_World::Closest_Intersection(const Ray& ray)
{
    Hit closest_hit;
    closest_hit = {nullptr, 0, 0};
    double min_t = std::numeric_limits<double>::max();

    for(size_t i = 0; i < objects.size(); i++) {
        Hit hit = objects[i]->Intersection(ray, -1);
        if (hit.dist >= small_t && hit.dist < min_t) {
            min_t = hit.dist;
            closest_hit = hit;
        }
    }
    // TODO; //find nearest intersection along ray

    return closest_hit;
}

// set up the initial view ray and call
void Render_World::Render_Pixel(const ivec2& pixel_index)
{
    vec3 pixel_position = camera.World_Position(pixel_index);
    Ray ray(camera.position, pixel_position - camera.position); // direction gets normalized in Ray constructor
    // TODO; //set up ray start and direction
    vec3 color=Cast_Ray(ray,recursion_depth_limit);
    camera.Set_Pixel(pixel_index,Pixel_Color(color));
}

void Render_World::Render()
{
    if(!disable_hierarchy)
        Initialize_Hierarchy(); //ignore this untill the last 2 test cases

    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));
}

// cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth)
{
    vec3 color;
    
    Hit hit = Closest_Intersection(ray);
    // hit.object will be nullptr if no intersection occurred
    if (hit.object) {
        // every hit points to an object, every object points to a material_shader
        vec3 pointOfIntersection = ray.Point(hit.dist);
        vec3 normalAtIntersection = hit.object->Normal(pointOfIntersection, hit.part);
        color = hit.object->material_shader->Shade_Surface(ray, pointOfIntersection, normalAtIntersection, recursion_depth);
    }
    else {
        // using dummy intersection point and normal since they are not used in the background shader
        color = background_shader->Shade_Surface(ray, vec3(0,0,0), vec3(0,0,0), recursion_depth);
    }
    // TODO; //fill color with casted ray result;

    return color;
}

void Render_World::Initialize_Hierarchy()
{
    TODO; // Fill in hierarchy.entries; there should be one entry for
    // each part of each object.

    hierarchy.Reorder_Entries();
    hierarchy.Build_Tree();
}
