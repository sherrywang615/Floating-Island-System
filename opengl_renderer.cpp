#include "opengl_renderer.h"
#include "mesh.h"
#include "noise.h"
#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <limits>

// GLFW includes
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

OpenGLRenderer::OpenGLRenderer()
    : window(nullptr), window_width(800), window_height(600),
      camera_position(0, 5, 15), camera_target(0, 0, 0), camera_up(0, 1, 0),
      camera_fov(45.0), shader_program(0)
{
    SetIdentity(view_matrix);
    SetIdentity(projection_matrix);
}

OpenGLRenderer::~OpenGLRenderer()
{
    Shutdown();
}

bool OpenGLRenderer::Initialize(int width, int height, const char* title)
{
    window_width = width;
    window_height = height;
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Configure GLFW
    // On macOS, we need to be careful with OpenGL versions
    // Try OpenGL 2.1 first (widely supported, has immediate mode)
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
#elif defined(_WIN32)
    // On Windows, use OpenGL 3.3 compatibility profile for best compatibility
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
#else
    // On Linux, try compatibility profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
    
    // Create window
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        const char* description;
        int code = glfwGetError(&description);
        std::cerr << "Failed to create GLFW window: " << description << " (code: " << code << ")" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    
    // Set viewport
    glViewport(0, 0, width, height);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Disable back face culling to show all faces
    // This is necessary because voxel mesh generation may have inconsistent winding
    glDisable(GL_CULL_FACE);
    
    // Initialize matrices
    SetProjectionMatrix(camera_fov, (double)width / height, 0.1, 100.0);
    SetViewMatrix(camera_position, camera_target, camera_up);
    
    // Try to load shaders (optional - we're using fixed function pipeline)
    // If shaders fail, we'll use fixed function rendering
    if (!LoadShaders()) {
        std::cout << "Note: Shaders not loaded, using fixed function pipeline" << std::endl;
        shader_program = 0; // Mark as not available
    }
    
    std::cout << "OpenGL Renderer initialized successfully" << std::endl;
    return true;
}

void OpenGLRenderer::Shutdown()
{
    if (shader_program) {
        glDeleteProgram(shader_program);
        shader_program = 0;
    }
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void OpenGLRenderer::RenderMesh(const Mesh& mesh)
{
    if (mesh.vertices.empty() || mesh.triangles.empty()) return;
    
    // Use shader program
    glUseProgram(shader_program);
    
    // For immediate mode with OpenGL 2.1, use fixed function pipeline
    // Disable shader program and use fixed function
    glUseProgram(0);
    
    // Set up fixed function pipeline matrices
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection_matrix);
    
    glMatrixMode(GL_MODELVIEW);
    float modelview[16];
    // modelview = view * model
    // For now, just use view matrix (model is identity)
    memcpy(modelview, view_matrix, 16 * sizeof(float));
    glLoadMatrixf(modelview);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Set light position
    float light_pos[4] = {5.0f, 10.0f, 5.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    
    // Set material color (brown/tan)
    float mat_ambient[4] = {0.8f, 0.6f, 0.4f, 1.0f};
    float mat_diffuse[4] = {0.8f, 0.6f, 0.4f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);
    
    // Render using immediate mode
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < mesh.triangles.size(); i++) {
        ivec3 tri = mesh.triangles[i];
        if (tri[0] >= (int)mesh.vertices.size() || 
            tri[1] >= (int)mesh.vertices.size() || 
            tri[2] >= (int)mesh.vertices.size()) {
            continue; // Skip invalid triangles
        }
        
        vec3 v0 = mesh.vertices[tri[0]];
        vec3 v1 = mesh.vertices[tri[1]];
        vec3 v2 = mesh.vertices[tri[2]];
        
        // Calculate normal
        vec3 normal = cross(v1 - v0, v2 - v0);
        double len = normal.magnitude();
        if (len > 1e-6) {
            normal = normal / len;
        }
        
        // Send normal and vertices using fixed function
        glNormal3f(normal[0], normal[1], normal[2]);
        glVertex3f(v0[0], v0[1], v0[2]);
        glVertex3f(v1[0], v1[1], v1[2]);
        glVertex3f(v2[0], v2[1], v2[2]);
    }
    glEnd();
    
    glDisable(GL_LIGHTING);
}

void OpenGLRenderer::RenderVolumetricClouds(const vec3& box_min, const vec3& box_max,
                                            float density_scale,
                                            float noise_scale,
                                            int octaves,
                                            float opacity_scale)
{
    // CPU-side ray marching through volume with Perlin noise sampling
    // Render individual pixels by casting rays and accumulating density
    
    // Set up for rendering
    glUseProgram(0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, window_width, 0, window_height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Ray marching parameters
    const int max_steps = 64;
    const float step_size = 0.2f;
    
    // Sample every pixel (stride of 1 for smooth rendering)
    const int pixel_stride = 4;
    
    // Use quads for better pixel coverage
    glBegin(GL_QUADS);
    
    for (int py = 0; py < window_height; py += pixel_stride) {
        for (int px = 0; px < window_width; px += pixel_stride) {
            // Generate ray from camera through pixel
            float ndc_x = (2.0f * px) / window_width - 1.0f;
            float ndc_y = (2.0f * py) / window_height - 1.0f;
            
            // Calculate ray direction
            float aspect = (float)window_width / window_height;
            float fov_rad = camera_fov * M_PI / 180.0f;
            float tan_fov = tan(fov_rad * 0.5f);
            
            vec3 forward = (camera_target - camera_position).normalized();
            vec3 right = cross(forward, camera_up).normalized();
            vec3 up = cross(right, forward).normalized();
            
            vec3 ray_dir = forward + right * (ndc_x * aspect * tan_fov) + up * (ndc_y * tan_fov);
            ray_dir = ray_dir.normalized();
            
            // Ray-box intersection
            double t_near = -std::numeric_limits<double>::max();
            double t_far = std::numeric_limits<double>::max();
            bool hit_box = true;
            
            for (int i = 0; i < 3; i++) {
                if (std::abs(ray_dir[i]) < 1e-8) {
                    if (camera_position[i] < box_min[i] || camera_position[i] > box_max[i]) {
                        hit_box = false;
                        break;
                    }
                } else {
                    double t1 = (box_min[i] - camera_position[i]) / ray_dir[i];
                    double t2 = (box_max[i] - camera_position[i]) / ray_dir[i];
                    if (t1 > t2) std::swap(t1, t2);
                    t_near = std::max(t_near, t1);
                    t_far = std::min(t_far, t2);
                    if (t_near > t_far) {
                        hit_box = false;
                        break;
                    }
                }
            }
            
            if (!hit_box || t_far < 0) continue;
            if (t_near < 0) t_near = 0;
            
            // Ray march through volume
            float accumulated_alpha = 0.0f;
            double t = t_near;
            
            for (int step = 0; step < max_steps && t < t_far; step++) {
                vec3 sample_pos = camera_position + ray_dir * t;
                
                // Sample 3D Perlin noise
                vec3 noise_coord = sample_pos * noise_scale;
                float noise_value = noise::perlin3D(
                    noise_coord[0], noise_coord[1], noise_coord[2]
                );
                
                // Remap from [-1, 1] to [0, 1]
                noise_value = (noise_value + 1.0f) * 0.5f;
                
                
                float density = noise_value * density_scale;
                
                if (density > 0.01f) {
                    // Accumulate using Beer's law
                    float step_alpha = 1.0f - exp(-density * 0.3f * step_size);
                    accumulated_alpha += (1.0f - accumulated_alpha) * step_alpha * opacity_scale;
                    
                    // Early termination
                    if (accumulated_alpha > 0.99f) break;
                }
                
                t += step_size;
            }
            
            // Render pixel if cloud was hit
            if (accumulated_alpha > 0.01f) {
                // Cloud color with accumulated transparency
                glColor4f(0.95f, 0.95f, 1.0f, std::min(accumulated_alpha, 0.95f));
                
                // Draw quad covering the pixel area
                float x0 = px;
                float y0 = py;
                float x1 = px + pixel_stride;
                float y1 = py + pixel_stride;
                
                glVertex2f(x0, y0);
                glVertex2f(x1, y0);
                glVertex2f(x1, y1);
                glVertex2f(x0, y1);
            }
        }
    }
    
    glEnd();
    
    // Restore state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void OpenGLRenderer::SetViewMatrix(const vec3& position, const vec3& target, const vec3& up)
{
    LookAt(view_matrix, position, target, up);
}

void OpenGLRenderer::SetProjectionMatrix(double fov, double aspect, double near, double far)
{
    Perspective(projection_matrix, (float)fov, (float)aspect, (float)near, (float)far);
}

void OpenGLRenderer::Clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::SwapBuffers()
{
    glfwSwapBuffers(window);
}

bool OpenGLRenderer::ShouldClose()
{
    return window ? glfwWindowShouldClose(window) : true;
}

void OpenGLRenderer::GetWindowSize(int& width, int& height)
{
    width = window_width;
    height = window_height;
    if (window) {
        glfwGetWindowSize(window, &width, &height);
        window_width = width;
        window_height = height;
    }
}

void OpenGLRenderer::PollEvents()
{
    glfwPollEvents();
}

bool OpenGLRenderer::LoadShaders()
{
    // Vertex shader source - GLSL 120 for OpenGL 2.1 compatibility
    // Note: GLSL 120 doesn't have inverse(), so we use a simpler version
    const char* vertex_shader_source = R"(
        #version 120
        attribute vec3 aPos;
        attribute vec3 aNormal;
        
        uniform mat4 modelMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 projectionMatrix;
        
        varying vec3 FragPos;
        varying vec3 Normal;
        
        void main() {
            FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
            // For GLSL 120, use normal directly (assuming modelMatrix doesn't scale non-uniformly)
            Normal = normalize((modelMatrix * vec4(aNormal, 0.0)).xyz);
            gl_Position = projectionMatrix * viewMatrix * vec4(FragPos, 1.0);
        }
    )";
    
    // Fragment shader source - GLSL 120 for OpenGL 2.1 compatibility
    const char* fragment_shader_source = R"(
        #version 120
        varying vec3 FragPos;
        varying vec3 Normal;
        
        uniform vec3 lightPos;
        uniform vec3 lightColor;
        uniform vec3 objectColor;
        
        void main() {
            // Ambient
            float ambientStrength = 0.3;
            vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
            
            // Diffuse
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
            
            // Combine
            vec3 result = (ambient + diffuse) * objectColor;
            gl_FragColor = vec4(result, 1.0);
        }
    )";
    
    // Compile shaders
    GLuint vertex_shader = CompileShader(vertex_shader_source, GL_VERTEX_SHADER);
    if (vertex_shader == 0) return false;
    
    GLuint fragment_shader = CompileShader(fragment_shader_source, GL_FRAGMENT_SHADER);
    if (fragment_shader == 0) {
        glDeleteShader(vertex_shader);
        return false;
    }
    
    // Link program
    shader_program = LinkProgram(vertex_shader, fragment_shader);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return shader_program != 0;
}

GLuint OpenGLRenderer::CompileShader(const char* source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        std::cerr << "Shader compilation failed: " << info_log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint OpenGLRenderer::LinkProgram(GLuint vertex_shader, GLuint fragment_shader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        std::cerr << "Shader linking failed: " << info_log << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

void OpenGLRenderer::SetIdentity(float* matrix)
{
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

void OpenGLRenderer::Perspective(float* matrix, float fov, float aspect, float near, float far)
{
    float f = 1.0f / tan(fov * 0.5f * M_PI / 180.0f);
    
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = f / aspect;
    matrix[5] = f;
    matrix[10] = (far + near) / (near - far);
    matrix[11] = -1.0f;
    matrix[14] = (2.0f * far * near) / (near - far);
}

void OpenGLRenderer::LookAt(float* matrix, const vec3& eye, const vec3& target, const vec3& up)
{
    vec3 f = (target - eye).normalized();
    vec3 s = cross(f, up).normalized();
    vec3 u = cross(s, f);
    
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = s[0];
    matrix[1] = u[0];
    matrix[2] = -f[0];
    matrix[4] = s[1];
    matrix[5] = u[1];
    matrix[6] = -f[1];
    matrix[8] = s[2];
    matrix[9] = u[2];
    matrix[10] = -f[2];
    matrix[12] = -dot(s, eye);
    matrix[13] = -dot(u, eye);
    matrix[14] = dot(f, eye);
    matrix[15] = 1.0f;
}

