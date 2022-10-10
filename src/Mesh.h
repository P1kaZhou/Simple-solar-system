//
// Created by daniel on 10/10/22.
//

#ifndef TPOPENGL_MESH_H
#define TPOPENGL_MESH_H

#include <vector>
#include <cmath>
#include <math.h>
#include <iostream>
#include <glad/glad.h>
#include "dep/glfw/deps/linmath.h"
#include "Camera.h"
#include <glm/ext.hpp>
#include "utils.h"

class Mesh {
public:
    void init(); // should properly set up the geometry buffer
    void render(Camera g_camera) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

        const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
        const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
        const glm::vec3 camPosition = g_camera.getPosition();
        glUniform3f(glGetUniformLocation(m_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);
        glUniformMatrix4fv(glGetUniformLocation(m_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(
                viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
        glUniformMatrix4fv(glGetUniformLocation(m_program, "projMat"), 1, GL_FALSE, glm::value_ptr(
                projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program
        glBindVertexArray(m_vao);     // activate the VAO storing geometry data
        glDrawElements(GL_TRIANGLES, m_triangleIndices.size(), GL_UNSIGNED_INT,
                       0); // Call for rendering: stream the current GPU geometry through the current GPU program
    };


    inline void initCPU(const size_t resolution,
                        float radius,
                        float x_center,
                        float y_center,
                        float z_center) {


        float x, y, z, xy;                              // vertex position
        float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
        float s, t;                                     // vertex texCoord

        float sectorStep = 2 * M_PI / resolution;
        float stackStep = M_PI / resolution;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= resolution; ++i) {
            stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= resolution; ++j) {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                this->addPositionCoordinate(x + x_center);
                this->addPositionCoordinate(y + y_center);
                this->addPositionCoordinate(z + z_center);

                // normalized vertex normal (nx, ny, nz)
                nx = (x - x_center) * lengthInv;
                ny = (y - y_center) * lengthInv;
                nz = (z - z_center) * lengthInv;
                this->addNormalCoordinate(nx);
                this->addNormalCoordinate(ny);
                this->addNormalCoordinate(nz);

                // vertex tex coord (s, t) range between [0, 1]
                // s = (float)j / resolution;
                // t = (float)i / resolution;
                // texCoords.push_back(s);
                // texCoords.push_back(t);
            }
        }

        // generate CCW index list of sphere triangles
        // k1--k1+1
        // |  / |
        // | /  |
        // k2--k2+1
        int k1, k2;
        for (int i = 0; i < resolution; ++i) {
            k1 = i * (resolution + 1);     // beginning of current stack
            k2 = k1 + resolution + 1;      // beginning of next stack

            for (int j = 0; j < resolution; ++j, ++k1, ++k2) {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0) {
                    this->addTriangleCoordinate(k1);
                    this->addTriangleCoordinate(k2);
                    this->addTriangleCoordinate(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (resolution - 1)) {
                    this->addTriangleCoordinate(k1 + 1);
                    this->addTriangleCoordinate(k2);
                    this->addTriangleCoordinate(k2 + 1);
                }

                // store indices for lines
                // vertical lines for all stacks, k1 => k2
                this->addLineIndice(k1);
                this->addLineIndice(k2);
                if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
                {
                    this->addLineIndice(k1);
                    this->addLineIndice(k1 + 1);
                }
            }
        }
        std::cout << "Voilà" << std::endl;

    } // should generate a unit sphere

    void initGPUGeometrySphere() {

        glGenVertexArrays(1, &m_vao);
        // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
        glBindVertexArray(m_vao);
        // Generate a GPU buffer to store the positions of the vertices
        size_t vertexBufferSize =
                sizeof(float) * m_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
        size_t vertexColorBufferSize = sizeof(float) * m_vertexNormals.size();

        glGenBuffers(1, &m_posVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, m_vertexPositions.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &m_normalVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
        glBufferData(GL_ARRAY_BUFFER, vertexColorBufferSize, m_vertexNormals.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(1);

        // Same for an index buffer object that stores the list of indices of the
        // triangles forming the meshSun
        size_t indexBufferSize = sizeof(unsigned int) * m_triangleIndices.size();


        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_READ);


        glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
    }

    void initGPUProgram(){
        m_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
        loadShader(m_program, GL_VERTEX_SHADER, "vertexShader.glsl");
        loadShader(m_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
        glLinkProgram(m_program); // The main GPU program is ready to be handle streams of polygons

        glUseProgram(m_program);
        // TODO: set shader variables, textures, etc.
    }

    void setColor(float r, float g, float b){
        gluniform3f(glGetUniformLocation(m_program), r,g,b);
    }


private:
    std::vector<float> m_vertexPositions;
    std::vector<float> m_vertexNormals;
    std::vector<float> m_textureCoords;
    std::vector<unsigned int> m_triangleIndices;
    std::vector<int> m_lineIndices;
    GLuint m_program;
    vec3 color;
    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_ibo = 0;

    void addPositionCoordinate(float x) {
        m_vertexPositions.push_back(x);
    }

    std::vector<float> getPositions() {
        return m_vertexPositions;
    }

    void addNormalCoordinate(float x) {
        m_vertexNormals.push_back(x);
    }

    std::vector<float> getNormals() {
        return m_vertexNormals;
    }

    void addTriangleCoordinate(unsigned int x) {
        m_triangleIndices.push_back(x);
    }

    std::vector<unsigned int> getTriangles() {
        return m_triangleIndices;
    }

    void addLineIndice(int x) {
        m_lineIndices.push_back(x);
    }

    std::vector<int> getLineIndices() {
        return m_lineIndices;
    }


};


#endif //TPOPENGL_MESH_H
