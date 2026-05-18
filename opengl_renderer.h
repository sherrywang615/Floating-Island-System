#ifndef __OPENGL_RENDERER_H__
#define __OPENGL_RENDERER_H__

#include "vec.h"
#include "mesh.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#elif defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Forward declarations
class GLFWwindow;

class OpenGLRenderer
{
public:
    OpenGLRenderer();
    ~OpenGLRenderer();
    
    // Initialize OpenGL context and window
    bool Initialize(int width, int height, const char* title);
    
    // Cleanup
    void Shutdown();
    
    // Render a mesh
    void RenderMesh(const Mesh& mesh);
    
    // Set camera matrices
    void SetViewMatrix(const vec3& position, const vec3& target, const vec3& up);
    void SetProjectionMatrix(double fov, double aspect, double near, double far);
    
    // Clear screen
    void Clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
    
    // Swap buffers
    void SwapBuffers();
    
    // Check if window should close
    bool ShouldClose();
    
    // Get window dimensions
    void GetWindowSize(int& width, int& height);
    
    // Input handling
    void PollEvents();
    
    // Camera control
    vec3 camera_position;
    vec3 camera_target;
    vec3 camera_up;
    double camera_fov;
    
    // Get window pointer for callbacks
    GLFWwindow* GetWindow() { return window; }
    
    // Render volumetric clouds with ray marching
    void RenderVolumetricClouds(const vec3& box_min, const vec3& box_max,
                                float density_scale = 1.0f,
                                float noise_scale = 0.5f,
                                int octaves = 4,
                                float opacity_scale = 1.0f);
    
private:
    GLFWwindow* window;
    int window_width;
    int window_height;
    
    // Shader program ID
    GLuint shader_program;
    
    // Matrix storage (column-major for OpenGL)
    float view_matrix[16];
    float projection_matrix[16];
    
    // Initialize shaders
    bool LoadShaders();
    GLuint CompileShader(const char* source, GLenum type);
    GLuint LinkProgram(GLuint vertex_shader, GLuint fragment_shader);
    
    // Matrix helpers
    void SetIdentity(float* matrix);
    void Perspective(float* matrix, float fov, float aspect, float near, float far);
    void LookAt(float* matrix, const vec3& eye, const vec3& target, const vec3& up);
};

#endif

