#ifndef SCENE_H
#define SCENE_H

#include <time.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <iostream>

class Scene
{
  enum buffers
  {
    VERTEX, UV, NORMAL, INDEX
  };

  struct VBO
  {
    VBO(GLuint v = 0, GLuint t = 0, GLuint n = 0, GLuint i = 0): _v(v), _t(t), _n(n), _i(i) {}
    GLuint _v, _t, _n, _i;
  };
    
  std::map<std::string, VBO>    _vboMap;
  std::map<std::string, GLuint> _textureMap;
  GLuint _program_2D;
  GLuint _program_3D;

  

  void loadVertex(GLvoid *vvp, size_t vvSize, 
                  GLvoid *uvp, size_t uvSize, 
                  GLvoid *ivp, size_t ivSize, 
                  GLvoid *nvp, size_t nvSize, const std::string& obj_name);

  void draw(GLuint tInd, 
            GLuint vBuf, 
            GLuint tBuf, 
            GLuint nBuf, 
            GLuint iBuf, 
            size_t vCount, glm::vec3 pos, glm::vec2 size);

  void delVBO(VBO &vbo);  

public:
  void frame();
  void cleanup();
  void load();
};

#endif