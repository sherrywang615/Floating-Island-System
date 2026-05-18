# Windows Build Guide for OpenGL Test

This guide explains how to build and run `opengl_test` on Windows.

## Prerequisites

### 1. Install Python 3
- Download from [python.org](https://www.python.org/downloads/)
- Make sure to add Python to PATH during installation
- Verify: `python --version`

### 2. Install SCons
```cmd
pip install scons
```

### 3. Install GLFW

**Option A: Using vcpkg (Recommended)**
```cmd
# Install vcpkg (if not already installed)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install GLFW
.\vcpkg install glfw3:x64-windows
```

**Option B: Manual Installation**
1. Download GLFW from [glfw.org](https://www.glfw.org/download.html)
2. Extract to one of these locations:
   - `C:\GLFW` (system-wide)
   - `C:\Program Files\GLFW` (system-wide, requires admin)
   - `%USERPROFILE%\GLFW` (user-specific, no admin needed)
3. Make sure `include/GLFW` and `lib` directories exist in the chosen location

**Option C: Using MSYS2**
```bash
pacman -S mingw-w64-x86_64-glfw
```

### 4. Install a C++ Compiler

**Option A: Visual Studio (Recommended)**
- Download [Visual Studio Community](https://visualstudio.microsoft.com/)
- Install "Desktop development with C++" workload
- Includes MSVC compiler (`cl.exe`)

**Option B: MinGW-w64**
- Download from [mingw-w64.org](https://www.mingw-w64.org/)
- Or use MSYS2: `pacman -S mingw-w64-x86_64-gcc`

## Building

### Using SCons (Cross-platform)

```cmd
# Build opengl_test
scons opengl_test

# Build ray_tracer
scons ray_tracer
```

The SConstruct file will automatically:
- Detect Windows platform
- Search for GLFW in these locations (in order):
  - User's home directory: `~/vcpkg/installed/x64-windows` or `~/GLFW`
  - System-wide: `C:\vcpkg\installed\x64-windows`, `C:\vcpkg\installed\x86-windows`
  - Common install paths: `C:\Program Files\GLFW`, `C:\GLFW`
- Link against `opengl32.lib` and `glu32.lib`
- Link against `glfw3.lib`

### Using Visual Studio (Alternative)

If SCons doesn't work, you can build manually:

```cmd
cl /EHsc /std:c++11 /O2 /W3 ^
   main_opengl_test.cpp ^
   opengl_renderer.cpp ^
   mesh.cpp box.cpp cubed_sphere.cpp ^
   density_field.cpp lerped_spheres.cpp surface_nets.cpp ^
   /I"C:\path\to\glfw\include" ^
   /link /LIBPATH:"C:\path\to\glfw\lib" ^
   opengl32.lib glu32.lib glfw3.lib ^
   /OUT:opengl_test.exe
```

### Using MinGW/MSYS2

```bash
g++ -std=c++11 -O3 ^
    main_opengl_test.cpp ^
    opengl_renderer.cpp ^
    mesh.cpp box.cpp cubed_sphere.cpp ^
    density_field.cpp lerped_spheres.cpp surface_nets.cpp ^
    -I/usr/include ^
    -L/usr/lib ^
    -lglfw3 -lopengl32 -lglu32 ^
    -o opengl_test.exe
```

## Running

```cmd
opengl_test.exe
```

## Troubleshooting

### GLFW Not Found
- Make sure GLFW is installed in one of these locations (searched in order):
  - **User-specific paths:**
    - `%USERPROFILE%\vcpkg\installed\x64-windows` (vcpkg in user directory)
    - `%USERPROFILE%\GLFW` (GLFW in user directory)
  - **System-wide paths:**
    - `C:\vcpkg\installed\x64-windows` (vcpkg 64-bit)
    - `C:\vcpkg\installed\x86-windows` (vcpkg 32-bit)
    - `C:\Program Files\GLFW`
    - `C:\GLFW`
- The build system will automatically search these paths
- If GLFW is in a different location, you can:
  - Add it to your system PATH
  - Or modify `SConstruct` to include your custom path

### OpenGL Context Creation Failed
- Update your graphics drivers
- Try a different OpenGL version in `opengl_renderer.cpp` (lines 44-48)

### Missing DLLs
- If using vcpkg, copy DLLs to executable directory:
  ```cmd
  copy C:\vcpkg\installed\x64-windows\bin\glfw3.dll .
  ```

## Platform-Specific Notes

### Windows OpenGL Support
- Windows includes OpenGL 1.1 by default (`opengl32.lib`)
- For newer OpenGL features, you may need to use extensions or update drivers
- The code uses OpenGL 3.3 compatibility profile for best compatibility

### GLFW Context
- Windows uses OpenGL 3.3 compatibility profile
- This supports both modern and legacy OpenGL features
- Immediate mode (glBegin/glEnd) works in compatibility profile

## File Changes for Windows

The following files were updated for Windows support:

1. **opengl_renderer.h**: Added Windows-specific OpenGL includes
   ```cpp
   #elif defined(_WIN32)
   #include <windows.h>
   #include <GL/gl.h>
   #include <GL/glu.h>
   ```

2. **opengl_renderer.cpp**: Added Windows-specific GLFW context hints
   ```cpp
   #elif defined(_WIN32)
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
   ```

3. **SConstruct**: Added Windows build configuration
   - Automatic GLFW path detection
   - Windows-specific library linking (`opengl32.lib`, `glu32.lib`, `glfw3.lib`)

