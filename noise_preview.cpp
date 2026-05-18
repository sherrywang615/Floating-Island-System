#include <fstream>
#include <iostream>
#include "noise.h"

int main()
{
    const int   width  = 512;
    const int   height = 512;
    const float scale  = 8.0f;
    const float zSlice = 0.0f;

    std::ofstream out("noise_preview.ppm");
    if (!out)
    {
        std::cerr << "Failed to open noise_preview.ppm for writing\n";
        return 1;
    }

    out << "P3\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float fx = (static_cast<float>(x) / width)  * scale;
            float fy = (static_cast<float>(y) / height) * scale;

            float n = noise::perlin3D_fbm(fx, fy, zSlice, 5);

            float v = 0.5f * (n + 1.0f);
            if (v < 0.0f) v = 0.0f;
            if (v > 1.0f) v = 1.0f;

            int c = static_cast<int>(v * 255.0f + 0.5f);

            out << c << ' ' << c << ' ' << c << ' ';
        }
        out << '\n';
    }

    out.close();
    std::cout << "Wrote noise_preview.ppm ("
              << width << "x" << height << ")\n";

    return 0;
}
