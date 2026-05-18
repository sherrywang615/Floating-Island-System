#include <algorithm>
#include "hierarchy.h"

// Reorder the entries vector so that adjacent entries tend to be nearby.
// You may want to implement box.cpp first.
void Hierarchy::Reorder_Entries()
{
    if(!entries.size()) return;
    TODO;//sort box entries to help speed up tracing
}

// Populate tree from entries.
void Hierarchy::Build_Tree()
{
    if(!entries.size()) return;
    TODO;//build bound box heirarchy
}

// Return a list of candidates (indices into the entries list) whose
// bounding boxes intersect the ray.
void Hierarchy::Intersection_Candidates(const Ray& ray, std::vector<int>& candidates) const
{
    TODO; //find all bounding boxes along ray
}
