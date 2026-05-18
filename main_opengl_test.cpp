#include "opengl_renderer.h"
#include "cubed_sphere.h"
#include "mesh.h"
#include "vec.h"
#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>
#include "lerped_spheres.h"
#include <random>
#include <vector>

// Global variables for input handling
static bool keys[1024] = {false};
static double last_mouse_x = 0.0, last_mouse_y = 0.0;
static bool first_mouse = true;
static double yaw = -90.0;
static double pitch = 0.0;

// Cloud parameters
static float cloud_density = 1.0f;
static float cloud_noise_scale = 0.5f;
static int cloud_octaves = 4;
static float cloud_opacity = 0.3f;

// GLFW callback functions
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
    if (action == GLFW_PRESS)
        keys[key] = true;
    else if (action == GLFW_RELEASE)
        keys[key] = false;
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (first_mouse) {
        last_mouse_x = xpos;
        last_mouse_y = ypos;
        first_mouse = false;
    }
    
    double xoffset = xpos - last_mouse_x;
    double yoffset = last_mouse_y - ypos; // Reversed since y-coordinates go from bottom to top
    
    last_mouse_x = xpos;
    last_mouse_y = ypos;
    
    double sensitivity = 0.1;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    yaw += xoffset;
    pitch += yoffset;
    
    // Constrain pitch
    if (pitch > 89.0) pitch = 89.0;
    if (pitch < -89.0) pitch = -89.0;
}

void ProcessInput(OpenGLRenderer& renderer, double delta_time)
{
    double camera_speed = 5.0 * delta_time;
    vec3& pos = renderer.camera_position;
    vec3& up = renderer.camera_up;
    
    // Calculate camera direction from yaw and pitch
    vec3 front;
    front[0] = cos(yaw * M_PI / 180.0) * cos(pitch * M_PI / 180.0);
    front[1] = sin(pitch * M_PI / 180.0);
    front[2] = sin(yaw * M_PI / 180.0) * cos(pitch * M_PI / 180.0);
    front = front.normalized();
    
    vec3 right = cross(front, up).normalized();
    vec3 camera_up = cross(right, front).normalized();
    
    // Move camera
    if (keys[GLFW_KEY_W])
        pos = pos + front * camera_speed;
    if (keys[GLFW_KEY_S])
        pos = pos - front * camera_speed;
    if (keys[GLFW_KEY_A])
        pos = pos - right * camera_speed;
    if (keys[GLFW_KEY_D])
        pos = pos + right * camera_speed;
    if (keys[GLFW_KEY_SPACE])
        pos = pos + camera_up * camera_speed;
    if (keys[GLFW_KEY_LEFT_SHIFT])
        pos = pos - camera_up * camera_speed;
    
    // Adjust cloud density
    float density_speed = 0.5f * delta_time;
    if (keys[GLFW_KEY_UP]) {
        cloud_density += density_speed;
        if (cloud_density > 5.0f) cloud_density = 5.0f;
    }
    if (keys[GLFW_KEY_DOWN]) {
        cloud_density -= density_speed;
        if (cloud_density < 0.0f) cloud_density = 0.0f;
    }
    
    // Adjust noise scale
    float noise_speed = 0.1f * delta_time;
    if (keys[GLFW_KEY_RIGHT]) {
        cloud_noise_scale += noise_speed;
        if (cloud_noise_scale > 2.0f) cloud_noise_scale = 2.0f;
    }
    if (keys[GLFW_KEY_LEFT]) {
        cloud_noise_scale -= noise_speed;
        if (cloud_noise_scale < 0.1f) cloud_noise_scale = 0.1f;
    }
    
    // Adjust opacity
    float opacity_speed = 0.5f * delta_time;
    if (keys[GLFW_KEY_EQUAL] || keys[GLFW_KEY_KP_ADD]) { // + key
        cloud_opacity += opacity_speed;
        if (cloud_opacity > 2.0f) cloud_opacity = 2.0f;
    }
    if (keys[GLFW_KEY_MINUS] || keys[GLFW_KEY_KP_SUBTRACT]) { // - key
        cloud_opacity -= opacity_speed;
        if (cloud_opacity < 0.0f) cloud_opacity = 0.0f;
    }
    
    // Update camera target
    renderer.camera_target = pos + front;
}



int main()
{
    std::cout << "Floating Island Renderer" << std::endl;
    std::cout << "========================" << std::endl;
    
    // Initialize renderer
    OpenGLRenderer renderer;
    
    if (!renderer.Initialize(1280, 720, "Floating Island Environment")) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }
    
    // Set up GLFW callbacks
    GLFWwindow* window = renderer.GetWindow();
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // ============================================
    // STEP 1: Create Multiple Islands
    // ============================================
    std::cout << "Creating multiple islands..." << std::endl;
    
    // Set up random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist(-50.0f, 50.0f);
    std::uniform_real_distribution<float> offset_dist(-7.0f, 7.0f);
    std::uniform_real_distribution<float> radius_dist(2.0f, 5.0f);
    std::uniform_int_distribution<int> num_spheres_dist(3, 6);
    
    // Create 10-15 islands
    int num_islands = 10 + (rand() % 6);
    std::vector<LerpedSpheres*> islands;
    std::vector<Mesh> island_meshes;
    
    float voxel_size = 0.5f;
    
    // First, create the main island in front of the camera with specific spheres
    {
        std::vector<SphereData> spheres;
        spheres.push_back({vec3(0,0,0),5.0f});
        spheres.push_back({vec3(5,0,0),3.0f});
        spheres.push_back({vec3(-4,5,2),2.5f});
        
        LerpedSpheres* main_island = new LerpedSpheres(spheres, voxel_size, 0.5f, false);
        islands.push_back(main_island);
        island_meshes.push_back(main_island->GetMesh());
    }
    
    // Then create the remaining random islands
    for (int i = 0; i < num_islands; i++) {
        // Random center position for this island
        vec3 island_center(pos_dist(gen), pos_dist(gen), pos_dist(gen));
        
        // Create 3-6 spheres for this island
        int num_spheres = num_spheres_dist(gen);
        std::vector<SphereData> spheres;
        
        for (int j = 0; j < num_spheres; j++) {
            vec3 sphere_pos = island_center;
            // Add random offset within 7 units
            sphere_pos[0] += offset_dist(gen);
            sphere_pos[1] += offset_dist(gen);
            sphere_pos[2] += offset_dist(gen);
            
            float sphere_radius = radius_dist(gen);
            spheres.push_back({sphere_pos, sphere_radius});
        }
        
        // Create the island mesh
        LerpedSpheres* island = new LerpedSpheres(spheres, voxel_size, 0.5f, false);
        islands.push_back(island);
        island_meshes.push_back(island->GetMesh());
    }
    
    std::cout << "Generated " << (num_islands + 1) << " islands (1 main + " << num_islands << " random)" << std::endl;
    for (size_t i = 0; i < island_meshes.size(); i++) {
        std::cout << "  Island " << i << (i == 0 ? " (main)" : "") << ": "
                  << island_meshes[i].vertices.size() << " vertices, "
                  << island_meshes[i].triangles.size() << " triangles" << std::endl;
    }
    
    // ============================================
    // STEP 2: Set up camera
    // ============================================
    renderer.camera_position = vec3(0, 20, 50);
    renderer.camera_target = vec3(0, 0, 0);
    renderer.camera_up = vec3(0, 1, 0);
    
    // Calculate initial yaw and pitch from camera position
    vec3 direction = (renderer.camera_target - renderer.camera_position).normalized();
    yaw = atan2(direction[2], direction[0]) * 180.0 / M_PI;
    pitch = asin(direction[1]) * 180.0 / M_PI;
    
    std::cout << "Starting interactive render loop..." << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  W/A/S/D - Move camera forward/left/backward/right" << std::endl;
    std::cout << "  Space/Shift - Move camera up/down" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  Up/Down Arrows - Increase/decrease cloud density" << std::endl;
    std::cout << "  Left/Right Arrows - Decrease/increase cloud detail size" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << std::endl;
    
    // Timing
    double last_time = glfwGetTime();
    double last_param_print = glfwGetTime();
    float last_printed_density = cloud_density;
    float last_printed_noise_scale = cloud_noise_scale;
    float last_printed_opacity = cloud_opacity;
    
    // ============================================
    // STEP 3: Render Loop
    // ============================================
    while (!renderer.ShouldClose()) {
        // Calculate delta time
        double current_time = glfwGetTime();
        double delta_time = current_time - last_time;
        last_time = current_time;
        
        // Process input
        ProcessInput(renderer, delta_time);
        
        // Print parameter changes every 0.5 seconds
        if (current_time - last_param_print > 0.5 &&
            (fabs(cloud_density - last_printed_density) > 0.01f ||
             fabs(cloud_noise_scale - last_printed_noise_scale) > 0.01f ||
             fabs(cloud_opacity - last_printed_opacity) > 0.01f))
        {
            std::cout << "\rDensity: " << cloud_density 
                     << " | Noise Scale: " << cloud_noise_scale 
                     << " | Opacity: " << cloud_opacity << "    " << std::flush;
            last_param_print = current_time;
            last_printed_density = cloud_density;
            last_printed_noise_scale = cloud_noise_scale;
            last_printed_opacity = cloud_opacity;
        }
        
        // Update camera matrices
        int w, h;
        renderer.GetWindowSize(w, h);
        renderer.SetProjectionMatrix(45.0, (double)w / h, 0.1, 100.0);
        renderer.SetViewMatrix(renderer.camera_position, renderer.camera_target, renderer.camera_up);
        
        // Clear screen
        renderer.Clear(0.0f, 0.0f, 0.0f, 0.0f); // background color
        
        // ============================================
        // RENDER THE ISLAND MESHES
        // ============================================
        for (const auto& mesh : island_meshes) {
            renderer.RenderMesh(mesh);
        }
        
        // ============================================
        // RENDER VOLUMETRIC CLOUDS
        // ============================================
        vec3 cloud_box_min(-100, -100, -100);
        vec3 cloud_box_max(100, 100, 100);
        renderer.RenderVolumetricClouds(cloud_box_min, cloud_box_max, 
                                       cloud_density,
                                       cloud_noise_scale,
                                       cloud_octaves,
                                       cloud_opacity);
        
        // Swap buffers and poll events
        renderer.SwapBuffers();
        renderer.PollEvents();
    }
    
    std::cout << "Shutting down..." << std::endl;
    
    // Clean up islands
    for (auto* island : islands) {
        delete island;
    }
    
    renderer.Shutdown();
    
    return 0;
}