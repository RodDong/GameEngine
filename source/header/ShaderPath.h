#pragma once
#include <string>
#include <unordered_map>
#include "Shader.h"

enum class ShaderType { Vertex, Fragment, Geometry };

inline std::string GetShaderPath(const std::string& name, ShaderType type) {
    static const std::string base = "resources/shaders/";
    static const std::unordered_map<ShaderType, std::string> ext = {
        {ShaderType::Vertex, ".vs"},
        {ShaderType::Fragment, ".fs"},
        {ShaderType::Geometry, ".gs"}
    };
    return base + name + ext.at(type);
}

inline Shader CreateShader(const std::string& name, bool hasGeometry = false) {
    if (hasGeometry) {
        return Shader(
            GetShaderPath(name, ShaderType::Vertex).c_str(),
            GetShaderPath(name, ShaderType::Fragment).c_str(),
            GetShaderPath(name, ShaderType::Geometry).c_str()
        );
    }
    else {
        return Shader(
            GetShaderPath(name, ShaderType::Vertex).c_str(),
            GetShaderPath(name, ShaderType::Fragment).c_str()
        );
    }
}