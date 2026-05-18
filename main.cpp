#include "render_world.h"
#include "object.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdio>

typedef unsigned int Pixel;

char *optarg = NULL;  // Global variable to hold the argument for the current option
int optind = 1;       // Index of the next argument to process

int getopt(int argc, char *const argv[], const char *optstring) {
    if (optind >= argc) {
        return -1;  // No more arguments
    }

    // Get the current argument
    char *current_arg = argv[optind];
    if (current_arg[0] != '-' || current_arg[1] == '\0') {
        return -1;  // Non-option argument or invalid option
    }

    // Move to the next argument (after '-')
    current_arg++; 
    optind++;

    // Get the option character
    char option = *current_arg;
    const char *opt = strchr(optstring, option);
    if (opt == NULL) {
        return '?'; // Invalid option
    }

    // Check if the option requires an argument (indicated by ':')
    if (opt[1] == ':') {
        if (current_arg[1] != '\0') {
            // Argument is part of the same string (e.g., -bvalue)
            optarg = current_arg + 1;
            return option;
        } else if (optind < argc) {
            // Argument is in the next argument
            optarg = argv[optind++];
            return option;
        } else {
            return ':'; // Missing argument for this option
        }
    }

    // Option doesn't require an argument
    optarg = NULL;
    return option;
}

bool Read_ppm(Pixel *& data, int& width, int& height, const char* filename) {
    std::ifstream file(filename);
    
    if (!file) {
        std::cerr << "Error: Could not open file for reading." << std::endl;
        return false;
    }

    std::string line;
    
    // Read the PPM file type
    std::getline(file, line);
    if (line != "P3") {
        std::cerr << "Error: Unsupported PPM format." << std::endl;
        return false;
    }

    // Read image dimensions
    std::getline(file, line);
    std::istringstream dimensions(line);
    dimensions >> width >> height;

    // Read maximum color value
    std::getline(file, line);
    int maxVal;
    std::istringstream maxValStream(line);
    maxValStream >> maxVal;

    if (maxVal != 255) {
        std::cerr << "Error: Unsupported maximum color value. Only 255 is supported." << std::endl;
        return false;
    }

    // Read pixel data
    data = new Pixel[width * height];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
        unsigned int red, green, blue;
        file >> red >> green >> blue;
        data[(height - y - 1) * width + x] = (red << 24) | (green << 16) | (blue << 8);
    }
    }

    file.close();
    return true;
}

void Dump_ppm(Pixel* data,int width,int height,const char* filename)
{
    std::ofstream file(filename, std::ios::binary);
    std::cout << "Dumping output to " << filename << std::endl;
    
    if (!file) {
        std::cerr << "Error: Could not open file for writing." << std::endl;
        return;
    }

    // Write PPM header
    file << "P3\n"; // PPM type (P3 for ASCII format)
    file << width << " " << height << "\n"; // Image dimensions
    file << "255\n"; // Maximum color value

    // Write pixel data
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned int pixel = data[(height - y - 1) * width + x];
            unsigned char red   = (pixel >> 24) & 0xFF;
            unsigned char green = (pixel >> 16) & 0xFF;
            unsigned char blue  = (pixel >> 8) & 0xFF;

            file << static_cast<int>(red)   << " "
                << static_cast<int>(green) << " "
                << static_cast<int>(blue)  << "\n";
        }
    }

    file.close();
}

/*

  Usage: ./ray_tracer -i <test-file> [ -s <solution-file> ] [ -o <stats-file> ] [ -x <debug-x-coord> -y <debug-y-coord> ]

  Examples:

  ./ray_tracer -i 00.txt

  Renders the scene described by 00.txt.  Dumps the result to output.ppm.

  ./ray_tracer -i 00.txt -s 00.ppm

  Renders the scene described by 00.txt.  Dumps the result to output.ppm.
  Compares this to the solution in 00.ppm and dumps the error to diff.ppm.
  Outputs a measure of the error.

  ./ray_tracer -i 00.txt -x 123 -y 234

  The -x and -y flags give you the opportunity to print out lots of detailed
  information about the rendering of a single pixel.  This allows you to be
  verbose about a pixel of interest without printing this information for every
  pixel.  For many of the scenes, there is a pixel trace on the project page
  detailing the results of various computations (intersections, shading, etc.)
  for one specially chosen pixel.

  The -o flag is used by the grading script.  It causes the results of your ray
  tracer to be printed to a file rather than to the standard output.  This
  prevents the grading script from getting confused by debugging output.

 */

// Indicates that we are debugging one pixel; can be accessed everywhere.
bool debug_pixel=false;

// This can be used to quickly disable the hierarchy for testing purposes.
// Though this is not required, it is highly recommended that you implement
// this, as it will make debugging your hierarchy much easier.
bool disable_hierarchy=false;

void Usage(const char* exec)
{
    std::cerr<<"Usage: "<<exec<<" -i <test-file> [ -s <solution-file> ] [ -o <stats-file> ] [ -x <debug-x-coord> -y <debug-y-coord> ]"<<std::endl;
    exit(1);
}

void Parse(Render_World& world,int& width,int& height,const char* test_file);
void Dump_png(Pixel* data,int width,int height,const char* filename);
void Read_png(Pixel*& data,int& width,int& height,const char* filename);

int main(int argc, char** argv)
{
    const char* solution_file = 0;
    const char* input_file = 0;
    const char* statistics_file = 0;
    int test_x=-1, test_y=-1;

    // Parse commandline options
    while(1)
    {
        int opt = getopt(argc, argv, "s:i:m:o:x:y:h");
        if(opt==-1) break;
        switch(opt)
        {
            case 's': solution_file = optarg; break;
            case 'i': input_file = optarg; break;
            case 'o': statistics_file = optarg; break;
            case 'x': test_x = atoi(optarg); break;
            case 'y': test_y = atoi(optarg); break;
            case 'h': disable_hierarchy=true; break;
        }
    }
    if(!input_file) Usage(argv[0]);

    int width=0;
    int height=0;
    Render_World world;
    
    // Parse test scene file
    Parse(world,width,height,input_file);

    // Render the image
    world.Render();

    // For debugging.  Render only the pixel specified on the commandline.
    // Useful for printing out information about a single pixel.
    if(test_x>=0 && test_y>=0)
    {
        // Set a global variable to indicate that we are debugging one pixel.
        // This way you can do: if(debug_pixel) cout<<lots<<of<<stuff<<here<<endl;
        debug_pixel = true;

        // Render just the pixel we are debugging
        world.Render_Pixel(ivec2(test_x,test_y));

        // Mark the pixel we are testing green in the output image.
        world.camera.Set_Pixel(ivec2(test_x,test_y),0x00ff00ff);
    }

    // Save the rendered image to disk
    Dump_ppm(world.camera.colors,width,height,"output.ppm");

    // If a solution is specified, compare against it.
    if(solution_file)
    {
        int width2 = 0, height2 = 0;
        Pixel* data_sol = 0;

        // Read solution from disk
        Read_ppm(data_sol,width2,height2,solution_file);
        assert(width==width2);
        assert(height==height2);

        // For each pixel, check to see if it matches solution
        double error = 0, total = 0;
        for(int i=0; i<height*width; i++)
        {
            vec3 a=From_Pixel(world.camera.colors[i]);
            vec3 b=From_Pixel(data_sol[i]);
            for(int c=0; c<3; c++)
            {
                double e = fabs(a[c]-b[c]);
                error += e;
                total++;
                b[c] = e;
            }
            data_sol[i]=Pixel_Color(b);
        }

        // Output information on how well it matches. Optionally save to file
        // to avoid getting confused by debugging print statements.
        FILE* stats_file = stdout;
        if(statistics_file) stats_file = fopen(statistics_file, "w");
        fprintf(stats_file,"diff: %.2f\n",error/total*100);
        if(statistics_file) fclose(stats_file);

        // Output images showing the error that was computed to aid debugging
        Dump_ppm(data_sol,width,height,"diff.ppm");
        delete [] data_sol;
    }

    return 0;
}

