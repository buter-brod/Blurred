#include "scene.h"
#include "utils.h"

Scene::~Scene()
{
  cleanup();
}

void Scene::SetLightOn(bool lightOn)    {_lightOn = lightOn;  }
void Scene::SetSize   (const Size& size){_sizes[SCENE] = size;}
void Scene::SetAngle  (float angle)     {_angle = angle;      }

void Scene::loadVertex(GLvoid *vvp, size_t vvSize, GLvoid *uvp, size_t uvSize, GLvoid *ivp, size_t ivSize, GLvoid *nvp, size_t nvSize, size_t count, const std::string& obj_name)
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

  if(nvSize > 0)
  {
    glGenBuffers(1, &bufInd[NORMAL]);
    glBindBuffer(GL_ARRAY_BUFFER, bufInd[NORMAL]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * nvSize, nvp, GL_STATIC_DRAW);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  _vboMap[obj_name] = VBO(bufInd[VERTEX], 
                          bufInd[UV], 
                          bufInd[NORMAL],
                          bufInd[INDEX], 
                          count);
}

void Scene::draw(GLuint tInd, VBO& vbo)
{
  draw(tInd, vbo._v, 
             vbo._t, 
             vbo._n, 
             vbo._i, 
             vbo._count);
}

void Scene::draw(GLuint tInd, GLuint vBuf, GLuint tBuf, GLuint nBuf, GLuint iBuf, size_t vCount)
{
  glBindTexture(GL_TEXTURE_2D, tInd);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vBuf);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, tBuf);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  if(nBuf > 0) // normals provided
  {
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, nBuf);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  }
  else
    glDisableVertexAttribArray(2);

  if (iBuf > 0)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuf);
    glDrawElements(GL_TRIANGLES, vCount, GL_UNSIGNED_BYTE, (GLvoid*)0);
  }
  else
    glDrawArrays(GL_TRIANGLES, 0, vCount); // no index data available

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Scene::delVBO(VBO &vbo)
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteBuffers(1, &vbo._t);
  glDeleteBuffers(1, &vbo._v);

  if(vbo._n > 0)
    glDeleteBuffers(1, &vbo._n);

  if(vbo._i > 0)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo._i);
  }
}

void Scene::cleanup()
{
  if (!_ready)
    std::cerr << "warning: trying to cleanup empty scene";

  for(auto& vbo : _vboMap)
    delVBO(vbo.second);

  for(auto& tex : _textureMap)
    glDeleteTextures(1, &tex.second);

  _vboMap    .clear();
  _textureMap.clear();

  glDeleteProgram(_program_2D);
  glDeleteProgram(_program_3D);
  glDeleteProgram(_program_2D_blur);

  _program_2D      = 0;
  _program_3D      = 0;
  _program_2D_blur = 0;

  glDeleteFramebuffers (1, &_framebufferInd);
  glDeleteRenderbuffers(1, &_depthrenderbuffer);
  glDeleteTextures     (1, &_renderedTexture);
  glDeleteTextures     (1, &_blurMaskTex);

  _framebufferInd    = 0;
  _depthrenderbuffer = 0;
  _renderedTexture   = 0;
  _blurMaskTex       = 0;

  _angle = 0.f;
  _ready = false;
}

void Scene::prepareTexture(const std::string& obj_name, const std::string& filename)
{
  if (_textureMap.count(obj_name) > 0)
    std::cerr << "error, trying to reload texture for existing object " << obj_name.c_str();
  else
  {
    _textureMap[obj_name] = 0;
    bool loaded = utils::loadTexture(filename, _textureMap[obj_name]);
    if (!loaded)
    {
      std::cerr << filename.c_str() << ": unable to load texture.";
      assert(false);
    }    
    glGenerateTextureMipmap(_textureMap[obj_name]);
  }
}

void Scene::Load(const Size& rtt_size, const Size& mask_size, mask_type mask_t)
{
  if (_ready)
  {
    std::cout << "----\n"; // 'new scene' separator in logs
    cleanup();
  }

  _sizes[RTT]   = rtt_size;
  _sizes[MASK]  = mask_size;

  _mask_type = mask_t;

  prepareTexture("background", _bg_filename);
  prepareTexture("object",     _obj_tex_filename);
  
  {//load 3D object
   bool exist_in_cache = _objCache.count(_obj_filename) > 0;
   obj_data& obj = _objCache[_obj_filename];

   if (!exist_in_cache)
   {     
     obj._count = utils::loadOBJ(_obj_filename.c_str(), obj._vs, obj._uvs, obj._ns);
     assert(obj._count > 0 && "unable to load object");
   }

   loadVertex(obj. _vs.data(), 
              obj. _vs.size(), 
              obj._uvs.data(), 
              obj._uvs.size(), (GLvoid*)0, 0, 
              obj. _ns.data(), 
              obj. _ns.size(), obj._count, "object");
  }
  {
    //load background geometry

    static GLfloat bg_vertices[4][3] =
    {
      {-1.f, -1.f, 0.f},
      { 1.f, -1.f, 0.f},
      { 1.f,  1.f, 0.f},
      {-1.f,  1.f, 0.f}
    };

    static GLfloat bg_uvs[4][2] =
    {
      {0.f, 0.f},
      {1.f, 0.f},
      {1.f, 1.f},
      {0.f, 1.f}
    };

    static GLubyte bg_indices[] =
    {
      0, 1, 2,
      0, 2, 3
    };

    loadVertex(bg_vertices, 4 * 3, bg_uvs, 4 * 2, bg_indices, 6, nullptr, 0, 6, "background");
  } 

  bool shaders_loaded_2D   = utils::loadShaders("2D.vert",      "2D.frag",      _program_2D);
  bool shaders_loaded_3D   = utils::loadShaders("3D.vert",      "3D.frag",      _program_3D);
  bool shaders_loaded_2D_b = utils::loadShaders("2D_blur.vert", "2D_blur.frag", _program_2D_blur);

  assert(shaders_loaded_2D   && 
         shaders_loaded_3D   && 
         shaders_loaded_2D_b && 
         "failed to load shaders");

  prepareRTT();
  buildBlurMask();

  _ready = true;
}

void Scene::buildBlurMask()
{
  glGenTextures(1, &_blurMaskTex);
  glBindTexture(GL_TEXTURE_2D, _blurMaskTex);

  unsigned char *image = new unsigned char [_sizes[MASK]._x * _sizes[MASK]._y];
  
  const size_t center = _sizes[MASK]._x / 2;

  for  (unsigned int y = 0; y < _sizes[MASK]._y; y++)
    for(unsigned int x = 0; x < _sizes[MASK]._x; x++)
    {
      unsigned char value = 0;

      switch (_mask_type)
      {
      case SMOOTH:
        value = unsigned char(UCHAR_MAX * float(x) / (_sizes[MASK]._x - 1));
        break;
      case EDGE:
        value = (x < _sizes[MASK]._x / 2) ? 0 : UCHAR_MAX;
        break;
      case PEAK_AT_CENTER:
        value = unsigned char((1.f - abs(float(center) - x) / (_sizes[MASK]._x / 2.f)) * UCHAR_MAX);
        break;
      default:
        assert(false && "unknown mask type");
      }
      image[_sizes[MASK]._x * y + x] = value;
    }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // mask is one-channel texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _sizes[MASK]._x, _sizes[MASK]._y, 0, GL_RED, GL_UNSIGNED_BYTE, image);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  glGenerateTextureMipmap(_blurMaskTex);

  delete[] image;
}

void Scene::prepareRTT()
{
  glGenFramebuffers(1, &_framebufferInd);
  glBindFramebuffer(GL_FRAMEBUFFER, _framebufferInd);
  
  glGenTextures(1, &_renderedTexture);
  glBindTexture(GL_TEXTURE_2D, _renderedTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _sizes[RTT]._x, _sizes[RTT]._y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glGenRenderbuffers(1, &_depthrenderbuffer);
  glBindRenderbuffer   (GL_RENDERBUFFER, _depthrenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _sizes[RTT]._x, _sizes[RTT]._y);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthrenderbuffer);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _renderedTexture, 0);
  
  GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
  glDrawBuffers(1, &drawBuffer);
  auto st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(st != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cerr << "glDrawBuffers error: " << st;
    assert(false && "glDrawBuffers");
  }
}

void Scene::draw3DObject()
{
  glUseProgram(_program_3D);

  GLuint      light_id = glGetUniformLocation(_program_3D, "LightPosition_worldspace");
  GLuint lightPower_id = glGetUniformLocation(_program_3D, "LightPower");
  GLuint    lightOn_id = glGetUniformLocation(_program_3D, "Light_On");

  glm::mat4 camRotM = glm::rotate(glm::mat4(), _angle, glm::vec3(0.0, 1.0, 0.0));
  glm::vec3 camPositionCurr = utils::xyz(camRotM * glm::vec4(0.0, _objDistance, _objDistance, 0.f));
  glm::mat4 viewMatrix  = glm::lookAt(camPositionCurr, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  
  glm::mat4 projectionMatrix = glm::perspective(45.f, 1.f, 0.1f, 20.f);
  glm::mat4 modelMatrix = glm::mat4(1.0);

  glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

  GLuint      matrix_id = glGetUniformLocation(_program_3D, "MVP");
  GLuint  viewMatrix_id = glGetUniformLocation(_program_3D, "V");
  GLuint modelMatrix_id = glGetUniformLocation(_program_3D, "M");

  glUniformMatrix4fv(     matrix_id, 1, GL_FALSE, &MVP        [0][0]);
  glUniformMatrix4fv(modelMatrix_id, 1, GL_FALSE, &modelMatrix[0][0]);
  glUniformMatrix4fv( viewMatrix_id, 1, GL_FALSE, &viewMatrix [0][0]);

  glUniform3f(     light_id, camPositionCurr.x, camPositionCurr.y, camPositionCurr.z);
  glUniform1f(lightPower_id, _lightPower);
  glUniform1f(   lightOn_id, _lightOn ? 1.f : 0.f);

  draw(_textureMap["object"], _vboMap["object"]);
}

void Scene::Frame()
{
  glBindFramebuffer(GL_FRAMEBUFFER, _framebufferInd);
  glViewport(0, 0, _sizes[RTT]._x, _sizes[RTT]._x);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 viewM_2D, bgModelM_2D, projM_2D, mvpM_2D;
  {
    projM_2D = glm::ortho<float>(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    mvpM_2D = projM_2D * viewM_2D * bgModelM_2D;
  }
  {
    // background (RTT)
    glDepthMask(GL_FALSE); // avoid writing depth here
    glUseProgram(_program_2D);
    GLuint matrix_id = glGetUniformLocation(_program_2D, "MVP");
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvpM_2D[0][0]);
    draw(_textureMap["background"], _vboMap["background"]);
    glDepthMask(GL_TRUE);
  }

  draw3DObject(); // object RTT

  //RTT finished, now rendering to main scene
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, _sizes[SCENE]._x, _sizes[SCENE]._y);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  {
    glUseProgram(_program_2D_blur);
    GLuint matrix_id = glGetUniformLocation(_program_2D_blur, "MVP");
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvpM_2D[0][0]);

    glActiveTexture(GL_TEXTURE0 + 1); // fragment shader will use two textures, second one for one-channel blur mask 0..1
    glBindTexture(GL_TEXTURE_2D, _blurMaskTex);
    glActiveTexture(GL_TEXTURE0);

    GLuint texture_id     = glGetUniformLocation(_program_2D_blur, "currTex");
    GLuint textureMask_id = glGetUniformLocation(_program_2D_blur, "maskTex");

    glUniform1i(    texture_id, 0);
    glUniform1i(textureMask_id, 1);

    draw(_renderedTexture, _vboMap["background"]);
  }
}