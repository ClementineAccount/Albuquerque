#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string_view>
#include <string>

namespace Utility
{
  std::string LoadFile(std::string_view path);
}
