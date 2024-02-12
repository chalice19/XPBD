#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <glad/glad.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class ShaderProgram {
public:
  // Create the program. A valid OpenGL context must be active.
  ShaderProgram();
  virtual ~ShaderProgram();

  // Generate a minimal shader program, made of one vertex shader and one fragment shader
  static std::shared_ptr<ShaderProgram> genBasicShaderProgram(
    const std::string &vertexShaderFilename, const std::string &fragmentShaderFilename);

  // OpenGL identifier of the program
  GLuint id() const { return _id; }

  // Loads and compile a shader from a text file, before attaching it to a program
  void loadShader(GLenum type, const std::string &shaderFilename);

  // The main GPU program is ready to be handle streams of polygons
  void link() { glLinkProgram(_id); }

  // Activate the program
  void use() { glUseProgram(_id); }

  // Desactivate the current program
  static void stop() { glUseProgram(0); }

  GLuint getLocation(const std::string &name) const
  {
    return glGetUniformLocation(_id, name.c_str());
  }

  void set(const std::string &name, int value)
  {
    glUniform1i(getLocation(name.c_str()), value);
  }

  void set(const std::string &name, float value)
  {
    glUniform1f(getLocation(name.c_str()), value);
  }

  void set(const std::string &name, const glm::vec2 &value)
  {
    glUniform2fv(getLocation(name.c_str()), 1, glm::value_ptr(value));
  }

  void set(const std::string &name, const glm::vec3 &value)
  {
    glUniform3fv(getLocation(name.c_str()), 1, glm::value_ptr(value));
  }

  void set(const std::string &name, const glm::vec4 &value)
  {
    glUniform4fv(getLocation(name.c_str()), 1, glm::value_ptr(value));
  }

  void set(const std::string &name, const glm::mat4 &value)
  {
    glUniformMatrix4fv(getLocation(name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
  }

  void set(const std::string &name, const glm::mat3 &value)
  {
    glUniformMatrix3fv(getLocation(name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
  }

private:
  // Loads the content of an ASCII file in a standard C++ string
  std::string file2String(const std::string &filename);

  GLuint _id = 0;
};

#endif  // SHADER_PROGRAM_H
