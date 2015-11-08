#ifndef UTILS_H
#define UTILS_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <time.h>

// 3rdparty code

namespace utils
{
  float dt(clock_t first, clock_t second);

  glm::vec3 xyz(const glm::vec4& v);

  bool loadTexture(const std::string& texName, GLuint &id);

  size_t loadOBJ(const char *path,
    std::vector<float>& out_vertices,
    std::vector<float>& out_uvs,
    std::vector<float>& out_normals
    );

  bool loadShaders(const char *vertex_file_path, const char *fragment_file_path, GLuint& id);
}

#endif