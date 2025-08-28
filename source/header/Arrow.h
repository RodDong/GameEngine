#ifndef ARROW_H
#define ARROW_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

#include "Mesh.h"
class Shader;

class Arrow {
public:
    Mesh shaft;
    Mesh head;

    Arrow(float shaftLength = 0.7f, float shaftRadius = 0.02f,
        float headLength = 0.3f, float headRadius = 0.06f,
        int segments = 20);

    void Draw(const glm::vec3& dir, Shader& shader, const glm::vec3& pos = glm::vec3(0.0f), float scale = 1.0f, const glm::vec3& color = glm::vec3(0.0f));

private:
    Mesh generateShaft(float length, float radius, int segments);
    Mesh generateHead(float baseZ, float height, float radius, int segments);
};

#endif