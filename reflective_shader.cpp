#include "reflective_shader.h"
#include "ray.h"
#include "render_world.h"

vec3 Reflective_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color = shader->Shade_Surface(ray, intersection_point, normal, recursion_depth);

    if (recursion_depth <= 1) {
        return (1 - reflectivity) * color;
    }

    vec3 reflect_direction = (2 * dot(-ray.direction, normal) * normal + ray.direction).normalized();
    Ray reflect_ray(intersection_point + small_t * reflect_direction, reflect_direction);
    vec3 reflect_color = world.Cast_Ray(reflect_ray, recursion_depth - 1);
    color = (1 - reflectivity) * color + reflectivity * reflect_color;


    // TODO; //recursively cast ray untill recursion_depth is reached;
    //combine results into color;

    return color;
}
