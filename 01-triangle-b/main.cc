#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <print>
#include <utility>

#define UTILS_IMPLEMENTATION
#include "utils.hpp"

#define SRC_DIR "01-triangle-from-files"

constexpr int window_width = 640;
constexpr int window_height = 480;

constexpr const char* vert_cstr =
  "#version 460\n"
  "\n"
  "layout(location = 0) in vec3 aPos;\n"
  "layout(location = 1) in vec3 aColor;\n"
  "\n"
  "out vec4 vertexColor;\n"
  "\n"
  "void main ()\n"
  "{\n"
  "  gl_Position = vec4(aPos, 1.0);\n"
  "  vertexColor = vec4(aColor, 1.0);\n"
  "}\n";

constexpr const char* frag_cstr =
  "#version 460\n"
  "\n"
  "in vec4 vertexColor;\n"
  "out vec4 FragColor;\n"
  "\n"
  "void main()\n"
  "{\n"
  "    FragColor = vertexColor;\n"
  "}\n";

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

  Position pos;
  if (!get_window_center_pos(window_width, window_height, pos))
  {
    glfwTerminate();
    return -1;
  }
  glfwSetWindowPos(window, pos.width, pos.height);
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

  GLuint attribPos{0};
  GLuint attribCol{1};

  // VBO
  GLuint  vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);

  // VAO
  GLuint vao;
  glCreateVertexArrays(1, &vao);

  GLuint vaoBindingPoint{0}; // A binding point in VAO. See GL_MAX_VERTEX_ATTRIB_BINDINGS
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

  // Create a program
  GLuint program{};
  if (!create_program_from_cstr(vert_cstr, frag_cstr, program))
  {
    glfwTerminate();
    return -1;
  }

  glUseProgram(program); // Could be copy-pasted in render loop too...

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
  glDeleteProgram(program);

  // Terminate
  glfwTerminate();

  return 0;
}
