#include "scene.h"
#include "utils.h"

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

  _vboMap[obj_name] = VBO(bufInd[VERTEX], bufInd[UV], bufInd[NORMAL], bufInd[INDEX], count);
}

void Scene::draw(GLuint tInd, VBO& vbo)
{
  draw(tInd, vbo._v, vbo._t, vbo._n, vbo._i, vbo._count);
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
  for(auto& vbo : _vboMap)
    delVBO(vbo.second);

  for(auto& tex : _textureMap)
    glDeleteTextures(1, &tex.second);

  _vboMap    .clear();
  _textureMap.clear();

  glDeleteProgram(_program_2D);
  glDeleteProgram(_program_3D);
}

void Scene::prepareTexture(const std::string& obj_name, const std::string& filename)
{
  if (_textureMap.count(obj_name) > 0)
    std::cerr << "error, trying to load texture for the same object " << obj_name.c_str();
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

void Scene::load()
{
  _angle = 0.f;

  prepareTexture("background", "chess.tga");
  prepareTexture("object",     "goodevil.tga");
  
  {//load 3D object

   std::vector<float> obj_vs, obj_ns;
   std::vector<float> obj_uvs;

   unsigned int v_count = utils::loadOBJ("ball.obj", obj_vs, obj_uvs, obj_ns);
   assert(v_count > 0 && "unable to load object");

   loadVertex(obj_vs.data(), obj_vs.size(), obj_uvs.data(), obj_uvs.size(), (GLvoid*)0, 0, obj_ns.data(), obj_ns.size(), v_count, "object");
  }
  
  //load background geometry
  loadVertex(bg_vertices, 4 * 3, bg_uvs, 4 * 2, bg_indices, 6, nullptr, 0, 6, "background");

  //load 2D program
  {
    _program_2D = 0;
    bool shaders_loaded_2D = utils::loadShaders("2D.vert", "2D.frag", _program_2D);
    assert(shaders_loaded_2D && "unable to load shaders");
  }

  //load 3D program  
  {
    _program_3D = 0;
    bool shaders_loaded_3D = utils::loadShaders("3D.vert", "3D.frag", _program_3D);
    assert(shaders_loaded_3D && "unable to load shaders");
  }
}

void Scene::frame()
{  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  {
    {
      //draw texture

      glUseProgram(_program_2D);
      GLuint matrix_id = glGetUniformLocation(_program_2D, "MVP");
      glm::mat4 bgViewM, bgModelM;
      glm::mat4 bgProjM = glm::ortho<float>(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
      glm::mat4 MVP = bgProjM * bgViewM * bgModelM;
      glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);

      //draw texture
      draw(_textureMap["background"], _vboMap["background"]);
    }
    glClear(GL_DEPTH_BUFFER_BIT);
  }
    
  {
    //draw obj to texture   
    
    const float offset = 5.0;
    const float light_power = 120.f;
    
    glUseProgram(_program_3D);

    GLuint light_id      = glGetUniformLocation(_program_3D, "LightPosition_worldspace");
    GLuint lightPower_id = glGetUniformLocation(_program_3D, "LightPower");
    GLuint lightOn_id    = glGetUniformLocation(_program_3D, "LightOn");

    glm::vec4 camPosition4(0.0, offset, offset, 0.f);
    glm::mat4 camRotM = glm::rotate(glm::mat4(), _angle, glm::vec3(0.0, 1.0, 0.0));
    glm::vec3 camPositionCurr = utils::xyz(camRotM * camPosition4);
    glm::mat4 projectionMatrix = glm::perspective(45.f, 1.f, 0.1f, 20.f);
    glm::mat4 modelMatrix = glm::mat4(1.0);
    glm::mat4 viewMatrix  = glm::lookAt(camPositionCurr, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

    GLuint matrix_id      = glGetUniformLocation(_program_3D, "MVP");
    GLuint viewMatrix_id  = glGetUniformLocation(_program_3D, "V");
    GLuint modelMatrix_id = glGetUniformLocation(_program_3D, "M"); 

    glUniformMatrix4fv(matrix_id,      1, GL_FALSE, &MVP        [0][0]);
    glUniformMatrix4fv(modelMatrix_id, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(viewMatrix_id,  1, GL_FALSE, &viewMatrix [0][0]);

    glUniform3f(light_id, camPositionCurr.x, camPositionCurr.y, camPositionCurr.z);
    glUniform1f(lightPower_id, light_power);

    draw(_textureMap["object"], _vboMap["object"]);
  }

  _angle += 0.01f;
}