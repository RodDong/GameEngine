#include "../header/Arrow.h"
#include "../header/Shader.h"

Arrow::Arrow(float shaftLength, float shaftRadius,
    float headLength, float headRadius,
    int segments)
    : shaft(generateShaft(shaftLength, shaftRadius, segments)),
    head(generateHead(shaftLength, headLength, headRadius, segments)) {}


Mesh Arrow::generateShaft(float length, float radius, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * glm::pi<float>() * i / segments;
        float x = cos(theta), y = sin(theta);

        glm::vec3 normal(x, y, 0.0f);
        glm::vec3 p1(x * radius, y * radius, 0.0f);
        glm::vec3 p2(x * radius, y * radius, length);

        Vertex v1 = { p1, normal, {}, {}, {} };
        Vertex v2 = { p2, normal, {}, {}, {} };

        vertices.push_back(v1);
        vertices.push_back(v2);
    }

    for (unsigned int i = 0; i < segments * 2; i += 2) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);

        indices.push_back(i + 1);
        indices.push_back(i + 3);
        indices.push_back(i + 2);
    }

    return Mesh(vertices, indices, {});
}

Mesh Arrow::generateHead(float baseZ, float height, float radius, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 tip(0, 0, baseZ + height);
    unsigned int baseIndex = 0;

    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * glm::pi<float>() * i / segments;
        float x = cos(theta), y = sin(theta);
        glm::vec3 base(x * radius, y * radius, baseZ);
        glm::vec3 normal = glm::normalize(glm::cross(glm::cross(tip - base, glm::vec3(0, 1, 0)), tip - base));

        Vertex vBase = { base, normal, {}, {}, {} };
        Vertex vTip = { tip, normal, {}, {}, {} };

        vertices.push_back(vBase);
        vertices.push_back(vTip);
    }

    for (unsigned int i = 0; i < segments * 2; i += 2) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    return Mesh(vertices, indices, {});
}

void Arrow::Draw(const glm::vec3& dir, Shader& shader, const glm::vec3& pos, float scale, const glm::vec3& color) {
    glm::vec3 defaultDir(0.0f, 0.0f, -1.0f);
    glm::vec3 targetDir = glm::normalize(dir);
    glm::vec3 axis = glm::cross(defaultDir, targetDir);
    float dot = glm::dot(defaultDir, targetDir);
    float angle = acos(glm::clamp(dot, -1.0f, 1.0f));

    glm::mat4 model(1.0f);
    model = glm::translate(model, pos);
    if (glm::length(axis) < 1e-4f) {
        if (dot < 0.999f) model = glm::rotate(model, glm::pi<float>(), glm::vec3(0, 1, 0));
    }
    else {
        model = glm::rotate(model, angle, glm::normalize(axis));
    }
    model = glm::scale(model, glm::vec3(scale));

    shader.use();
    shader.setMat4("model", model);
    shader.setVec3("color", color);
    glDisable(GL_CULL_FACE);
    shaft.Draw(shader);
    head.Draw(shader);
    glEnable(GL_CULL_FACE);
}