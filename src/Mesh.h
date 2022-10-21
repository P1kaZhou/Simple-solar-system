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

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "utils.h"

class Mesh {
public:
    void init(); // should properly set up the geometry buffer

    void render(Camera g_camera) {

        glUseProgram(m_program);
        setColor(r, g, b); //Because the colors are soft "updated" at each loop

        const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
        const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
        const glm::vec3 camPosition = g_camera.getPosition();
        glUniform3f(glGetUniformLocation(m_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);
        glUniformMatrix4fv(glGetUniformLocation(m_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(
                viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
        glUniformMatrix4fv(glGetUniformLocation(m_program, "projMat"), 1, GL_FALSE, glm::value_ptr(
                projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program
        glm::vec4 vec(x, y, z, 1.0f);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(x, y, z));
        trans = glm::scale(trans, glm::vec3(radius_multiplier, radius_multiplier, radius_multiplier));
        trans = glm::rotate(trans, rotation_angle, rotation_axis);

        glUniform4f(glGetUniformLocation(m_program, "position"), x, y, z, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(m_program, "trans"), 1, GL_FALSE, glm::value_ptr(trans));
        glBindVertexArray(m_vao);     // activate the VAO storing geometry data
        glDrawElements(GL_TRIANGLES, m_triangleIndices.size(), GL_UNSIGNED_INT, 0);
        // Call for rendering: stream the current GPU geometry through the current GPU program

        // MOVE


        glActiveTexture(GL_TEXTURE0); // activate texture unit 0
        glBindTexture(GL_TEXTURE_2D, textureInt);
        // Line indices or textureCoords? Or triangleindices?

    };


    inline void initCPU(const size_t resolution,
                        float radius,
                        float x_center,
                        float y_center,
                        float z_center,
                        glm::vec3 rotation_axis) {

        this->x = x_center;
        this->y = y_center;
        this->z = z_center;

        this->radius_multiplier = radius;
        this->rotation_axis = rotation_axis;
        this->rotation_angle = 0;

        float x, y, z, xy;                              // vertex position
        float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
        float s, t;                                     // vertex texCoord

        float sectorStep = 2 * M_PI / (float) resolution;
        float stackStep = M_PI / (float) resolution;
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
                m_vertexPositions.push_back(x + x_center);
                m_vertexPositions.push_back(y + y_center);
                m_vertexPositions.push_back(z + z_center);

                // normalized vertex normal (nx, ny, nz)
                nx = (x - x_center) * lengthInv;
                ny = (y - y_center) * lengthInv;
                nz = (z - z_center) * lengthInv;
                m_vertexNormals.push_back(nx);
                m_vertexNormals.push_back(ny);
                m_vertexNormals.push_back(nz);


                // vertex tex coord (s, t) range between [0, 1]
                s = j / (float) resolution;
                t = i / (float) resolution;
                m_textureCoords.push_back(s);
                m_textureCoords.push_back(t);
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
                    m_triangleIndices.push_back(k1);
                    m_triangleIndices.push_back(k2);
                    m_triangleIndices.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (resolution - 1)) {
                    m_triangleIndices.push_back(k1 + 1);
                    m_triangleIndices.push_back(k2);
                    m_triangleIndices.push_back(k2 + 1);
                }

                // store indices for lines
                // vertical lines for all stacks, k1 => k2
                m_lineIndices.push_back(k1);
                m_lineIndices.push_back(k2);
                if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
                {
                    m_lineIndices.push_back(k1);
                    m_lineIndices.push_back(k1 + 1);
                }
            }
        }
    } // should generate a unit sphere

    void initGPUGeometrySphere() {
        glGenVertexArrays(1, &m_vao);

        // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
        glBindVertexArray(m_vao);
        // Generate a GPU buffer to store the positions of the vertices
        size_t vertexBufferSize = sizeof(float) * m_vertexPositions.size();
        // Gather the size of the buffer from the CPU-side vector
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

        size_t vertexTextureBufferSize = sizeof(float) * m_textureCoords.size();
        glGenBuffers(1, &m_tbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_tbo);
        glBufferData(GL_ARRAY_BUFFER, vertexTextureBufferSize, m_textureCoords.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0); // todo: what do I put here as well
        glEnableVertexAttribArray(2);

        // Same for an index buffer object that stores the list of indices of the triangles forming the mesh
        size_t indexBufferSize = sizeof(unsigned int) * m_triangleIndices.size();
        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_READ);


        glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
    }

    void loadTextureFromFileToGPU(std::string texture) {
        // Loading the image in CPU memory using stb_image
        int width, height, numComponents;
        unsigned char *data = stbi_load(texture.c_str(), &width, &height, &numComponents, 0);
        GLuint texID; // OpenGL texture identifier
        glGenTextures(1, &texID); // generate an OpenGL texture container
        glBindTexture(GL_TEXTURE_2D, texID); // activate the texture
// Setup the texture filtering option and repeat mode; check www.opengl.org for details.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// Fill the GPU texture with the data stored in the CPU image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
// Free useless CPU memory
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

        textureInt = texID; // = return texID
    }

    void initGPUProgram(std::string texture) {

        m_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
        loadShader(m_program, GL_VERTEX_SHADER, "vertexShader.glsl");
        loadShader(m_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
        glLinkProgram(m_program); // The main GPU program is ready to be handle streams of polygons

        loadTextureFromFileToGPU(texture);

        glUniform1i(glGetUniformLocation(m_program, "material.albedoTex"), 0);

    }

    void setColor(float r, float g, float b) {
        glUniform3f(glGetUniformLocation(m_program, "diffuseColor"), r, g, b);
        this->r = r;
        this->g = g;
        this->b = b;
    }

    void changeRotationAngle(float angle) {
        this->rotation_angle = angle;
    }

    void setX(float next) {
        this->x = next;
    }

    void setZ(float next) {
        this->z = next;
    }

private:
    std::vector<float> m_vertexPositions;
    std::vector<float> m_vertexNormals;
    std::vector<float> m_textureCoords;
    std::vector<unsigned int> m_triangleIndices;
    std::vector<int> m_lineIndices;
    GLuint m_program;

    float r; // Red
    float g; // Green
    float b; // Blue
    float x; // x coordinate
    float y; // y coordinate
    float z; // z coordinate
    float radius_multiplier;
    GLuint textureInt;

    glm::vec3 rotation_axis;
    float rotation_angle;

    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_ibo = 0;  // Index buffer object
    GLuint m_tbo = 0;

};

#endif //TPOPENGL_MESH_H