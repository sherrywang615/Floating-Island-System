#pragma once

// Simple 3D Perlin noise utilities.

namespace noise
{
    // Basic 3D Perlin noise in range ~[-1, 1]
    float perlin3D(float x, float y, float z);

    float perlin3D_fbm(float x, float y, float z,
                       int   octaves     = 5,
                       float lacunarity  = 2.0f,
                       float gain        = 0.5f);
}
