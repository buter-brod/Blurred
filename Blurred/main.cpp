
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "utils.h"
#include <map>

#include <glm/gtc/matrix_transform.hpp>

const float FPS   = 60.f;
const float DELAY = 1.f / FPS;

const int g_screen_x = 512;
const int g_screen_y = 512;

clock_t g_timeLastRedraw;

enum buffers
{ 
  VERTEX, UV, NORMAL, INDEX
};

struct VBO
{
  VBO(GLuint v = 0, GLuint t = 0, GLuint n = 0, GLuint i = 0): _v(v), _t(t), _n(n), _i(i) {}
  GLuint _v, _t, _n, _i;
};
std::map<std::string, VBO> _vboMap;
std::map<std::string, GLuint> _textureMap;

void loadVertex(GLvoid *vvp, size_t vvSize, GLvoid *uvp, size_t uvSize, GLvoid *ivp, size_t ivSize, GLvoid *nvp, size_t nvSize, const std::string& obj_name)
{
  if(_vboMap.count(obj_name) > 0)
  {
    std::cerr << obj_name.c_str() << " VBO already exists with ids=" << _vboMap[obj_name]._t << ", " << _vboMap[obj_name]._v << ", " << _vboMap[obj_name]._n;
    return;
  }

  GLuint bufInd[4] = {0, 0, 0, 0};

  if(vvSize > 0)
  {
    glGenBuffers(1, &bufInd[VERTEX]);
    glBindBuffer(GL_ARRAY_BUFFER, bufInd[VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vvSize, vvp, GL_STATIC_DRAW);
  }
  
  if(uvSize > 0)
  {
    glGenBuffers(1, &bufInd[UV]);
    glBindBuffer(GL_ARRAY_BUFFER, bufInd[UV]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uvSize, uvp, GL_STATIC_DRAW);
  }

  if(ivSize > 0)
  {
    glGenBuffers(1, &bufInd[INDEX]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufInd[INDEX]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * ivSize, ivp, GL_STATIC_DRAW);
  }

  if (nvSize > 0)
  {
    glGenBuffers(1, &bufInd[NORMAL]);
    glBindBuffer(GL_ARRAY_BUFFER, bufInd[NORMAL]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * nvSize, nvp, GL_STATIC_DRAW);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  _vboMap[obj_name] = VBO(bufInd[VERTEX], bufInd[UV], bufInd[NORMAL], bufInd[INDEX]);
}

void draw(GLuint tInd, GLuint vBuf, GLuint tBuf, GLuint nBuf, GLuint iBuf, size_t vCount, glm::vec3 pos, glm::vec2 size, float opacity)
{
  glBindTexture(GL_TEXTURE_2D, tInd);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vBuf);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  //glVertexPointer(3, GL_FLOAT, 0, 0); // for pipeline

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, tBuf);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  //glTexCoordPointer(2, GL_FLOAT, 0, 0); //for pipeline

  if(nBuf > 0) // normals provided
  {
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, nBuf);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    //glNormalPointer(GL_FLOAT, 0, 0); // for pipeline
  }

  if (iBuf > 0)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuf);
  }  

  glDrawElements(GL_TRIANGLES, vCount, GL_UNSIGNED_BYTE, (GLvoid*)0);

  glBindBuffer(GL_ARRAY_BUFFER,         0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void delVBO(VBO &vbo)
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteBuffers(1, &vbo._t);
  glDeleteBuffers(1, &vbo._v);
  
  if (vbo._n > 0)
    glDeleteBuffers(1, &vbo._n);

  if(vbo._i > 0)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo._i);
  }
}

void cleanup()
{
  for(auto vbo : _vboMap)
    delVBO(vbo.second);

  for(auto tex : _textureMap)
    glDeleteTextures(1, &tex.second);

  _vboMap    .clear();
  _textureMap.clear();
}

void frame()
{
  clock_t frameStartTime = clock();
  float delta = dt(frameStartTime, g_timeLastRedraw);
  if(delta >= DELAY)
  {
    g_timeLastRedraw = frameStartTime;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //draw obj to texture
    
    {
      //draw texture
      draw(_textureMap["background"], 
               _vboMap["background"]._v, 
               _vboMap["background"]._t, 
               _vboMap["background"]._n, 
               _vboMap["background"]._i, 
        6, glm::vec3(0.f, 0.f, 0.f), glm::vec2(0.5f, 0.5f), 0.5f);
    }
  }
}

int main(void)
{
  if(glfwInit() != GL_TRUE)
  {
    std::cerr << "glfwInit failed";
    return -1;
  }
  
  GLFWwindow* window = glfwCreateWindow(g_screen_x, g_screen_y, "View", NULL, NULL);
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

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glFrontFace(GL_CW);

  GLfloat bg_vertices[4][3] =
  {
    {-1.f, -1.f, 0.f},
    { 1.f, -1.f, 0.f},
    { 1.f,  1.f, 0.f},
    {-1.f,  1.f, 0.f}
  };

  GLfloat bg_uvs[4][2] =
  {
    {0.f, 0.f},
    {1.f, 0.f},
    {1.f, 1.f},
    {0.f, 1.f}
  };

  GLubyte bg_indices[] =
  {
    0, 1, 2,
    0, 2, 3
  };
  
  _textureMap["background"] = 0;
  bool bg_loaded = loadTexture("chess.tga", _textureMap["background"]);
  assert(bg_loaded && "unable to load bg texture");

  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D); 
  }

  std::vector<float> obj_vs, obj_ns;
  std::vector<float> obj_uvs;

  //bool obj_loaded = loadOBJ("ball.obj", obj_vs, obj_uvs, obj_ns);
  //assert(obj_loaded && "unable to load object");    

  //loadVertex(obj_vs.data(), obj_vs.size(), obj_uvs.data(), obj_uvs.size(), obj_ns.data(), obj_ns.size(), "object");

  loadVertex(bg_vertices, 4*3, bg_uvs, 4*2, bg_indices, 6, nullptr, 0, "background");
  
  GLuint program_id = 0;
  bool shaders_loaded = loadShaders("v.vert", "f.frag", program_id);
  assert(shaders_loaded && "unable to load shaders");

  GLuint light_id      = glGetUniformLocation(program_id, "LightPosition_worldspace");
  GLuint lightPower_id = glGetUniformLocation(program_id, "LightPower");
  GLuint lightOn_id    = glGetUniformLocation(program_id, "LightOn");

  GLuint matrix_id      = glGetUniformLocation(program_id, "MVP");
  GLuint viewMatrix_id  = glGetUniformLocation(program_id, "V");
  GLuint modelMatrix_id = glGetUniformLocation(program_id, "M");

  glUseProgram(program_id);

  {
    GLuint texture_id = glGetUniformLocation(program_id, "currTex");
    glUniform1i(texture_id, 0);

    glUniform1i(lightOn_id, 0);
    glm::mat4 bgViewM, bgModelM;

    glm::mat4 bgProjM = glm::ortho<float>(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    glm::mat4 MVP = bgProjM * bgViewM * bgModelM;

    glUniformMatrix4fv(matrix_id,      1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(modelMatrix_id, 1, GL_FALSE, &bgModelM[0][0]);
    glUniformMatrix4fv(viewMatrix_id,  1, GL_FALSE, &bgViewM[0][0]);
  }

  do 
  {
    frame();   
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  while(!glfwWindowShouldClose(window));

  cleanup();
  glDeleteProgram(program_id);

  glfwDestroyWindow(window);
  glfwTerminate();
  std::cout << "Cool!";
    
  return 0;
}