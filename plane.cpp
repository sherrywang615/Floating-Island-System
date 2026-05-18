#include "plane.h"
#include "ray.h"
#include <cfloat>
#include <limits>

// Intersect with the half space defined by the plane.  The plane's normal
// points outside.  If the ray starts on the "inside" side of the plane, be sure
// to record a hit with t=0 as the first entry in hits.
Hit Plane::Intersection(const Ray& ray, int part) const
{
    // Check for an intersection against the ray.  If there was an
    // intersection, record the distance to the first intersection.
    // If an intersection was found, the object structure member
    // should be set to this.  If no intersection was found, the
    // object member should be set to NULL.
    // Do not return intersections where dist<small_t.

    // check if ray starts inside the half space
    vec3 ray_to_plane = ray.endpoint - x1;
    // if we dot the ray_to_plane vector with the normal,
    // we can see if the resulting vector is positive or negative
    // if positive, the ray starts outside the half space
    // if negative, the ray starts inside the half space
    // if zero, the ray starts exactly on the plane
    float signed_distance = dot(normal, ray_to_plane);
    if (signed_distance < small_t) {
        return {this, 0.0, part};
    }
    else {
        // ray starts outside the half space, so we need to check for intersection
        float numerator = dot(normal, x1 - ray.endpoint);
        float denominator = dot(normal, ray.direction);
        // if ray and plane are parallel
        if (denominator < small_t && denominator > -small_t) {
            return {nullptr, 0, part};
        }
        float dist = numerator / denominator;
        // if intersection is basically at zero or negative (behind the ray)
        if (dist < small_t) {
            return {nullptr, 0, part};
        }
        return {this, dist, part};
    }



    // TODO; //calculate ray+plane intersection

    return {nullptr, 0, part};
}

vec3 Plane::Normal(const vec3& point, int part) const
{
    //normal is part of the plane so this one is a gimme
    return normal;
}

// There is not a good answer for the bounding box of an infinite object.
// The safe thing to do is to return a box that contains everything.
Box Plane::Bounding_Box(int part) const
{
    //also a gimme
    Box b;
    b.hi.fill(std::numeric_limits<double>::max());
    b.lo=-b.hi;
    return b;
}
