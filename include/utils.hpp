#ifndef INCLUDE_UTILS_H
#define INCLUDE_UTILS_H

#include <GLFW/glfw3.h>

#include <filesystem>
#include <fstream>
#include <print>

struct Position {
  int width;
  int height;
};

bool get_window_center_pos(int window_width, int window_height, Position& pos);
bool create_program_from_filepath(const char* vert_filepath, const char* frag_filepath, GLuint& program);
bool create_program_from_cstr(const char* vert_cstr, const char* frag_cstr, GLuint& program);

#ifdef UTILS_IMPLEMENTATION

bool get_window_center_pos(int window_width, int window_height, Position& pos)
{
  GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
  if (!primary_monitor)
  {
    std::println(stderr, "[Error]: Failed to get primary monitor");
    return false;
  }

  const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
  if (!mode)
  {
    std::println(stderr, "[Error]: Failed to get video modes");
    return false;

  }

  pos.width = (mode->width - window_width)/2;
  pos.height = (mode->height - window_height)/2;

  return true;
}

bool read_file(std::string& content, std::filesystem::path target_path)
{
  // Check if the file exists and is a regular file
  if (!std::filesystem::exists(target_path) || !std::filesystem::is_regular_file(target_path))
  {
    std::println(stderr,
        "[Error]: File does not exist or is not a regular file: {}", target_path.string());
    return false;
  }

  // Get the file size
  std::uintmax_t size = std::filesystem::file_size(target_path);

  // Open the file in binary mode
  // (Specify binary to prevent size mismatch caused by newline conversion on Windows)
  std::ifstream file(target_path, std::ios::in | std::ios::binary);
  if (!file)
  {
    std::println(stderr, "[Error]: Could not open file");
    return false;
  }

  // Prepare a string with memory pre-allocated for the file size
  content.resize(size);

  // Read all data into the buffer at once
  if (!file.read(content.data(), size))
  {
    return false;
  }

  return true;
}

bool create_program(GLuint vert_shader, GLuint frag_shader, GLuint& program)
{
  int result{};

  program = glCreateProgram();
  glAttachShader(program, vert_shader);
  glAttachShader(program, frag_shader);

  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &result);
  if (result == GL_FALSE)
  {
    GLint info_len{};

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 1)
    {
      std::string info_log;
      info_log.resize(info_len);
      glGetProgramInfoLog(program, info_len, nullptr, info_log.data());
      std::print(stderr, "{}", info_log);
    }
    std::println(stderr, "[Error]: could not link shader program GL index {}", program );
    return false;
  }
  std::println("[Log]: created program {}. attaching shaders {} and {}", program, vert_shader, frag_shader );
  return true;
}

bool create_shader_from_filepath(const char* filepath, GLuint& shader, int kind)
{
  std::filesystem::path path{filepath};
  std::string content{};

  if (!read_file(content, path))
  {
    return false;
  }

  std::println("[Log]: creating shader from \"{}\"", filepath);
  shader = glCreateShader(kind);
  const char* content_ptr = content.c_str();
  glShaderSource(shader, 1, &content_ptr, nullptr);
  glCompileShader(shader);

  int result{};

  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE)
  {
    GLint info_len{};

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 1)
    {
      std::string info_log;
      info_log.resize(info_len);
      glGetShaderInfoLog(shader, info_len, nullptr, info_log.data());
      std::print(stderr, "{}", info_log);
    }
    std::println(stderr, "[Error]: GL shader index {} did not compile ({})", shader, filepath);
    return false;
  }
  return true;
}

bool create_program_from_filepath(const char* vert_filepath, const char* frag_filepath, GLuint& program)
{
  int result{};

  GLuint vert_shader{};
  result = create_shader_from_filepath(vert_filepath, vert_shader, GL_VERTEX_SHADER);
  if (!result)
  {
    return false;
  }

  GLuint frag_shader{};
  result = create_shader_from_filepath(frag_filepath, frag_shader, GL_FRAGMENT_SHADER);
  if (!result)
  {
    glDeleteShader(vert_shader);
    return false;
  }

  if (!create_program(vert_shader, frag_shader, program))
  {
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  return true;
}

bool create_shader_from_cstr(const char* content, GLuint& shader, int kind)
{
  shader = glCreateShader(kind);
  glShaderSource(shader, 1, &content, nullptr);
  glCompileShader(shader);

  int result{};

  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE)
  {
    GLint info_len{};

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 1)
    {
      std::string info_log;
      info_log.resize(info_len);
      glGetShaderInfoLog(shader, info_len, nullptr, info_log.data());
      std::print(stderr, "{}", info_log);
    }
    std::println(stderr, "[Error]: GL shader index {} did not compile", shader);
    return false;
  }
  return true;
}

bool create_program_from_cstr(const char* vert_cstr, const char* frag_cstr, GLuint& program)
{
  int result{};

  GLuint vert_shader{};
  result = create_shader_from_cstr(vert_cstr, vert_shader, GL_VERTEX_SHADER);
  if (!result)
  {
    return false;
  }


  GLuint frag_shader{};
  result = create_shader_from_cstr(frag_cstr, frag_shader, GL_FRAGMENT_SHADER);
  if (!result)
  {
    glDeleteShader(vert_shader);
    return false;
  }

  if (!create_program(vert_shader, frag_shader, program))
  {
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  return true;
}

#endif // end of UTILS_IMPLEMENTATION

#endif // end of INCLUDE_UTILS_H
