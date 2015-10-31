
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "3rdparty.h"

const int g_screen_x = 1024;
const int g_screen_y = 768;

int main(void)
{
  if(glfwInit() != GL_TRUE)
  {
    fprintf(stderr, "Error: glfwInit failed");
    return -1;
  }
  
  glm::vec2 screen(g_screen_x, g_screen_y);
  GLFWwindow* window = glfwCreateWindow(screen.x, screen.y, "View", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  GLenum init_result = glewInit();
  if(init_result != GLEW_OK)
  {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(init_result));
    return -1;
  }

  

  return 0;
}