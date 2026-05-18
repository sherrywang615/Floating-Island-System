#include <algorithm>
#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

vec3 Phong_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color = color_ambient * world.ambient_intensity * world.ambient_color;

    for (int i = 0; i < world.lights.size(); i++) {
        Light* light = world.lights[i];

        vec3 light_direction = (light->position - intersection_point).normalized();

        // for shadows, cast a ray from the intersection point to the light
        if (world.enable_shadows) {
            // start the new ray small_t * light_direction away from surface
            Ray shadow_ray(intersection_point + small_t * light_direction, light_direction);
            Hit shadow_hit = world.Closest_Intersection(shadow_ray);
            // if there's an object between the intersection point and the light, skip the contributions of this light
            if (shadow_hit.object && shadow_hit.dist < (light->position - intersection_point).magnitude()) {
                continue;
            }
        }

        // diffuse
        // diffuse intensity = dot(light_direction, normal)
        float diffuse_intensity = std::max(0.0, dot(light_direction, normal));
        // divide by distance squared to light to account for dissipation
        diffuse_intensity = diffuse_intensity / (intersection_point - light->position).magnitude_squared();
        color += color_diffuse * diffuse_intensity * light->Emitted_Light(light_direction);

        // specular
        // specular intensity = dot(reflect_direction, view_direction);
        // view_direction FROM the intersection_point TO the viewer
        vec3 view_direction = (ray.endpoint - intersection_point).normalized();
        vec3 reflect_direction = (2 * dot(light_direction, normal) * normal - light_direction).normalized();
        float specular_intensity = dot(reflect_direction, view_direction);

        if (specular_intensity > 0) {
            specular_intensity = pow(specular_intensity, specular_power);
            // divide by distance squared to light to account for dissipation
            specular_intensity = specular_intensity / (intersection_point - light->position).magnitude_squared();
            color += color_specular * specular_intensity * light->Emitted_Light(light_direction);
        }
    }
    
    
    // TODO; //calculate the phong ambient + diffuse + specular


    return color;
}