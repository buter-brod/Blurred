#ifndef SCENE_H
#define SCENE_H

#include <time.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <iostream>
#include <vector>

class Scene
{
public:
  ~Scene();

  struct Size
  {
    Size() :_x(0), _y(0) {}
    Size(size_t x, size_t y): _x(x), _y(y) {}
    size_t _x, _y;
  };

  enum mask_type
  {
    /* all three options are 'horizontal';
    SMOOTH: power grows slowly from left to right,
    EDGE:   power switching from min to max at the center
    PEAK_AT_CENTER: grows slowly, max at center, then decreases back to 0
    */

    SMOOTH, EDGE, PEAK_AT_CENTER
  };

  void SetSize(const Size& size);
  void Frame();
  
  void SetLightOn(bool lightOn);
  void SetAngle(float angle);
  void SetLightPower(float power);

  float GetAngle()        const {return _angle;}
  float GetLightPower()   const {return _lightPower;}
  bool GetLightOn()       const {return _lightOn;}
  Size GetRttSize()       const {return _sizes[RTT];}
  Size GetMaskSize()      const {return _sizes[MASK];}
  mask_type GetMaskType() const {return _mask_type;}

  void Load(const Size& rtt_size, const Size& mask_size, mask_type mask);

private:
  enum buffers
  {
    VERTEX, UV, NORMAL, INDEX
  };

  struct VBO
  {
    VBO(GLuint v = 0, GLuint t = 0, GLuint n = 0, GLuint i = 0, GLuint count = 0): _v(v), _t(t), _n(n), _i(i), _count(count) {}
    GLuint _v, _t, _n, _i, _count;
  };

  enum res_type { SCENE, RTT, MASK };

  struct obj_data
  {
    std::vector<float> _vs, _ns, _uvs;
    size_t _count;
  };
  
  GLuint _program_2D;
  GLuint _program_2D_blur;
  GLuint _program_3D;
  GLuint _framebufferInd;
  GLuint _renderedTexture;
  GLuint _blurMaskTex;
  GLuint _depthrenderbuffer;

  Size      _sizes[3];

  float     _angle       = 0.f;
  float     _objDistance = 3.f;
  float     _lightPower  = 8.f;

  bool      _ready   = false;
  bool      _lightOn = true;
  
  mask_type _mask_type;

  std::map<std::string, VBO>      _vboMap;
  std::map<std::string, GLuint>   _textureMap;
  std::map<std::string, obj_data> _objCache;

  const std::string _obj_filename     = "obj.obj";
  const std::string _bg_filename      = "background.png";
  const std::string _obj_tex_filename = "object.png";


  void prepareTexture(const std::string& obj_name, const std::string& filename);
  inline void prepareRTT();
  inline void buildBlurMask();

  void loadVertex(GLvoid *vvp, size_t vvSize,
    GLvoid *uvp, size_t uvSize,
    GLvoid *ivp, size_t ivSize,
    GLvoid *nvp, size_t nvSize, size_t count, const std::string& obj_name);

  inline void draw3DObject();

  void Scene::draw(GLuint tInd, VBO& vbo);

  void draw(GLuint tInd, 
            GLuint vBuf, 
            GLuint tBuf, 
            GLuint nBuf, 
            GLuint iBuf, 
            size_t vCount);

  void cleanup();
  void delVBO(VBO &vbo);  
};

#endif