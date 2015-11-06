#include "scene.h"
#include "utils.h"



//enum units
//{
//  BACKGROUND = 0, OBJECT = 1
//};

GLfloat bg_vertices[4][3] =
{
  {-1.f, -1.f, 0.f},
  {1.f, -1.f, 0.f},
  {1.f,  1.f, 0.f},
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

void Scene::loadVertex(GLvoid *vvp, size_t vvSize, GLvoid *uvp, size_t uvSize, GLvoid *ivp, size_t ivSize, GLvoid *nvp, size_t nvSize, const std::string& obj_name)
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

  _vboMap[obj_name] = VBO(bufInd[VERTEX], bufInd[UV], bufInd[NORMAL], bufInd[INDEX]);
}

void Scene::draw(GLuint tInd, GLuint vBuf, GLuint tBuf, GLuint nBuf, GLuint iBuf, size_t vCount, glm::vec3 pos, glm::vec2 size)
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

  if(iBuf > 0)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuf);
  }

  glDrawElements(GL_TRIANGLES, vCount, GL_UNSIGNED_BYTE, (GLvoid*)0);

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
  for(auto vbo : _vboMap)
    delVBO(vbo.second);

  for(auto tex : _textureMap)
    glDeleteTextures(1, &tex.second);

  _vboMap    .clear();
  _textureMap.clear();

  glDeleteProgram(_program_2D);
  glDeleteProgram(_program_3D);
}

void Scene::load()
{
  {//load background texture
    _textureMap["background"] = 0;
    bool bg_loaded = loadTexture("chess.tga", _textureMap["background"]);
    assert(bg_loaded && "unable to load bg texture");
    glGenerateTextureMipmap(_textureMap["background"]);
  }
  
  {//load 3D object

   std::vector<float> obj_vs, obj_ns;
   std::vector<float> obj_uvs;

   bool obj_loaded = loadOBJ("ball.obj", obj_vs, obj_uvs, obj_ns);
   assert(obj_loaded && "unable to load object");    

   loadVertex(obj_vs.data(), obj_vs.size(), obj_uvs.data(), obj_uvs.size(), (GLvoid*)0, 0, obj_ns.data(), obj_ns.size(), "object"); 
  }
  
  //load background geometry
  loadVertex(bg_vertices, 4 * 3, bg_uvs, 4 * 2, bg_indices, 6, nullptr, 0, "background");

  //load 2D program
  {
    _program_2D = 0;
    bool shaders_loaded_2D = loadShaders("2D.vert", "2D.frag", _program_2D);
    assert(shaders_loaded_2D && "unable to load shaders");
  }

  //load 3D program 
  {
    _program_3D = 0;
    bool shaders_loaded_3D = loadShaders("3D.vert", "3D.frag", _program_3D);
    assert(shaders_loaded_3D && "unable to load shaders");
  }
  
  {
    //GLuint light_id      = glGetUniformLocation(_program_id, "LightPosition_worldspace");   
    //GLuint lightPower_id = glGetUniformLocation(_program_id, "LightPower");   
    //GLuint lightOn_id    = glGetUniformLocation(_program_id, "LightOn");

    
    //GLuint viewMatrix_id  = glGetUniformLocation(_program_id, "V");
    //GLuint modelMatrix_id = glGetUniformLocation(_program_id, "M");


    //glUniform1i(lightOn_id, 0);
    //glm::mat4 bgViewM, bgModelM;

    //glm::mat4 bgProjM = glm::ortho<float>(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    //glm::mat4 MVP = bgProjM * bgViewM * bgModelM;

    //glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);
    //glUniformMatrix4fv(modelMatrix_id, 1, GL_FALSE, &bgModelM[0][0]);
    //glUniformMatrix4fv(viewMatrix_id, 1, GL_FALSE, &bgViewM[0][0]);
  }
}

void Scene::frame()
{  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
  {
    //draw obj to texture
  }

  {
    //draw texture

    glUseProgram(_program_2D);
    //GLuint texture_id = glGetUniformLocation(_program_2D, "currTex");
    //glUniform1i(texture_id, BACKGROUND);
    GLuint matrix_id = glGetUniformLocation(_program_2D, "MVP");
    glm::mat4 bgViewM, bgModelM;
    glm::mat4 bgProjM = glm::ortho<float>(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    glm::mat4 MVP = bgProjM * bgViewM * bgModelM;
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);

    //draw texture
    draw(_textureMap["background"],
      _vboMap["background"]._v,
      _vboMap["background"]._t,
      _vboMap["background"]._n,
      _vboMap["background"]._i,
      6, glm::vec3(0.f, 0.f, 0.f), glm::vec2(0.5f, 0.5f));
  }
}