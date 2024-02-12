#include "Error.h"

#include <iostream>
#include <cstdlib>
#include <string>

void APIENTRY debugMessageCallback(
  GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar *message,
  const void *userParam)
{
  std::string sourceString ("Unknown");
  if(source == GL_DEBUG_SOURCE_API)
    sourceString = "API";
  else if(source  == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
    sourceString = "Window system API";
  else if(source  == GL_DEBUG_SOURCE_SHADER_COMPILER)
    sourceString = "Shading laguage compiler";
  else if(source  == GL_DEBUG_SOURCE_THIRD_PARTY)
    sourceString = "Application associated with OpenGL";
  else if(source  == GL_DEBUG_SOURCE_APPLICATION)
    sourceString = "User generated";
  else if(source  == GL_DEBUG_SOURCE_OTHER)
    sourceString = "Other";

  std::string severityString ("Unknown");
  if(severity == GL_DEBUG_SEVERITY_HIGH)
    severityString = "High";
  else if(severity == GL_DEBUG_SEVERITY_MEDIUM)
    severityString = "Medium";
  else if(severity == GL_DEBUG_SEVERITY_LOW)
    severityString = "Low";
  else if(severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    severityString = "Notification";

  std::string typeString ("Unknown");
  if(type == GL_DEBUG_TYPE_ERROR)
    typeString = "Error";
  else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
    typeString = "Deprecated behavior";
  else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
    typeString = "Undefined behavior";
  else if(type == GL_DEBUG_TYPE_PORTABILITY)
    typeString = "Portability issue";
  else if(type == GL_DEBUG_TYPE_PERFORMANCE)
    typeString = "Performance issue";
  else if(type == GL_DEBUG_TYPE_MARKER)
    typeString = "Command stream annotation";
  else if(type == GL_DEBUG_TYPE_PUSH_GROUP)
    typeString = "Group pushing";
  else if(type == GL_DEBUG_TYPE_POP_GROUP)
    typeString = "Group popping";
  else if(type == GL_DEBUG_TYPE_OTHER)
    typeString = "Other";

  std::cerr <<
    "----------------" << std::endl <<
    "[OpenGL Callback Message]: " << ( type == GL_DEBUG_TYPE_ERROR ? "** CRITICAL **" : "** NON CRITICAL **" ) << std::endl <<
    "    source = " << sourceString << std::endl <<
    "    type = " << typeString << std::endl <<
    "    severity = " << severityString << std::endl <<
    "    message = " << message << std::endl << std::endl <<
    "----------------" << std::endl;

  if(type == GL_DEBUG_TYPE_ERROR)
    std::exit(EXIT_FAILURE);
}
