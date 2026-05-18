#include "sphere.h"
#include "ray.h"

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray& ray, int part) const
{
    // TODO; //calculate ray sphere intersection



    float a = pow(2 * dot(ray.direction, ray.endpoint - center), 2);;
    float b = 4 * ray.direction.magnitude_squared();
    float c = (ray.endpoint - center).magnitude_squared() - radius * radius;
    float discriminant = a - (b * c);

    float d = -2 * dot(ray.direction, ray.endpoint - center);
    float e = 2 * ray.direction.magnitude_squared();

    // if discriminant is positive, two intersections
    if (discriminant > small_t) {
        float dist1 = (d + sqrt(discriminant)) / e;
        float dist2 = (d - sqrt(discriminant)) / e;
        float closest_dist = std::min(dist1, dist2);
        // if both distances are less than small_t, no intersection
        if (dist1 < small_t && dist2 < small_t) {
            return {nullptr, 0.0, part};
        }
        // if one distance is less than small_t, return the other distance
        else if (dist1 < small_t && dist2 >= small_t) {
            closest_dist = dist2;
        }
        else if (dist1 >= small_t && dist2 < small_t) {
            closest_dist = dist1;
        }
        return {this, closest_dist, part};
    }
    // if discriminant is basically zero, one intersection
    else if (discriminant < small_t && discriminant > -small_t) {
        float dist = d / e;
        if (dist < small_t) {
            return {nullptr, 0.0, part};
        }
        return {this, dist, part};
    }

    return {nullptr, 0.0, part};
}

vec3 Sphere::Normal(const vec3& point, int part) const
{
    vec3 normal;

    normal = (point - center).normalized();

    // TODO; //calculate Sphere surface normal at point

    return normal;
}

Box Sphere::Bounding_Box(int part) const
{
    Box box;
    TODO; // calculate bounding box
    return box;
}
