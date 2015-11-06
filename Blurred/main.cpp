#include "utils.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include "scene.h"

const int screen_size[2] = {512, 512};

const float FPS   = 60.f;
const float DELAY = 1.f / FPS;

int main(void)
{
  if(glfwInit() != GL_TRUE)
  {
    std::cerr << "glfwInit failed";
    return -1;
  }
  
  GLFWwindow* window = glfwCreateWindow(screen_size[0], screen_size[1], "View", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  GLenum init_result = glewInit();
  if(init_result != GLEW_OK)
  {
    std::cerr << "glew init error : " << glewGetErrorString(init_result);
    return -1;
  }
  
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  clock_t timeLastRedraw = 0;

  //glFrontFace(GL_CW);

  Scene scene;
  scene.load();

  //std::vector<float> obj_vs, obj_ns;
  //std::vector<float> obj_uvs;

  //bool obj_loaded = loadOBJ("ball.obj", obj_vs, obj_uvs, obj_ns);
  //assert(obj_loaded && "unable to load object");    

  //loadVertex(obj_vs.data(), obj_vs.size(), obj_uvs.data(), obj_uvs.size(), obj_ns.data(), obj_ns.size(), "object");
  
  do 
  {
    clock_t frameStartTime = clock();
    float delta = dt(frameStartTime, timeLastRedraw);
    if(delta >= DELAY)
    {
      scene.frame();
      glfwSwapBuffers(window);
      timeLastRedraw = frameStartTime;
    }

    glfwPollEvents();
  }
  while(!glfwWindowShouldClose(window));

  scene.cleanup();
  

  glfwDestroyWindow(window);
  glfwTerminate();
  std::cout << "Cool!";
    
  return 0;
}