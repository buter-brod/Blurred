#include <stdio.h>
#include <string>
#include <fstream>

#include <GL/glew.h>
#include "3rdparty.h"

bool loadOBJ(const char * path, 
             std::vector<glm::vec3> & out_vertices, 
             std::vector<glm::vec2> & out_uvs,
             std::vector<glm::vec3> & out_normals
             )
{
  printf("Loading OBJ file %s...\n", path);

  std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
  std::vector<glm::vec3> temp_vertices; 
  std::vector<glm::vec2> temp_uvs;
  std::vector<glm::vec3> temp_normals;

  FILE * file = fopen(path, "r");
  if( file == NULL ){
    printf("File not found\n");
    return false;
  }
  while( 1 ){

    char lineHeader[128];
    int res = fscanf(file, "%s", lineHeader);
    if (res == EOF)
      break;
    if ( strcmp( lineHeader, "v" ) == 0 ){
      glm::vec3 vertex;
      fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
      temp_vertices.push_back(vertex);
    }else if ( strcmp( lineHeader, "vt" ) == 0 ){
      glm::vec2 uv;
      fscanf(file, "%f %f\n", &uv.x, &uv.y );
      temp_uvs.push_back(uv);
    }else if ( strcmp( lineHeader, "vn" ) == 0 ){
      glm::vec3 normal;
      fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
      temp_normals.push_back(normal);
    }else if ( strcmp( lineHeader, "f" ) == 0 ){
      std::string vertex1, vertex2, vertex3;
      unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
      int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
      if (matches != 9){
        printf("File can't be read by parser\n");
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

  for( unsigned int i=0; i<vertexIndices.size(); i++ ){

    unsigned int vertexIndex = vertexIndices[i];
    unsigned int uvIndex = uvIndices[i];
    unsigned int normalIndex = normalIndices[i];

    glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
    glm::vec2 uv = temp_uvs[ uvIndex-1 ];
    glm::vec3 normal = temp_normals[ normalIndex-1 ];

    out_vertices.push_back(vertex);
    out_uvs     .push_back(uv);
    out_normals .push_back(normal);
  }
  return true;
}

GLuint LoadShaders(const char *vertex_file_path,const char *fragment_file_path)
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
  printf("Compiling shader : %s\n", vertex_file_path);
  char const *VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    printf("%s\n", &VertexShaderErrorMessage[0]);
  }
  printf("Compiling shader : %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    printf("%s\n", &FragmentShaderErrorMessage[0]);
  }
  printf("Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    std::vector<char> ProgramErrorMessage(InfoLogLength+1);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    printf("%s\n", &ProgramErrorMessage[0]);
  }
  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);
  return ProgramID;
}
