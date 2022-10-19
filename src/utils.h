//
// Created by daniel on 10/10/22.
//

#ifndef TPOPENGL_UTILS_H
#define TPOPENGL_UTILS_H

#include <string>
#include <iostream>
#include "glad/glad.h"
#include <fstream>
#include <sstream>

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string &filename) {
    std::ifstream t(filename.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename) {
    GLuint shader = glCreateShader(
            type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a meshSun
    std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
    const GLchar *shaderSource = (const GLchar *) shaderSourceString.c_str(); // Interface the C++ string through a C pointer
    glShaderSource(shader, 1, &shaderSource, NULL); // load the vertex shader code
    glCompileShader(shader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
    }
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

#endif //TPOPENGL_UTILS_H
