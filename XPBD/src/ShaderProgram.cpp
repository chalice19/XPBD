#include "ShaderProgram.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <exception>
#include <ios>

// Create a GPU program i.e., a graphics pipeline
ShaderProgram::ShaderProgram() : _id(glCreateProgram()) {}

ShaderProgram::~ShaderProgram()
{
  glDeleteProgram(_id);
}

std::string ShaderProgram::file2String(const std::string &filename)
{
  std::ifstream input(filename.c_str());
  if(!input)
    throw std::ios_base::failure("[Shader Program][file2String] Error: cannot open " + filename);
  std::stringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void ShaderProgram::loadShader(GLenum type, const std::string &shaderFilename)
{
  // Loads and compile a shader, before attaching it to a program
  GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
  std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
  if(shaderSourceString.empty()) {
    std::cerr << "No content in shader " << shaderFilename << std::endl;
    glDeleteShader(shader);
    return;
  }
  const GLchar *shaderSource = (const GLchar *) shaderSourceString.c_str(); // Interface the C++ string through a C pointer
  glShaderSource(shader, 1, &shaderSource, NULL); // Load the vertex shader source code
  glCompileShader(shader);      // THe GPU driver compile the shader
  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if(!compiled) {
    GLsizei len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    GLchar *log = new GLchar[len+1];
    glGetShaderInfoLog(shader, len, &len, log);
    std::cerr << "Compilation error in shader " << shaderFilename << " : " << std::endl << log << std::endl;
    delete [] log;
    glDeleteShader(shader);
    return;
  }
  glAttachShader(_id, shader); // Set the vertex shader as the one ot be used with the program/pipeline
  glDeleteShader(shader);
}


std::shared_ptr<ShaderProgram> ShaderProgram::genBasicShaderProgram(
  const std::string &vertexShaderFilename,
  const std::string &fragmentShaderFilename)
{
  std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
  shaderProgramPtr->loadShader(GL_VERTEX_SHADER, vertexShaderFilename);
  shaderProgramPtr->loadShader(GL_FRAGMENT_SHADER, fragmentShaderFilename);
  shaderProgramPtr->link();
  shaderProgramPtr->use();
  return shaderProgramPtr;
}
