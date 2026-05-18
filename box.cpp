#include <limits>
#include "box.h"

// Return whether the ray intersects this box.
bool Box::Intersection(const Ray& ray) const
{
    TODO;
    return true;
}

// Compute the smallest box that contains both *this and bb.
Box Box::Union(const Box& bb) const
{
    Box box;
    TODO;
    return box;
}

// Enlarge this box (if necessary) so that pt also lies inside it.
void Box::Include_Point(const vec3& pt)
{
    // if pt.x is less than lo.x, set lo.x to pt.x
    // if pt.x is greater than hi.x, set hi.x to pt.x
    // do the same for y and z
    if (pt[0] < lo[0]) lo[0] = pt[0];
    if (pt[0] > hi[0]) hi[0] = pt[0];
    if (pt[1] < lo[1]) lo[1] = pt[1];
    if (pt[1] > hi[1]) hi[1] = pt[1];
    if (pt[2] < lo[2]) lo[2] = pt[2];
    if (pt[2] > hi[2]) hi[2] = pt[2];
    // TODO;
}

// Create a box to which points can be correctly added using Include_Point.
void Box::Make_Empty()
{
    lo.fill(std::numeric_limits<double>::infinity());
    hi=-lo;
}
