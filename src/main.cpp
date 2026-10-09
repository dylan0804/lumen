#include "camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ostream>
#include <vector>
static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

float deltaTime = 0.0f;
float lastFrame = 0.0f;
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

float lastX, lastY;
bool firstMouse = true;
static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
  Camera *c = static_cast<Camera *>(glfwGetWindowUserPointer(window));
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  c->setYaw(c->getYaw() + xoffset * 0.1f);
  c->setPitch(c->getPitch() + yoffset * 0.1f);

  if (c->getPitch() > 89.f)
    c->setPitch(89.f);
  if (c->getPitch() < -89.f)
    c->setPitch(-89.f);

  glm::vec3 direction;
  direction.x =
      cos(glm::radians(c->getYaw())) * cos(glm::radians(c->getPitch()));
  direction.y = sin(glm::radians(c->getPitch()));
  direction.z =
      sin(glm::radians(c->getYaw())) * cos(glm::radians(c->getPitch()));
  c->setFront(glm::normalize(direction));
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    std::cout << "clicked" << std::endl;
}

void generate_cylinder(float radius, float height, int longSegments,
                       int latSegments, std::vector<Vertex> &verts,
                       std::vector<unsigned int> &indices) {
  float half_h = height / 2;
  int verts_per_ring = longSegments + 1;
  int vert_count = verts_per_ring * 2; // top n bottom

  glm::vec3 q = glm::vec3(0, half_h, 0);
  // top hemisphere
  for (int i = 0; i <= latSegments; i++) {
    float theta = (glm::pi<float>() / 2) * ((float)i / latSegments);
    float ringRadius = radius * sin(theta);

    for (int j = 0; j <= longSegments; j++) {
      float angle = 2.f * glm::pi<float>() * ((float)j / longSegments);
      float y = half_h + (radius * cos(theta));
      glm::vec3 pos =
          glm::vec3(ringRadius * cos(angle), y, ringRadius * sin(angle));
      glm::vec3 normal = glm::normalize(pos - q);
      verts.push_back({pos, normal});
    }
  }

  // top EBO
  for (int lat = 0; lat < latSegments; lat++) {
    int ringA = lat * (longSegments + 1);
    int ringB = (lat + 1) * (longSegments + 1);

    for (int j = 0; j < longSegments; j++) {
      int topA = ringA + j;
      int topB = ringA + j + 1;
      int botA = ringB + j;
      int botB = ringB + j + 1;

      indices.push_back(topA);
      indices.push_back(botA);
      indices.push_back(topB);
      indices.push_back(topB);
      indices.push_back(botA);
      indices.push_back(botB);
    }
  }

  int wall_start = verts.size();
  q = glm::vec3(0, half_h, 0);
  for (int i = 0; i <= longSegments; i++) {
    float angle = 2.f * glm::pi<float>() * ((float)i / longSegments);
    glm::vec3 pos = glm::vec3(radius * cos(angle), half_h, radius * sin(angle));
    glm::vec3 normal = glm::normalize(pos - q);
    verts.push_back({pos, normal});
  }

  q = glm::vec3(0, -half_h, 0);
  for (int i = 0; i <= longSegments; i++) {
    float angle = 2.f * glm::pi<float>() * ((float)i / longSegments);
    glm::vec3 pos =
        glm::vec3(radius * cos(angle), -half_h, radius * sin(angle));
    glm::vec3 normal = glm::normalize(pos - q);
    verts.push_back({pos, normal});
  }

  for (int i = 0; i <= longSegments; i++) {
    int topA = wall_start + i;
    int topB = wall_start + i + 1;
    int botA = verts_per_ring + wall_start + i;
    int botB = verts_per_ring + wall_start + i + 1;

    indices.push_back(topA);
    indices.push_back(botA);
    indices.push_back(topB);
    indices.push_back(topB);
    indices.push_back(botA);
    indices.push_back(botB);
  }

  q = glm::vec3(0, -half_h, 0);
  int bot_start = verts.size();
  // bottom hemisphere
  for (int i = 0; i <= latSegments; i++) {
    float theta = (glm::pi<float>() / 2) * ((float)i / latSegments);
    float ringRadius = radius * sin(theta);

    for (int j = 0; j <= longSegments; j++) {
      float angle = 2.f * glm::pi<float>() * ((float)j / longSegments);
      float y = -half_h - (radius * cos(theta));
      glm::vec3 pos(ringRadius * cos(angle), y, ringRadius * sin(angle));
      glm::vec3 normal = glm::normalize(pos - q);
      verts.push_back({pos, normal});
    }
  }

  for (int lat = 0; lat < latSegments; lat++) {
    int ringA = bot_start + (lat * verts_per_ring);
    int ringB = bot_start + ((lat + 1) * verts_per_ring);

    for (int j = 0; j < longSegments; j++) {
      int topA = ringA + j;
      int topB = ringA + j + 1;
      int botA = ringB + j;
      int botB = ringB + j + 1;

      indices.push_back(topA);
      indices.push_back(botA);
      indices.push_back(topB);
      indices.push_back(topB);
      indices.push_back(botA);
      indices.push_back(botB);
    }
  }
}

int main(void) {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glm::vec3 pos = glm::vec3(0.f, 5.f, 3.f);
  glm::vec3 front = glm::vec3(0.f, 0.f, -1.f);
  glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
  Camera c(pos, front, up, -90.f);
  c.setMoveSpeed(7.5f);

  glfwSetWindowUserPointer(window, &c);
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  // if (glfwRawMouseMotionSupported())
  // glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwSwapInterval(1);
  glEnable(GL_DEPTH_TEST);

  // Mesh grid;
  // {
  //   std::vector<Vertex> vertices;
  //   std::vector<unsigned int> indices;
  //   // generate_vertices(vertices, indices);
  //   grid = Mesh(vertices, indices);
  // }
  Mesh capsule;
  {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    generate_cylinder(0.4f, 1.0f, 32, 8, vertices, indices);
    Texture capsuleTex(230, 110, 50); // solid warm orange
    capsule = Mesh(vertices, indices, capsuleTex.getTextureID());
  }
  Mesh grid;
  {
    std::vector<Vertex> vertices;
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        float x = 0 + i + ((i > 0) ? 100 : 0);
        float y = 0.5;
        float z = 0 + j + ((j > 0) ? 100 : 0);
        glm::vec3 position(x, y, z);
        glm::vec3 q(x, 0, z);
        glm::vec3 normal = glm::normalize(position - q);
        glm::vec2 texture(x, z);
        vertices.push_back({position, normal, texture});
      }
    }
    std::vector<unsigned int> indices{0, 1, 3, 0, 2, 3};
    Texture t("mc.jpg");
    grid = Mesh(vertices, indices, t.getTextureID());
    std::cout << grid.getTextureID();
  }
  Mesh cube;
  {
    cube = Mesh(
        std::vector<Vertex>{
            {{-0.5f, -0.5f, -0.5f}},
            {{0.5f, -0.5f, -0.5f}},
            {{-0.5f, 0.5f, -0.5f}},
            {{0.5f, 0.5f, -0.5f}},
            {{-0.5f, -0.5f, 0.5f}},
            {{0.5f, -0.5f, 0.5f}},
            {{-0.5f, 0.5f, 0.5f}},
            {{0.5f, 0.5f, 0.5f}},
        },
        std::vector<unsigned int>{
            0, 3, 1, 0, 2, 3, 4, 5, 7, 7, 6, 4, 6, 2, 0, 0, 4, 6,
            7, 1, 3, 1, 7, 5, 0, 1, 5, 5, 4, 0, 2, 7, 3, 7, 2, 6,
        },
        0);
  }

  Shader shader("glsl/vert.glsl", "glsl/frag.glsl");
  shader.setUniformLocation(
      std::vector<std::string>{"lightColor", "model", "view", "projection",
                               "lightPos", "viewPos", "ourTexture"});
  Shader lightSource("glsl/vert.glsl", "glsl/cube_frag.glsl");
  lightSource.setUniformLocation(
      std::vector<std::string>{"model", "view", "projection"});

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  float lastFrame = 0.f;
  float deltaTime = 0.f;

  shader.use();
  shader.setInt("ourTexture", 0);
  std::cout << grid.getTextureID();

  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float ratio = width / (float)height;

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view =
        glm::lookAt(c.getPos(), c.getPos() + c.getFront(), c.getUp());
    glm::mat4 projection =
        glm::perspective(glm::radians(60.0f), ratio, 0.1f, 1000.0f);

    // shader.use();
    // // shader.setVec3("fillColor", 0.8f, 0.8f, 0.8f);
    // // shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    // shader.setMat4("model", model);
    // shader.setMat4("view", view);
    // shader.setMat4("projection", projection);
    //
    // grid.draw();

    float t = (float)glfwGetTime();

    // light orbits in a circle above the center of the floor, instead of
    // clipping through floor level
    glm::vec3 orbitCenter(50.f, 0.5f, 50.f);
    float orbitRadius = 20.f;
    float orbitHeight = 15.f;
    glm::vec3 cubePos =
        orbitCenter +
        glm::vec3(orbitRadius * cos(t), orbitHeight, orbitRadius * sin(t));

    lightSource.use();
    model = glm::translate(glm::mat4(1.0f), cubePos);
    lightSource.setMat4("model", model);
    lightSource.setMat4("view", view);
    lightSource.setMat4("projection", projection);
    cube.draw();

    // cycle the light color through a slow rainbow instead of static white
    glm::vec3 lightColor =
        glm::vec3(sin(t), sin(t + 2.094f), sin(t + 4.188f)) * 0.5f +
        glm::vec3(0.5f);

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("viewPos", c.getPos());
    shader.setVec3("lightPos", cubePos);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grid.getTextureID());
    model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    grid.draw();

    glBindTexture(GL_TEXTURE_2D, capsule.getTextureID());
    model = glm::translate(glm::mat4(1.0f), glm::vec3(50.f, 1.4f, 50.f));
    shader.setMat4("model", model);
    capsule.draw();

    c.processMovement(window, lastFrame, deltaTime);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
