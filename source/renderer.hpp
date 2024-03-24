#pragma once

//#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include <Windows.h>

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
inline VkInstance inst;
inline VkSurfaceKHR presentSurface;
inline VkDevice device;
inline VkSwapchainKHR swapchain;

VkResult init(const HWND, const HINSTANCE);


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