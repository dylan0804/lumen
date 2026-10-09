#pragma once

#include "glad/glad.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include <vector>

typedef struct {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texture;
} Vertex;
static_assert(sizeof(Vertex) == 32);

class Mesh {
public:
  Mesh() = default;
  Mesh(const std::vector<Vertex> &vertices,
       const std::vector<unsigned int> &indices,
       const unsigned int &texture_id);

  Mesh(const Mesh &other) = delete;
  Mesh &operator=(const Mesh &other) = delete;

  Mesh(Mesh &&other) noexcept
      : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
        indexCount(other.indexCount), textureId(other.textureId) {
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
  }
  Mesh &operator=(Mesh &&other) noexcept {
    if (this != &other) {
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &EBO);
      VAO = other.VAO;
      VBO = other.VBO;
      EBO = other.EBO;
      indexCount = other.indexCount;
      textureId = other.textureId;
      other.VAO = 0;
      other.VBO = 0;
      other.EBO = 0;
    }

    return *this;
  }
  ~Mesh();

  void draw() const;
  unsigned int getTextureID() const;

private:
  GLuint VAO = 0;
  GLuint VBO = 0;
  GLuint EBO = 0;
  GLsizei indexCount = 0;
  unsigned int textureId = 0;
};
