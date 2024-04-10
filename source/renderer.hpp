#pragma once

//#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include <Windows.h>

#include <glm/glm.hpp>
#include <stddef.h>
#include <vector>

#ifdef DEBUG
    #include <cstdio>
    #define LOG(A) std::printf(A)
    
    inline PFN_vkDestroyDebugUtilsMessengerEXT p_vkDestroyDebugUtilsMessengerEXT;
#else
    #define LOG(A)
#endif
#define COMMA ,

namespace sjt4{
namespace rndr {
#ifdef DEBUG
uint32_t __stdcall debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);

inline VkDebugUtilsMessengerEXT debugMessenger;
#endif
static constexpr uint8_t sceneMainIndex {0};

class DrawableObject_t {
private:
    VkDeviceMemory vboMem;
    VkDeviceMemory uboMem;
    VkBuffer vertexBuf;
    VkBuffer uniformBuf;
    //VkPipeline graphicsPipeline;
public:
    void readData(const char*&);   // Read data from .res and put it into buffers, store them

    ~DrawableObject_t();
};


inline VkInstance inst;
inline VkSurfaceKHR presentSurface;
inline VkPhysicalDevice physicalDevice;
inline VkDevice device;
inline VkSwapchainKHR swapchain;
inline VkCommandPool commandPool;
inline std::vector<VkCommandBuffer> cmdBuffers;
inline std::vector<VkImageView> swapchainImagesData;
inline std::vector<VkFramebuffer> frameBuffers;
inline std::vector<DrawableObject_t> sceneObjects;

VkResult init(const HWND, const HINSTANCE);
bool readSceneData(const char*, uint8_t);
// FetchSceneInfo


template<int N>
void pickPhysicalDevice(const VkPhysicalDevice *const in_list, const uint32_t in_length, const typename std::array<const char* const, N>& reqExtensions, VkPhysicalDevice &out_physicalDevice, uint32_t &out_qFamilyIndx) noexcept;
template<int N>
[[gnu::always_inline]] [[gnu::const]] bool physicalDeviceIsSuitable(const VkPhysicalDevice in_physicalDevice, const typename std::array<const char* const, N> in_reqExtensions, uint32_t &out_qFamilyIndx) noexcept;
[[gnu::always_inline]] bool containsLyr(const std::vector<VkLayerProperties>::iterator&, const std::vector<VkLayerProperties>::iterator&, const char* const);
[[gnu::always_inline]] bool containsExt(const std::vector<VkExtensionProperties>::iterator&, const std::vector<VkExtensionProperties>::iterator&, const char* const);
template<int N>
[[gnu::always_inline]] bool containsExt(const typename std::array<const char* const, N>::iterator&, const typename std::array<const char* const, N>::iterator&, const char* const);

}
}

namespace sjt4 {namespace rndr{
namespace obj{  // This whole fucking namespace is completely temporary, sceneData-like structure (with a vague type) will be stored in .res section and read at runtime
    
    struct vertexData {
        glm::vec3 pos;
        glm::vec3 col;
    } __attribute__((packed));
    struct objectsData {
        glm::mat3 transform;    // Scale + rotate + move(homogeneous)
        uint32_t vertexCnt{3};
        vertexData vertex[3];
    } __attribute__((packed));
    struct sceneTemplateData {
        std::size_t size{sizeof(uint16_t) + sizeof(objectsData)*2};
        uint16_t objectsCount {2};   // Shouldn't ever be more than 65k
        objectsData objects[2] {
            {
                {glm::vec3{0, -0.5f, 0}, glm::vec3{-0.5f, 0, 0}, glm::vec3{0.5, 0, 1}},
                3,
                {
                {{-0.6f, -0.4f, 1.0f}, {0.7, 0.5, 0.0}},
                {{0.6f, -0.4f, 1.0f}, {0.0f, 0.5f, 0.5f}},
                {{0.0f, 0.6f, 1.0f}, {1.0f, 1.0f, 1.0f}}
                }
            },
            {
                {glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{-0.5f, 0.0f, 1.0f}},
                3,
                {
                {{-0.6f, -0.4f, 1.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.6f, -0.4f, 1.0f}, {0.0f, 1.0f, 0.0f}},
                {{0.0f, 0.6f, 1.0f}, {0.0f, 0.0f, 1.0f}}
                }
            }
        };
    } __attribute__((packed));
    struct scenesData{
        uint8_t numberOfTemplates {1};
        sceneTemplateData templates[1];
    } __attribute__((packed));

    constexpr const inline scenesData data {
        .numberOfTemplates= 1,
        .templates = {{.size = sizeof(uint16_t) + sizeof(objectsData)*2, .objectsCount = 2, .objects{{{glm::vec3{0, -0.5f, 0}, glm::vec3{-0.5f, 0, 0}, glm::vec3{0.5, 0, 1}},
                3,
                {
                {{-0.6f,-0.4f, 1.0f}, {0.7, 0.5, 0.0}},
                {{0.6f, -0.4f, 1.0f}, {0.0f, 0.5f, 0.5f}},
                {{0.0f, 0.6f, 1.0f}, {1.0f, 1.0f, 1.0f}}
                }}, {{glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{-0.5f, 0.0f, 1.0f}},
                3,
                {
                {{-0.6f, -0.4f, 1.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.6f, -0.4f, 1.0f}, {0.0f, 1.0f, 0.0f}},
                {{0.0f, 0.6f, 1.0f}, {0.0f, 0.0f, 1.0f}}
        }}}}
    }};
}
}}