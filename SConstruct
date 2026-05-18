import os
import platform

env = Environment(ENV = os.environ)

env.Append(CXXFLAGS=["-std=c++11","-g","-Wall","-O3"])

# Add OpenGL and GLFW support (platform-specific)
if platform.system() == "Darwin":
    # macOS: Check for Homebrew installation locations (Apple Silicon vs Intel)
    homebrew_paths = ["/opt/homebrew", "/usr/local"]
    glfw_path = None
    for path in homebrew_paths:
        if os.path.exists(f"{path}/lib/libglfw.dylib"):
            glfw_path = path
            break
    
    if glfw_path:
        env.Append(CPPPATH=[f"{glfw_path}/include"])
        env.Append(LIBPATH=[f"{glfw_path}/lib"])
    
    env.Append(LINKFLAGS=["-framework", "OpenGL", "-lglfw"])

elif platform.system() == "Windows":
    # Windows: Find GLFW (common installation locations)
    # Try vcpkg first (common on Windows)
    vcpkg_paths = [
        os.path.expanduser("~/vcpkg/installed/x64-windows"),
        "C:/vcpkg/installed/x64-windows",
        "C:/vcpkg/installed/x86-windows",
    ]
    
    # Also try common GLFW installation paths
    glfw_paths = [
        "C:/Program Files/GLFW",
        "C:/GLFW",
        os.path.expanduser("~/GLFW"),
    ]
    
    glfw_found = False
    for path in vcpkg_paths + glfw_paths:
        if os.path.exists(path):
            include_path = os.path.join(path, "include")
            lib_path = os.path.join(path, "lib")
            if os.path.exists(include_path) and os.path.exists(lib_path):
                env.Append(CPPPATH=[include_path])
                env.Append(LIBPATH=[lib_path])
                glfw_found = True
                break
    
    # Windows OpenGL libraries
    env.Append(LIBS=["opengl32", "glu32"])
    
    # GLFW library (try different names)
    if glfw_found:
        env.Append(LIBS=["glfw3"])
    else:
        # If GLFW not found in standard locations, try system-wide
        env.Append(LIBS=["glfw3", "glfw"])
        print("Warning: GLFW not found in standard locations. Make sure GLFW is in your system PATH or library search path.")

else:
    # Linux: Use pkg-config if available, otherwise try common paths
    import subprocess
    try:
        # Try to get GLFW info from pkg-config
        result = subprocess.run(["pkg-config", "--cflags", "--libs", "glfw3"], 
                              capture_output=True, text=True)
        if result.returncode == 0:
            # Parse pkg-config output (simplified - may need more robust parsing)
            flags = result.stdout.split()
            for flag in flags:
                if flag.startswith("-I"):
                    env.Append(CPPPATH=[flag[2:]])
                elif flag.startswith("-L"):
                    env.Append(LIBPATH=[flag[2:]])
            env.Append(LIBS=["glfw3", "GL"])
        else:
            # Fallback to common Linux paths
            env.Append(LIBS=["glfw3", "GL", "GLU"])
    except:
        # pkg-config not available, use defaults
        env.Append(LIBS=["glfw3", "GL", "GLU"])

# Original ray tracer
env.Program("ray_tracer",
            [
                "camera.cpp","hierarchy.cpp",
                "flat_shader.cpp","main.cpp","parse.cpp",
                "phong_shader.cpp","plane.cpp","reflective_shader.cpp",
                "render_world.cpp","sphere.cpp","box.cpp","mesh.cpp","cubed_sphere.cpp",
				"density_field.cpp","lerped_spheres.cpp","surface_nets.cpp","noise.cpp"
            ])

# OpenGL test executable
env.Program("opengl_test",
            [
                "main_opengl_test.cpp",
                "opengl_renderer.cpp",
                "mesh.cpp",
                "box.cpp",
                "cubed_sphere.cpp",
                "density_field.cpp",
                "lerped_spheres.cpp",
                "surface_nets.cpp",
                "noise.cpp"
            ])
