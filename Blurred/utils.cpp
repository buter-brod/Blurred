#include "utils.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include "FreeImage.h"

namespace utils
{
  glm::vec3 xyz(glm::vec4 v) 
  {
    return glm::vec3(v.x, v.y, v.z); 
  }

  float dt(clock_t first, clock_t second)
  {
    return std::abs(float(first) - float(second)) / CLOCKS_PER_SEC;
  }


   bool loadTexture(const std::string& texName, GLuint& id)
  {  
    FIBITMAP* bitmap = FreeImage_Load(FreeImage_GetFileType(texName.c_str(), 0), texName.c_str());
    if(!bitmap)
    {
      std::cerr << "unable to load texture " << texName;
      return false;
    }

    BITMAPINFO *info = FreeImage_GetInfo(bitmap);

    GLsizei w = (GLsizei)FreeImage_GetWidth(bitmap);
    GLsizei h = (GLsizei)FreeImage_GetHeight(bitmap);

    GLuint texInd = -1;
    glGenTextures(1, &texInd);
    glBindTexture(GL_TEXTURE_2D, texInd);
    void* bits = FreeImage_GetBits(bitmap);

    bool with_alpha = info->bmiHeader.biBitCount > 24;

    glTexImage2D(GL_TEXTURE_2D, 0, with_alpha ? GL_RGBA : GL_RGB, w, h,
      0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bits);

    FreeImage_Unload(bitmap);
    id = texInd;
    return true;
  }

   unsigned int loadOBJ(const char * path,
               std::vector<float>& out_vertices, 
               std::vector<float>& out_uvs,
               std::vector<float>& out_normals
               )
  {
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices; 
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
  
    FILE* file = nullptr;
    fopen_s(&file, path, "r");

    if( file == NULL ){
      std::cerr << "cannot open file " << path;
      return 0;
    }
    while( 1 ){

      char lineHeader[128];
      int res = fscanf_s(file, "%s", lineHeader, 128);
      if (res == EOF)
        break;
      if ( strcmp( lineHeader, "v" ) == 0 ){
        glm::vec3 vertex;
        fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
        temp_vertices.push_back(vertex);
      }else if ( strcmp( lineHeader, "vt" ) == 0 ){
        glm::vec2 uv;
        fscanf_s(file, "%f %f\n", &uv.x, &uv.y );
        temp_uvs.push_back(uv);
      }else if ( strcmp( lineHeader, "vn" ) == 0 ){
        glm::vec3 normal;
        fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
        temp_normals.push_back(normal);
      }else if ( strcmp( lineHeader, "f" ) == 0 ){
        std::string vertex1, vertex2, vertex3;
        unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
        int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
        if (matches != 9){
          std::cerr << "can't parse file " << path;
          return false;
        }
        vertexIndices.push_back(vertexIndex[0]);
        vertexIndices.push_back(vertexIndex[1]);
        vertexIndices.push_back(vertexIndex[2]);
      
        uvIndices    .push_back(uvIndex[0]);
        uvIndices    .push_back(uvIndex[1]);
        uvIndices    .push_back(uvIndex[2]);

        normalIndices.push_back(normalIndex[0]);
        normalIndices.push_back(normalIndex[1]);
        normalIndices.push_back(normalIndex[2]);
      }else{
        char stupidBuffer[1000];
        fgets(stupidBuffer, 1000, file);
      }
    }

    for(unsigned int i = 0; i < vertexIndices.size(); i++)
    {
      unsigned int vertexIndex = vertexIndices[i];
      unsigned int uvIndex = uvIndices[i];
      unsigned int normalIndex = normalIndices[i];

      glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
      glm::vec2 uv = temp_uvs[ uvIndex-1 ];
      glm::vec3 normal = temp_normals[ normalIndex-1 ];

      out_vertices.push_back(vertex[0]);
      out_vertices.push_back(vertex[1]);
      out_vertices.push_back(vertex[2]);
      out_uvs     .push_back(uv[0]);
      out_uvs     .push_back(uv[1]);
      out_normals .push_back(normal[0]);
      out_normals .push_back(normal[1]);
      out_normals .push_back(normal[2]);
    }
    return vertexIndices.size();
  }

   bool loadShaders(const char *vertex_file_path,const char *fragment_file_path, GLuint& id)
  {
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
    GLint Result = GL_FALSE;
    int InfoLogLength;
    char const *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
      glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
      std::cerr << "shader error: " << &VertexShaderErrorMessage[0];
      return false;
    }
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
      glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
      std::cerr << "shader error: " << &FragmentShaderErrorMessage[0];
      return false;
    }
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> ProgramErrorMessage(InfoLogLength+1);
      glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
      std::cerr << "shader error: " << &ProgramErrorMessage[0];
      return false;
    }
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    id = ProgramID;
    return true;
  }
}