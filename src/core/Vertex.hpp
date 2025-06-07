#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <string>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <glm/gtx/hash.hpp>


const std::string MODEL_PATH = "model/viking.obj";
const std::string TEXTURE_PATH = "textures/viking.png";



void loadModel();

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;
    
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator==(const Vertex& other) const {
       return pos == other.pos && 
              color == other.color &&
              texCoord == other.texCoord &&
              normal == other.normal;
   }
};


namespace std {
   template<> struct hash<Vertex> {
       size_t operator()(const Vertex& vertex) const {
           return  (hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1) ^
                   (hash<glm::vec3>()(vertex.normal) << 1);
       }
   };
}

extern std::vector<Vertex> vertices; 
extern std::vector<uint32_t> indices;


