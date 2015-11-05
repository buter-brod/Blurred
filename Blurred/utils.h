#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <time.h>

float dt(clock_t first, clock_t second);

bool loadTexture(const std::string& texName, GLuint &id);

bool loadOBJ(const char * path, 
             std::vector<float>& out_vertices, 
             std::vector<float>& out_uvs,
             std::vector<float>& out_normals
             );

bool loadShaders(const char *vertex_file_path,const char *fragment_file_path, GLuint& id);