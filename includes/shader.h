#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <GLM/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
   public:
    unsigned int id;
    Shader(const char *vertexPath, const char *fragmentPath) {
        std::string vertexCode, fragmentCode;
        readFile(vertexPath, vertexCode);
        readFile(fragmentPath, fragmentCode);
        GLuint vertexShader, fragmentShader;
        const char *vertexChar = vertexCode.c_str();
        const char *fragmentChar = fragmentCode.c_str();
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexChar, nullptr);
        glCompileShader(vertexShader);
        check(vertexShader, compilationType::VERTEX);
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentChar, nullptr);
        glCompileShader(fragmentShader);
        check(fragmentShader, compilationType::FRAGMENT);

        id = glCreateProgram();
        glAttachShader(id, vertexShader);
        glAttachShader(id, fragmentShader);
        glLinkProgram(id);
        check(id, compilationType::PROGRAM);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    void use() const { glUseProgram(id); }
    void set(const std::string &name, int value) const {
        glProgramUniform1i(id, glGetUniformLocation(id, name.c_str()), value);
    }
    void set(const std::string &name, float value) const {
        glProgramUniform1f(id, glGetUniformLocation(id, name.c_str()), value);
    }
    void set(const std::string &name, glm::vec2 value) const {
        glProgramUniform2fv(id, glGetUniformLocation(id, name.c_str()), 1,
                            &value[0]);
    }
    void set(const std::string &name, glm::vec3 value) const {
        glProgramUniform3fv(id, glGetUniformLocation(id, name.c_str()), 1,
                            &value[0]);
    }
    void set(const std::string &name, glm::vec4 value) const {
        glProgramUniform4fv(id, glGetUniformLocation(id, name.c_str()), 1,
                            &value[0]);
    }
    void set(const std::string &name, glm::mat3 value) const {
        glProgramUniformMatrix3fv(id, glGetUniformLocation(id, name.c_str()), 1,
                                  GL_FALSE, &value[0][0]);
    }
    void set(const std::string &name, glm::mat4 value) const {
        glProgramUniformMatrix4fv(id, glGetUniformLocation(id, name.c_str()), 1,
                                  GL_FALSE, &value[0][0]);
    }

   private:
    enum compilationType { PROGRAM, VERTEX, FRAGMENT };
    void readFile(const char *path, std::string &out) {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            file.open(path);
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            out = buffer.str();
        } catch (std::ifstream::failure &e) {
            std::cerr << "ERROR READING SHADER FILE: " << e.what() << std::endl;
        }
    }
    void check(GLuint shader, compilationType type) {
        int result, length;
        std::string message;
        if (type == compilationType::PROGRAM)
            glGetProgramiv(shader, GL_LINK_STATUS, &result);
        else
            glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            message.resize(length);
            if (type == compilationType::PROGRAM)
                glGetProgramInfoLog(shader, length, nullptr, message.data());
            else
                glGetShaderInfoLog(shader, length, nullptr, message.data());
            std::cerr << message << std::endl;
            glDeleteShader(shader);
        }
    }
};

#endif