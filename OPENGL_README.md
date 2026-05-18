# OpenGL Setup and Run Test


### 1. Install GLFW

GLFW is required for window management and OpenGL context creation.

**On macOS (using Homebrew):**
```bash
brew install glfw
```

**On Linux (Ubuntu/Debian):**
```bash
sudo apt-get install libglfw3-dev
```

**On Linux (Fedora):**
```bash
sudo dnf install glfw-devel
```

**Manual Installation:**
If package manager doesn't work, download from [GLFW website](https://www.glfw.org/download.html) and follow their build instructions.

### 2. Verify GLFW Installation

Check that GLFW is installed:
```bash
# macOS
ls /usr/local/lib/libglfw* || ls /opt/homebrew/lib/libglfw*

# Linux
pkg-config --modversion glfw3
```

## Building the Project

### Step 1: Navigate to Project Directory
```bash
cd /path/to/CSCI580-FinalProject
```

### Step 2: Build the OpenGL Test
```bash
scons opengl_test
```

This will compile:
- `main_opengl_test.cpp` - Main entry point with OpenGL setup
- `opengl_renderer.cpp` - OpenGL rendering system
- `cubed_sphere.cpp` - Greg's sphere-to-mesh conversion
- `mesh.cpp` - Mesh data structure
- `box.cpp` - Bounding box utilities
- `density_field.cpp` - Density field data structure

### Step 3: Verify Build Success

You should see output like:
```
scons: Reading SConscript files ...
scons: done reading SConscript files.
scons: Building targets ...
g++ -o opengl_test [object files]
scons: done building targets.
```

And a new executable `opengl_test` should be created.

## Running the Test

### Basic Run
```bash
./opengl_test
```

### Expected Output

**Console Output:**
```
Floating Island Renderer
========================
Creating cubed sphere...
Mesh generated: [X] vertices, [Y] triangles
Starting interactive render loop...
Controls:
  W/A/S/D - Move camera forward/left/backward/right
  Space/Shift - Move camera up/down
  Mouse - Look around
  ESC - Exit
```

**Visual Output:**
- A window should open showing a voxelized sphere (made of cubes)
- Sky blue background
- Interactive 3D view

## Controls

| Input | Action |
|-------|--------|
| **W** | Move camera forward |
| **A** | Move camera left |
| **S** | Move camera backward |
| **D** | Move camera right |
| **Space** | Move camera up |
| **Left Shift** | Move camera down |
| **Mouse Movement** | Look around (rotate camera) |
| **ESC** | Exit application |

## How It Works

### Code Flow

1. **Initialization**: OpenGL renderer and GLFW window are set up
2. **Sphere Creation**: `CubedSphere` is instantiated with:
   - Center: `(0, 0, 0)`
   - Radius: `5.0`
   - Voxel size: `0.5`
3. **Mesh Generation**: `CubedSphere` internally:
   - Creates a `DensityField` grid
   - Fills it with sphere shape (1.0 inside, 0.0 outside)
   - Converts density field to cube mesh using `Generate_Mesh_From_Density_Field()`
4. **Rendering**: The mesh is rendered through OpenGL in a loop

### Key Files

- **`main_opengl_test.cpp`**: Entry point, creates `CubedSphere`, handles input, render loop
- **`opengl_renderer.cpp`**: OpenGL context, shaders, mesh rendering
- **`cubed_sphere.cpp`**: Greg's implementation that converts sphere → density field → mesh
- **`mesh.cpp`**: Contains `Generate_Mesh_From_Density_Field()` that creates cube faces



## Customization

### Change Sphere Parameters

Edit `main_opengl_test.cpp`:
```cpp
vec3 sphere_center(0, 0, 0);      // Change center position
double sphere_radius = 5.0;       // Change radius
float voxel_size = 0.5f;          // Change voxel size (smaller = more cubes)
```

### Change Camera Position

Edit `main_opengl_test.cpp`:
```cpp
renderer.camera_position = vec3(0, 5, 15);  // Change initial camera position
```

### Change Window Size

Edit `main_opengl_test.cpp`:
```cpp
if (!renderer.Initialize(1280, 720, "Floating Island Environment")) {
    // Change 1280, 720 to desired width, height
}
```

## Next Steps

1. **Multiple Spheres**: Create multiple `CubedSphere` objects and combine their meshes
2. **Volume Blending**: Use `volume_lerp` functions to blend density fields
3. **Noise Addition**: Add terrain variation using `noise.cpp` functions
4. **Island Generator**: Use `IslandGenerator` class to create complex islands
5. **Volumetric Clouds**: Add cloud rendering using ray marching

## Integration with Ray Tracer

Note: `CubedSphere` is also used in the ray tracer (`ray_tracer` executable). The same class works for both:
- **Ray Tracer**: Uses `CubedSphere` for CPU-based rendering
- **OpenGL Test**: Uses `CubedSphere` for GPU-based real-time rendering

Both use the same `GetMesh()` method to access the generated mesh.