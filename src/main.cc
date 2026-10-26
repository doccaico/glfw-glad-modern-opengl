#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <print>
#include <utility>

namespace fs = std::filesystem;

constexpr int window_width = 640;
constexpr int window_height = 480;

std::pair<bool, std::pair<int, int>> get_window_center_pos()
{
  GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
  if (!primary_monitor)
  {
    std::println(stderr, "[Error]: Failed to get primary monitor");
    return { false, { 0, 0 } };
  }
  const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
  if (!mode)
  {
    std::println(stderr, "[Error]: Failed to get video modes");
    return { false, { 0, 0 } };

  }
  return { true, { (mode->width - window_width)/2, (mode->height - window_height)/2 } };
}

bool read_file(std::string& content, fs::path target_path)
{
  // Check if the file exists and is a regular file
  if (!fs::exists(target_path) || !fs::is_regular_file(target_path))
  {
    std::println(stderr,
        "[Error]: File does not exist or is not a regular file: {}", target_path.string());
    return false;
  }

  // Get the file size
  std::uintmax_t size = fs::file_size(target_path);

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

std::pair<bool, GLuint> create_shader_prog_from_files(const char* vert_file_name, const char* frag_file_name)
{
  int error_param;

  // VERTEX_SHADER
  fs::path vert_path{vert_file_name};
  std::string vert_str{};
  if (!read_file(vert_str, vert_path))
  {
    return { false, 0 };
  }
  std::println("[Log]: creating shader from \"{}\"", vert_file_name);
  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  const char* const vert_str_ptr = vert_str.c_str();
  glShaderSource(vert_shader, 1, &vert_str_ptr, nullptr);
  glCompileShader(vert_shader);

  glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &error_param);
  if (error_param != GL_TRUE)
  {
    GLint info_len = 0;
    glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 1)
    {
      std::string info_log;
      info_log.resize(info_len);
      glGetShaderInfoLog(vert_shader, info_len, nullptr, info_log.data());
      std::print(stderr, "{}", info_log);
    }
    std::println(stderr, "[Error]: GL shader index {} did not compile", vert_shader);
    return { false, 0 };
  }

  // FRAGMENT_SHADER
  fs::path frag_path{frag_file_name};
  std::string frag_str{};
  if (!read_file(frag_str, frag_path))
  {
    return { false, 0 };
  }
  std::println("[Log]: creating shader from \"{}\"", frag_file_name);
  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  const char* const frag_str_ptr = frag_str.c_str();
  glShaderSource(frag_shader, 1, &frag_str_ptr, nullptr);
  glCompileShader(frag_shader);

  glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &error_param);
  if (error_param != GL_TRUE)
  {
    GLint info_len = 0;
    glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 1)
    {
      std::string info_log;
      info_log.resize(info_len);
      glGetShaderInfoLog(frag_shader, info_len, nullptr, info_log.data());
      std::print(stderr, "{}", info_log);
    }
    std::println(stderr, "[Error]: GL shader index {} did not compile", frag_shader);
    return { false, 0 };
  }

  // PROGRAM
  GLuint program = glCreateProgram();
  std::println("[Log]: created program {}. attaching shaders {} and {}", program, vert_shader, frag_shader );
  glAttachShader(program, vert_shader);
  glAttachShader(program, frag_shader);

  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &error_param);
  if (error_param != GL_TRUE)
  {
    GLint info_len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 1)
    {
      std::string info_log;
      info_log.resize(info_len);
      glGetProgramInfoLog(program, info_len, nullptr, info_log.data());
      std::print(stderr, "{}", info_log);
    }
    std::println(stderr, "[Error]: could not link shader program GL index {}", program );
    return { false, 0 };
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  return { true, program };
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  (void) scancode;
  (void) mods;

  // Close the window when the ESC key is pressed
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

int main()
{
  // Initialize the window system (GLFW)
  if (glfwInit() != GLFW_TRUE)
  {
    std::println(stderr, "[Error]: Failed to initialize GLFW");
    return -1;
  }

  // Set OpenGL version and profile options (optional, but recommended for modern OpenGL)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

  GLFWwindow* window = glfwCreateWindow(window_width, window_height, "OpenGL + GLFW + GLAD", nullptr, nullptr);
  if (window == nullptr)
  {
    std::println(stderr, "[Error]: Failed to create GLFW window");
    glfwTerminate();
    return -1;
  }

  auto [get_window_center_pos_ok, position] = get_window_center_pos();
  if (!get_window_center_pos_ok)
  {
    glfwTerminate();
    return -1;
  }
  auto [win_pos_x, win_pos_y] = position;
  glfwSetWindowPos(window, win_pos_x, win_pos_y);
  glfwShowWindow(window);

  // Make the created window's context current for the calling thread
  glfwMakeContextCurrent(window);

  // Initialize OpenGL context
  int gl_version = gladLoadGL(glfwGetProcAddress);
  if (gl_version == 0) {
    std::println("[Error]: Failed to initialize OpenGL context");
    glfwTerminate();
    return -1;
  }

  // Successfully loaded OpenGL
  std::println("[Log]: Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(gl_version), GLAD_VERSION_MINOR(gl_version));

  // Register the callback function to receive keyboard input events
  glfwSetKeyCallback(window, key_callback);

  // Enable vertical synchronization (V-Sync) to lock the frame rate
  glfwSwapInterval(1); 

  // Set the background clear color (Red, Green, Blue, Alpha) - dark navy blue here
  glClearColor(0.1f, 0.15f, 0.25f, 1.0f);

  // Setting up the triangle data
  float vertices[] = {
    -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f
  };

  GLuint attribPos = 0;
  GLuint attribCol = 1;

  // VBO
  GLuint  vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);

  // VAO
  GLuint vao;
  glCreateVertexArrays(1, &vao);

  GLuint vaoBindingPoint = 0; // A binding point in VAO. See GL_MAX_VERTEX_ATTRIB_BINDINGS
  glVertexArrayVertexBuffer(
      vao,                   // vao to bind
      vaoBindingPoint,       // Could be 1, 2... if there were several vbo to source.
      vbo,                   // VBO to bound at "vaoBindingPoint".
      0,                     // offset of the first element in the buffer vbo.
      6*sizeof(float));      // stride == 3 position floats + 3 color floats.

  // Need to precise vao, as there is no context binding in DSA style
  glEnableVertexArrayAttrib(vao, attribPos);
  // Meaning no current vao is bound to the opengl context.
  glEnableVertexArrayAttrib(vao, attribCol);

  // Need to precise vao, as there is no context binding in DSA
  glVertexArrayAttribFormat(vao, attribPos, 3, GL_FLOAT, false, 0);
  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribFormat.xhtml
  glVertexArrayAttribFormat(vao, attribCol, 3, GL_FLOAT, false, 3*sizeof(float));

  // Explicit binding of an attribute to a vao binding point
  glVertexArrayAttribBinding(vao, attribPos, vaoBindingPoint);
  glVertexArrayAttribBinding(vao, attribCol, vaoBindingPoint);

  // Create shader_prog
  auto [create_shader_prog_from_files_ok , shader_prog] =
    create_shader_prog_from_files("triangle.vert", "triangle.frag");
  if (!create_shader_prog_from_files_ok)
  {
    glfwTerminate();
    return -1;
  }

  glUseProgram(shader_prog); // Could be copy-pasted in render loop too...

  // Main loop (repeats until the window is closed)
  while (glfwWindowShouldClose(window) != GLFW_TRUE)
  {
    // Clear the color buffer (fill with the background color specified above)
    glClear(GL_COLOR_BUFFER_BIT);

    // Write polygon or triangle rendering code here
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Swap front and back buffers (update the screen)
    glfwSwapBuffers(window);
    // Detect and process input events such as mouse, keyboard, and window movement
    glfwPollEvents();
  }

  // Clean up
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(shader_prog);

  // Terminate
  glfwTerminate();

  return 0;
}
