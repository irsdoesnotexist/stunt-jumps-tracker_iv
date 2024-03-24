#include "renderer.hpp"
#include "app.hpp"
#include "vulkan/vulkan_core.h"

#define VK_USE_PLATFORM_WIN32_KHR
//#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan_win32.h>
#include <vulkan/vulkan.h>

#include <stdint.h>
#include <vector>
#include <array>
#include <string>
#include <memory>
#include <algorithm>
#include <regex>

VkResult sjt4::rndr::init(const HWND in_mainWnd, const HINSTANCE in_hInst) {
    VkResult res{VK_SUCCESS};

    // Create VkInstance
#ifdef DEBUG
    std::vector <const char* const> instanceExts {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};  // App won't work without these
    std::vector <const char*> validationLayers {"VK_LAYER_LUNARG_api_dump\00", "VK_LAYER_KHRONOS_validation\00", "VK_LAYER_KHRONOS_profiles\00"}; // Ones that are not present will be erased
{
    uint32_t numberOfLayers{};
    std::vector<VkLayerProperties> layers;
    do {
        vkEnumerateInstanceLayerProperties(&numberOfLayers, nullptr);
        layers.resize(numberOfLayers);
        res = vkEnumerateInstanceLayerProperties(&numberOfLayers, layers.data());
    } while (res==VK_INCOMPLETE);
    for (auto itr{validationLayers.begin()}; itr<validationLayers.end(); ++itr) {
        if(!containsLyr(layers.begin(), layers.end(), *itr)) {
            validationLayers.erase(itr);
        }
    }

    constexpr std::array<const char* const, 1> lyrExtensions {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};   // Not requiered debugging extensions
    uint32_t numberOfExtensions{};
    std::vector<VkExtensionProperties> extensions;
    do {
        vkEnumerateInstanceExtensionProperties(nullptr, &numberOfExtensions, nullptr);
        extensions.resize(numberOfExtensions);
        vkEnumerateInstanceExtensionProperties(nullptr, &numberOfExtensions, extensions.data());
    } while (res==VK_INCOMPLETE);
    std::vector<VkExtensionProperties> layerExts {};
    uint32_t layerExtsCnt{};
    for (auto layer : validationLayers) {
        LOG("%s\n" COMMA layer);
        do {
            vkEnumerateInstanceExtensionProperties(layer,&layerExtsCnt ,nullptr);
            layerExts.resize(layerExtsCnt);
            res = vkEnumerateInstanceExtensionProperties(layer, &layerExtsCnt, layerExts.data());
        } while (res==VK_INCOMPLETE);
        for (auto ext : layerExts) {
            //LOG("*- %s\n" COMMA ext.extensionName);
            if(sjt4::rndr::containsExt(extensions.begin(), extensions.end(), ext.extensionName) && sjt4::rndr::containsExt<lyrExtensions.size()>(lyrExtensions.begin(), lyrExtensions.end(), ext.extensionName)) {
                char* cpytemp {new char[std::strlen(ext.extensionName)+1] {}};
                std::memcpy(cpytemp, ext.extensionName, std::strlen(ext.extensionName)+1);
                instanceExts.push_back(cpytemp);    // Note: After instanceExts goes out of scope, this becomes a memory leak. But... Then every c-style string is a memory leak as well?
                LOG("-> %s\n" COMMA ext.extensionName);
            }
        }
    }
}    
#else
    const std::vector<const char*> validationLayers {};
    const std::vector<const char*> instanceExts {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
#endif
{
    static constexpr VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,                                   // TODO: add VkValidationCheckEXT* here if EXT is availiable
        .pApplicationName = sjt4::appName,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "none",
        .engineVersion = NULL,
        .apiVersion = VK_API_VERSION_1_3
    };
    const VkInstanceCreateInfo instInfo {
        .sType= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext= nullptr,
        .flags= NULL,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instanceExts.size()),
        .ppEnabledExtensionNames = instanceExts.data()
    };
    if((res = vkCreateInstance(&instInfo, nullptr, &sjt4::rndr::inst)) != VK_SUCCESS) {
        LOG("Failed to create rndr::VkInstance. Code: %d\n" COMMA res);
        return res;
    }
}

#ifdef DEBUG
{
    // Create debug utils callback function
    VkDebugUtilsMessengerCreateInfoEXT debugMsgInfo {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = NULL,
        .messageSeverity = 0x1111,
        .messageType = 0b1111,
        .pfnUserCallback = sjt4::rndr::debugCallback,
        .pUserData = nullptr
    };
    PFN_vkCreateDebugUtilsMessengerEXT p_vkCreateDebugUtilsMessengerEXT {reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(sjt4::rndr::inst, "vkCreateDebugUtilsMessengerEXT"))};
    ::p_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(sjt4::rndr::inst, "vkDestroyDebugUtilsMessengerEXT"));
    if(p_vkCreateDebugUtilsMessengerEXT == nullptr) {
        LOG("Failed to find vkCreateDebugUtilsMessengerEXT\n");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    else if (p_vkDestroyDebugUtilsMessengerEXT == nullptr) {
        LOG("Failed to find vkDestroyDebugUtilsMessegnerEXT\n");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    p_vkCreateDebugUtilsMessengerEXT(sjt4::rndr::inst, &debugMsgInfo, nullptr, &sjt4::rndr::debugMessenger);
}
#endif

{
    // Create presentation surface
    VkWin32SurfaceCreateInfoKHR surfaceInfo {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = NULL,
        .hinstance = in_hInst,
        .hwnd = in_mainWnd
    };
    if((res = vkCreateWin32SurfaceKHR(sjt4::rndr::inst, &surfaceInfo, nullptr, &sjt4::rndr::presentSurface)) != VK_SUCCESS) {
        LOG("Failed to create rndr::presentSurface. Code: %d\n" COMMA res);
        return res;
    }
}

{                                                     // Note: Add the scope once I know where it ends
    // Find suitable VkPhysicalDevice
    VkPhysicalDevice physicalDevice {nullptr};
    uint32_t presentQIndx{~0u};
    constexpr const std::array<const char* const, 1>deviceExtentions {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; 
     uint32_t devicesCnt{};
     vkEnumeratePhysicalDevices(sjt4::rndr::inst, &devicesCnt, nullptr);
     std::unique_ptr<VkPhysicalDevice[]> devices{ new VkPhysicalDevice[devicesCnt] };
     vkEnumeratePhysicalDevices(sjt4::rndr::inst, &devicesCnt, devices.get());
    sjt4::rndr::pickPhysicalDevice<deviceExtentions.size()>(devices.get(), devicesCnt, deviceExtentions, physicalDevice, presentQIndx);
    if(physicalDevice== nullptr || presentQIndx== ~0u) {
        LOG("Failed to find suitable physical device. Devices in the system: %d\n" COMMA devicesCnt);
        return VK_ERROR_UNKNOWN;
    }
   
    // Create VkDevice
    auto priorities{1.0f};
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    const VkDeviceQueueCreateInfo queuesInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = NULL,
        .queueFamilyIndex = (uint32_t)presentQIndx,
        .queueCount = 1,
        .pQueuePriorities = &priorities
    };
    const VkDeviceCreateInfo deviceInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = NULL,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queuesInfo,
        .enabledLayerCount = NULL,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = deviceExtentions.size(),
        .ppEnabledExtensionNames = deviceExtentions.data(),
        .pEnabledFeatures = &deviceFeatures
    };
    if((res = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &sjt4::rndr::device)) != VK_SUCCESS) {
        LOG("Failed to create rndr::device. Code: %d" COMMA res);
        return res;
    }


    VkFormat swapchainImagesFormat {};
    VkExtent2D swapchainImageResolution;
    VkColorSpaceKHR swapchainColorSpace {};
    VkPresentModeKHR swapchainPresentMode{};
    //
     uint32_t imageFormatsCnt {};
     std::vector<VkSurfaceFormatKHR> formats;
    do {
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, sjt4::rndr::presentSurface, &imageFormatsCnt, nullptr);
        formats.resize(imageFormatsCnt);
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, sjt4::rndr::presentSurface, &imageFormatsCnt, formats.data());
    } while (res == VK_INCOMPLETE);
    for (auto format : formats) {
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchainColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, swapchainImagesFormat = VK_FORMAT_B8G8R8A8_SRGB;
            break;
    }}
    if(!swapchainColorSpace && !swapchainImagesFormat)
        swapchainImagesFormat = formats[0].format, swapchainColorSpace = formats[0].colorSpace;
    //
     uint32_t presentModesCnt{};
     std::vector<VkPresentModeKHR> presentModes;
    do {
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, sjt4::rndr::presentSurface, &presentModesCnt, nullptr);
        presentModes.resize(presentModesCnt);
        res = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, sjt4::rndr::presentSurface, &presentModesCnt, presentModes.data());
    } while (res == VK_INCOMPLETE);
    for (auto mode : presentModes) {
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
    }}
    if(!swapchainPresentMode)
        swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    //
    VkSurfaceCapabilitiesKHR surfaceProperties{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, sjt4::rndr::presentSurface, &surfaceProperties);
    if(!~surfaceProperties.currentExtent.height) {
        RECT wndResolution;
        GetClientRect(in_mainWnd, &wndResolution);
        swapchainImageResolution.height = (uint32_t)wndResolution.bottom, swapchainImageResolution.width = (uint32_t)wndResolution.right;
    }
    else {
        swapchainImageResolution = surfaceProperties.currentExtent;
    }

    VkSwapchainCreateInfoKHR swapchainInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = NULL,
        .surface = sjt4::rndr::presentSurface,
        .minImageCount = surfaceProperties.minImageCount<surfaceProperties.maxImageCount || !surfaceProperties.maxImageCount ? surfaceProperties.minImageCount+1 : surfaceProperties.minImageCount,
        .imageFormat = swapchainImagesFormat,
        .imageColorSpace = swapchainColorSpace,
        .imageExtent = swapchainImageResolution,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &presentQIndx,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapchainPresentMode,
        .clipped = VK_FALSE,
        .oldSwapchain = nullptr
    };
    if((res = vkCreateSwapchainKHR(sjt4::rndr::device, &swapchainInfo, nullptr, &sjt4::rndr::swapchain)) != VK_SUCCESS) {
        LOG("Failed to create rndr::swapchain. Code: %d\n" COMMA res);
        return res;
    }
}

    return VK_SUCCESS; 
}


[[gnu::pure]] [[gnu::always_inline]] bool sjt4::rndr::containsLyr(const ::std::vector<VkLayerProperties>::iterator& begin, const std::vector<VkLayerProperties>::iterator& end, const char* const lyr) {
    for (auto itr{begin}; itr<end; ++itr)
        if(!std::strcmp(itr->layerName, lyr))
            return true;
    return false;
}
[[gnu::pure]] [[gnu::always_inline]] bool sjt4::rndr::containsExt(const ::std::vector<VkExtensionProperties>::iterator& begin, const std::vector<VkExtensionProperties>::iterator& end, const char* const ext) {
    for (auto itr{begin}; itr<end; ++itr)
        if(!std::strcmp(itr->extensionName, ext))
            return true;
    return false;
}
template<int N>
[[gnu::pure]] [[gnu::always_inline]] bool sjt4::rndr::containsExt(const typename ::std::array<const char* const, N>::iterator& begin, const typename std::array<const char* const, N>::iterator& end, const char* const ext) {
    for (auto itr{begin}; itr<end; ++itr)
        if(!std::strcmp(*itr, ext))
            return true;
    return false;
}

uint32_t __stdcall sjt4::rndr::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void*) {
    LOG("Severity: %d;" COMMA severity); LOG("Type: %d;\n" COMMA type);
    LOG("[%s]:\n %s\n\n" COMMA data->pMessageIdName COMMA data->pMessage);
    LOG("Buffers:\n");
    for(uint32_t i{}; i<data->cmdBufLabelCount; ++i) {
        LOG(" * %s\n" COMMA data->pCmdBufLabels[i].pLabelName);
    }
    LOG("Queues:\n");
    for(uint32_t i{}; i<data->queueLabelCount; ++i)
        LOG(" * %s\n" COMMA data->pQueueLabels[i].pLabelName);
    LOG("Objects:\n");
    for(uint32_t i{}; i<data->objectCount; ++i)
        LOG(" * [%d]-> %s\n" COMMA data->pObjects[i].objectType COMMA data->pObjects[i].pObjectName);

    return VK_FALSE;
}

template <int N>
void sjt4::rndr::pickPhysicalDevice(const VkPhysicalDevice *const in_list, const uint32_t in_length, const typename std::array<const char* const, N>& reqExtensions, VkPhysicalDevice &out_physicalDevice, uint32_t &out_qFamilyIndx) noexcept {
    VkPhysicalDeviceProperties selProperties{}, itProperties;
    VkPhysicalDeviceMemoryProperties selMemProperties{}, itMemProperties;
    uint32_t selLocalHeapIndex{};
    out_physicalDevice = nullptr;
    for(size_t i{}; i<in_length; ++i) {
        vkGetPhysicalDeviceProperties(in_list[i], &itProperties);
        if(!out_physicalDevice) {
            if(sjt4::rndr::physicalDeviceIsSuitable<N>(in_list[i], reqExtensions, out_qFamilyIndx))
                goto assign;
        }
        else if (itProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && selProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            goto assign;
        }
        else if (itProperties.deviceType == selProperties.deviceType) {
            vkGetPhysicalDeviceMemoryProperties(in_list[i], &selMemProperties);
            for(uint32_t j{}; j<selMemProperties.memoryHeapCount; ++j) {
                if (selMemProperties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                    selLocalHeapIndex = j;
            }
            for(uint32_t j{}; j<itMemProperties.memoryHeapCount; ++j) {  // selMemProperties is guaranteed to have a valid memoryHeaps member, no need to check for that
                if (itMemProperties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                    if(itMemProperties.memoryHeaps[j].size > selMemProperties.memoryHeaps[selLocalHeapIndex].size)
                        if(physicalDeviceIsSuitable<N>(in_list[j], reqExtensions, out_qFamilyIndx)){
                            goto assign;
                        }
            }
        }
        continue;
    assign:
        vkGetPhysicalDeviceMemoryProperties(in_list[i], &selMemProperties);
        selProperties = itProperties;
        out_physicalDevice = in_list[i];
        // out_qFamilyIndx is assigned by physicalDeviceIsSuitable(...)
    }

    return;
}
template <int N>
[[gnu::always_inline]] [[gnu::const]] bool sjt4::rndr::physicalDeviceIsSuitable(const VkPhysicalDevice in_physicalDevice, const typename std::array<const char* const, N> in_reqExtensions, uint32_t &out_qFamilyIndx) noexcept {
    VkResult res;
    
    // Features support
    VkPhysicalDeviceFeatures itFeatures;
    vkGetPhysicalDeviceFeatures(in_physicalDevice, &itFeatures);
    if(!itFeatures.geometryShader){
        return false;
    }
    
    // Extentions support
    uint32_t extCnt{};
    std::vector<VkExtensionProperties> deviceExtentions;
    do {
        vkEnumerateDeviceExtensionProperties(in_physicalDevice, nullptr, &extCnt, nullptr);
        deviceExtentions.resize(extCnt);
        res = vkEnumerateDeviceExtensionProperties(in_physicalDevice, nullptr, &extCnt, deviceExtentions.data());
    } while (res == VK_INCOMPLETE);
    bool extInTheList;
    for (auto reqExt : in_reqExtensions) {
        extInTheList = false;
        for(auto ext : deviceExtentions) {
            if(!std::strcmp(ext.extensionName, reqExt))
                extInTheList = true;
        }
        if(!extInTheList)
            return false;
    }

    uint32_t presentModeCnt{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(in_physicalDevice, sjt4::rndr::presentSurface, &presentModeCnt, nullptr);
    if(!presentModeCnt) {
        return false;
    }

    uint32_t formatsCnt{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(in_physicalDevice, sjt4::rndr::presentSurface, &formatsCnt, nullptr);
    if(!formatsCnt) {
        return false;
    }

    // Presentation and rendering support
    bool hasTheQueue{false};
    uint32_t qFamiliesCnt{};
    std::vector<VkQueueFamilyProperties> itQFamilies;
    do {
        vkGetPhysicalDeviceQueueFamilyProperties(in_physicalDevice, &qFamiliesCnt, nullptr);
        itQFamilies.resize(qFamiliesCnt);
        vkGetPhysicalDeviceQueueFamilyProperties(in_physicalDevice,&qFamiliesCnt, itQFamilies.data());
        res = VK_SUCCESS;
    } while (res == VK_INCOMPLETE);
    uint32_t qCanPresent{false};
    for (auto itr{itQFamilies.begin()}; itr<itQFamilies.end(); ++itr) {
        if(!(itr->queueFlags & VK_QUEUE_GRAPHICS_BIT))
            continue;
        vkGetPhysicalDeviceSurfaceSupportKHR(in_physicalDevice, (uint32_t)(itr-itQFamilies.begin()), sjt4::rndr::presentSurface, &qCanPresent);
        if(!qCanPresent)
            continue;
        out_qFamilyIndx = (uint32_t)(itr-itQFamilies.begin());   // !Note: only assigns out_qFamilyIndx if device is suitable
        hasTheQueue = true; 
        break;
    }
    if(!hasTheQueue)
        return false;

    return true;
}