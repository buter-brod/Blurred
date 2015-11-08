#include "utils.h"
#include "scene.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

const int screen_size[2] = {512, 512}; 
const float PI = 3.141592f;
const float g_rotationSpeed = 0.2f;  // full rounds per second

std::shared_ptr<Scene> g_scene;

float& angle()
{
  static float angle = 0.f;
  return angle;
}

float& FPS()
{
  static float fps = 30.f;
  return fps;
}

void cycle_fps()
{
  static const float fps[3] = {30.f, 45.f, 15.f};
  static unsigned int fps_ind = 0;

  fps_ind = (fps_ind + 1) % 3;
  FPS() = fps[fps_ind];

  std::cout << "FPS now " << FPS() << "\n";
}

void toggle_light()
{
  bool enable = !g_scene->GetLightOn();
  g_scene->SetLightOn(enable);

  std::cout << "Light is now " << (enable ? "ON" : "OFF") << "\n";
}

void cycle_rtt_size()
{
  static const size_t sizes[3][2] = {{512, 512}, {256, 256}, {128, 128}};
  static unsigned int cur_size = 0;

  cur_size = (cur_size + 1) % 3;

  g_scene->Load(Scene::Size(sizes[cur_size][0], sizes[cur_size][1]), g_scene->GetMaskSize(), g_scene->GetMaskType());

  std::cout << "RTT size now " << sizes[cur_size][0] << ", " << sizes[cur_size][1] << "\n";
}

void cycle_mask_type()
{
  Scene::mask_type mask_t = Scene::mask_type((g_scene->GetMaskType() + 1) % 3);

  g_scene->Load(g_scene->GetRttSize(), g_scene->GetMaskSize(), mask_t);

  std::cout << "mask type now " << (mask_t == Scene::SMOOTH ? "Smooth" : (mask_t == Scene::EDGE ? "Edge" : "Peak at center")) << "\n";
}

void changeLightPower(float dPower)
{
  float prevPower = g_scene->GetLightPower();
  float newPower = prevPower + dPower;
  if (newPower >= 0.f)
  {
    g_scene->SetLightPower(newPower);
    std::cout << "light power is " << newPower << "\n";
  }  
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS)
  {
    switch (key)
    {
    case GLFW_KEY_SPACE:
      toggle_light();
      break;
    case GLFW_KEY_ENTER:
      cycle_rtt_size();
      break;
    case GLFW_KEY_BACKSPACE:
      cycle_mask_type();
      break;
    case GLFW_KEY_TAB:
      cycle_fps();
      break;
    case GLFW_KEY_UP:
      changeLightPower(1.f);
      break;
    case GLFW_KEY_DOWN:
      changeLightPower(-1.f);
      break;
    default:
      break;
    }
  }
}

inline void rotate(float dt)
{
  angle() = std::fmod(angle() + (2.f * PI) * g_rotationSpeed * dt, 2.f * PI);
  g_scene->SetAngle(angle());
}

int main(void)
{
  if(glfwInit() != GL_TRUE)
  {
    std::cerr << "glfwInit failed";
    return -1;
  }
  
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  GLFWwindow* window = glfwCreateWindow(screen_size[0], screen_size[1], "Blurred", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  
  GLenum init_result = glewInit();
  if(init_result != GLEW_OK)
  {
    std::cerr << "glew init error : " << glewGetErrorString(init_result);
    return -1;
  }
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  clock_t timeLastRedraw = 0;

  Scene::Size rtt_size (screen_size[0], screen_size[1]);
  Scene::Size mask_size = rtt_size;
  Scene::mask_type mask_t = Scene::SMOOTH;

  g_scene = std::make_shared<Scene>();
  
  std::cout << "Loading scene..\n";
  g_scene->Load(rtt_size, mask_size, mask_t);
  g_scene->SetSize(Scene::Size(screen_size[0], screen_size[1]));
  g_scene->SetLightOn(true);

  glfwSetKeyCallback(window, key_callback);

  std::cout << 
    "Scene is ready! \n\n\
    Feel free to change the settings: \n\n\
    - TAB to change FPS \n\
    - BACKSPACE to change blur mask type \n\
    - ENTER to change RTT resolution \n\
    - SPACE to turn lights On/Off \n\
    - UP/DOWN ARROWS to change light power (when light is ON) \n\n\
    ENJOY!\n\n";

  do 
  {
    clock_t frameStartTime = clock();
    float delta = utils::dt(frameStartTime, timeLastRedraw);
    if(delta >= 1.f / FPS())
    {
      timeLastRedraw = frameStartTime;

      rotate(delta);
      
      g_scene->Frame();
      
      glfwSwapBuffers(window);
    }

    glfwPollEvents();    
  }
  while(!glfwWindowShouldClose(window));

  g_scene.reset();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}