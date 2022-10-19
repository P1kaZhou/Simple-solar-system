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

    void render(Camera g_camera, std::string texture) {
        // Erase the color and z buffers.
        glUseProgram(m_program);
        setColor(r, g, b);
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
        // MOVE

        glm::vec4 vec(x, y, z, 1.0f);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(x, y, z) );
        trans = glm::scale(trans, glm::vec3(radius_multiplier, radius_multiplier, radius_multiplier));
        trans = glm::rotate(trans, rotation_angle , rotation_axis);

        glUniform4f(glGetUniformLocation(m_program, "position"), x, y, z, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(m_program, "trans"), 1, GL_FALSE, glm::value_ptr(
                trans));
        glActiveTexture(GL_TEXTURE0); // activate texture unit 0
        glBindTexture(GL_TEXTURE_2D, textureInt);
        glUniform1i(glGetUniformLocation(m_program, "ourTexture"), textureInt);

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

        radius_multiplier = radius;
        this->rotation_axis = rotation_axis;
        this->rotation_angle = 0;

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
                s = (float)j / resolution;
                t = (float)i / resolution;
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

        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ARRAY_BUFFER, m_ibo);


        // Same for an index buffer object that stores the list of indices of the
        // triangles forming the meshSun
        size_t indexBufferSize = sizeof(unsigned int) * m_triangleIndices.size();


        glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
    }

    void initGPUProgram(std::string texture) {

        size_t vertexTextureBufferSize = sizeof(float) * m_textureCoords.size();
        m_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
        loadShader(m_program, GL_VERTEX_SHADER, "vertexShader.glsl");
        loadShader(m_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
        glLinkProgram(m_program); // The main GPU program is ready to be handle streams of polygons

        int width, height, numComponents;
        // Loading the image in CPU memory using stb_image
        unsigned char *data = stbi_load(
                texture.c_str(),
                &width, &height,
                &numComponents, // 1 for a 8 bit grey-scale image, 3 for 24bits RGB image, 4 for 32bits RGBA image
                0);

        GLuint texID;

        glGenTextures(1, &texID); // generate an OpenGL texture container
        glBindTexture(GL_TEXTURE_2D, texID); // activate the texture
        // Setup the texture filtering option and repeat mode; check www.opengl.org for details.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glGenBuffers(1, &m_tbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_tbo);
        glBufferData(GL_ARRAY_BUFFER, vertexTextureBufferSize, m_textureCoords.data(), GL_DYNAMIC_READ);
        // Fill the GPU texture with the data stored in the CPU image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glUniform1i(glGetUniformLocation(m_program, "ourTexture"), texID);
        textureInt = texID;
        glDrawElements(GL_TRIANGLES, vertexTextureBufferSize, GL_UNSIGNED_INT, 0);
        // Free useless CPU memory
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture


    }

    void setColor(float r, float g, float b) {
        glUniform3f(glGetUniformLocation(m_program, "color"), r, g, b);
        this->r = r;
        this->g = g;
        this->b = b;
    }

    float getPositionX() {
        return x;
    }

    float getPositionY() {
        return y;
    }

    float getPositionZ() {
        return z;
    }

    void changeRotationAngle(float angle){
        this->rotation_angle = angle;
    }

    void move(float dx, float dy, float dz) {
        x += dx;
        y += dy;
        z += dz;
    }

    void setX(float next){
        this->x = next;
    }

    void setY(float next){
        this->y = next;
    }

    void setZ(float next){
        this->z = next;
    }



private:
    std::vector<float> m_vertexPositions;
    std::vector<float> m_vertexNormals;
    std::vector<float> m_textureCoords;
    std::vector<unsigned int> m_triangleIndices;
    std::vector<int> m_lineIndices;
    GLuint m_program;
    float r;
    float g;
    float b;
    float x;
    float y;
    float z;
    float radius_multiplier;
    GLuint textureInt;

    glm::vec3 rotation_axis;
    float rotation_angle;

    float next_x;
    float next_y;
    float next_z;

    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_ibo = 0;
    GLuint m_tbo =0;

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
