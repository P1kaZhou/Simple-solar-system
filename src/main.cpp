// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2022 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstdlib>
#include <iostream>

#include <vector>

#include <cmath>
#include <memory>

#include "Mesh.h"


#include "Camera.h"

// constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 10.;
const static float kRadOrbitMoon = 2.;

const static float earthRevolutionSpeed = 30.0 * M_PI / 360.0; // Orbital period of Earth
const static float moonRevolutionSpeed = 30.0 * 4.0 * M_PI / 360.0;
const static float earthRotationSpeed = 30.0 * 2.0 * M_PI / 360.0;
const static float moonRotationSpeed = 30.0 * 4.0 * M_PI / 360.0;


// Window parameters
GLFWwindow *g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// Textures
const std::string sunTexture = "media/earth.jpg";
const std::string earthTexture = "media/moon.jpg";
const std::string moonTexture = "media/sun.jpg";

/* Single triangle stuff
// All vertex positions packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;

//Colors
std::vector<float> g_vertexColors;
// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_colVbo = 0;
GLuint g_ibo = 0;
*/

//Mesh initialization
Mesh meshSun;
Mesh meshEarth;
Mesh meshMoon;

float _radius;
float _theta;
float _phi;
// Basic camera model

// Model transformation matrices
// glm::mat4 g_sun, g_earth, g_moon;


Camera g_camera;

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow *window, int width, int height) {
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint) width, (GLint) height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_W) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else if (action == GLFW_PRESS && key == GLFW_KEY_F) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else if (action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
        glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
    }
}

void errorCallback(int error, const char *desc) {
    std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW, the library responsible for window management
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to init GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Before creating the window, set some option flags
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create the window
    g_window = glfwCreateWindow(
            1024, 768,
            "Interactive 3D Applications (OpenGL) - Simple Solar System",
            nullptr, nullptr);
    if (!g_window) {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
    glfwMakeContextCurrent(g_window);
    glfwSetWindowSizeCallback(g_window, windowSizeCallback);
    glfwSetKeyCallback(g_window, keyCallback);
}

void initOpenGL() {
    // Load extensions for modern OpenGL
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
    glEnable(GL_DEPTH_TEST);      // Enable the z-buffer test in the rasterization
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

void initGPUprogram() {
    meshSun.initGPUProgram(sunTexture);
    meshSun.setColor(1.0, 1.0, 0.0);
    meshEarth.initGPUProgram(earthTexture);
    meshEarth.setColor(0.0, 1.0, 0.0);
    meshMoon.initGPUProgram(moonTexture);
    meshMoon.setColor(0.0, 0.0, 1.0);
}

// Define your meshSun(es) in the CPU memory
void initCPUgeometry() {
    /*
    g_vertexPositions = {
            0.f, 0.f, 0.f,
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f
    };

    g_vertexColors = {
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            0.f, 0.f, 1.f
    };

    g_triangleIndices = {0, 1, 2};
    */
    glm::vec3 sunRotation = glm::vec3(1., 0., 0.);
    meshSun.initCPU(32, kSizeSun, 0., 0., 0., sunRotation);
    glm::vec3 earthRotation = glm::vec3(cos(23.5 * M_PI / 180.), 0., sin(23.5 * M_PI / 180.));
    meshEarth.initCPU(32, kSizeEarth, 10., 0., 0., earthRotation);
    glm::vec3 moonRotation = glm::vec3(1., 0., 0.);
    meshMoon.initCPU(32, kSizeMoon, 12., 0., 0., moonRotation);
}


void initGPUgeometry() {

    meshSun.initGPUGeometrySphere();
    meshEarth.initGPUGeometrySphere();
    meshMoon.initGPUGeometrySphere();
    // Create a single handle, vertex array object that contains attributes,
    // vertex buffer objects (e.g., vertex's position, normal, and color)
    /*
    glGenVertexArrays(1,
                      &g_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.


    glBindVertexArray(g_vao);

    // Generate a GPU buffer to store the positions of the vertices
    size_t vertexBufferSize =
            sizeof(float) * g_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
    size_t vertexColorBufferSize = sizeof(float) * g_vertexColors.size();

    glGenBuffers(1, &g_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &g_colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_colVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexColorBufferSize, g_vertexColors.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);

    // Same for an index buffer object that stores the list of indices of the
    // triangles forming the meshSun
    size_t indexBufferSize = sizeof(unsigned int) * g_triangleIndices.size();


    glGenBuffers(1, &g_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_READ);


    glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
    */
}

void initCamera() {
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

    g_camera.setPosition(glm::vec3(0.0, 0.0, 3.0));
    g_camera.setNear(0.1);
    g_camera.setFar(80.1);
}

void init() {
    initGLFW();
    initOpenGL();
    initCPUgeometry();
    initGPUprogram();
    initGPUgeometry();
    initCamera();
}

void clear() {
    glDeleteProgram(g_program);

    glfwDestroyWindow(g_window);
    glfwTerminate();
}

// The main rendering call
void render() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
    const glm::vec3 camPosition = g_camera.getPosition();
    glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);
    glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(
            viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
    glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(
            projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program

    glBindVertexArray(g_vao);     // activate the VAO storing geometry data
    glDrawElements(GL_TRIANGLES, g_triangleIndices.size(), GL_UNSIGNED_INT,
                   0); // Call for rendering: stream the current GPU geometry through the current GPU program
    */

    float speed = 0.02f;
    if (glfwGetKey(g_window, GLFW_KEY_UP)) _theta = std::max(_theta - speed, 0.14f);
    if (glfwGetKey(g_window, GLFW_KEY_DOWN)) _theta = std::min(_theta + speed, 3.00f);
    if (glfwGetKey(g_window, GLFW_KEY_LEFT)) _phi -= speed;
    if (glfwGetKey(g_window, GLFW_KEY_RIGHT)) _phi += speed;

    g_camera.setPosition(glm::vec3(25.0 * std::sin(_theta) * std::sin(_phi),
                                   25.0 * std::sin(_theta) * std::cos(_phi),
                                   25.0 * std::cos(_theta)));

    meshSun.render(g_camera, sunTexture);
    meshEarth.render(g_camera, earthTexture);
    meshMoon.render(g_camera, moonTexture);
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {
    // std::cout << currentTimeInSec << std::endl;


    //Compute the movement

    float future_earth_x = kRadOrbitEarth * cos(earthRevolutionSpeed * currentTimeInSec);
    float future_earth_z = kRadOrbitEarth * sin(earthRevolutionSpeed * currentTimeInSec);

    meshEarth.setX(future_earth_x);
    meshEarth.setZ(future_earth_z);

    float future_moon_x = kRadOrbitMoon * cos(moonRevolutionSpeed * currentTimeInSec) + future_earth_x;
    float future_moon_z = kRadOrbitMoon * sin(moonRevolutionSpeed * currentTimeInSec) + future_earth_z;
    // Always relative to the Earth

    meshMoon.setX(future_moon_x);
    meshMoon.setZ(future_moon_z);

    meshEarth.changeRotationAngle(earthRotationSpeed * currentTimeInSec);
    meshMoon.changeRotationAngle(moonRotationSpeed * currentTimeInSec);
}

int main(int argc, char **argv) {
    init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
    while (!glfwWindowShouldClose(g_window)) {
        update(static_cast<float>(glfwGetTime()));
        render();
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }
    clear();
    return EXIT_SUCCESS;
}
