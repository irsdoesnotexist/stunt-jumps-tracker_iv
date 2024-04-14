#include "renderer.hpp"
#include "app.hpp"
#include "resources/resourcesInclude.h"
#include <wingdi.h>

#define VK_USE_PLATFORM_WIN32_KHR
//#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan_win32.h>
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include <errhandlingapi.h>
#include <winnt.h>

#include <stdint.h>
#include <vector>
#include <array>
#include <string>
#include <memory>
#include <algorithm>
#include <regex>
#include <optional>

VkResult sjt4::rndr::init(const HWND in_mainWnd, const HINSTANCE in_hInst) {
    VkResult res{VK_SUCCESS};

    // Create VkInstance
#ifdef DEBUG
    std::vector <const char*> instanceExts {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};  // App won't work without these
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
    //LOG("%p\n\n" COMMA sjt4::rndr::physicalDevice);
    vkGetPhysicalDeviceFeatures(sjt4::rndr::physicalDevice, &deviceFeatures);
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
    if((res = vkCreateDevice(sjt4::rndr::physicalDevice, &deviceInfo, nullptr, &sjt4::rndr::device)) != VK_SUCCESS) {
        LOG("Failed to create rndr::device. Code: %d" COMMA res);
        return res;
    }

    // Create swapchain
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

    // Make image views for the sapchain images
     uint32_t imagesCnt;
     std::vector<VkImage> swapchainImages;
    do {
        vkGetSwapchainImagesKHR(sjt4::rndr::device, sjt4::rndr::swapchain, &imagesCnt, nullptr);
        swapchainImages.resize(imagesCnt);
        res = vkGetSwapchainImagesKHR(sjt4::rndr::device, sjt4::rndr::swapchain, &imagesCnt, swapchainImages.data());
    } while (res == VK_INCOMPLETE);
    VkImageViewCreateInfo viewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = NULL,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchainImagesFormat,
        .components {.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
    };
    for (auto image : swapchainImages) {
        viewInfo.image = image;
        VkImageView view;
        res = vkCreateImageView(sjt4::rndr::device, &viewInfo, nullptr, &view);
        if(res != VK_SUCCESS) {
            LOG("Failed to create image view for swapchain images. Number of images: %d. Code: %d" COMMA imagesCnt COMMA res);
            return res;
        }
        sjt4::rndr::swapchainImagesData.push_back(view);
    }

    // Allocate command buffers
    const VkCommandPoolCreateInfo cmdPoolInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = NULL,
        .queueFamilyIndex = presentQIndx 
    };
    res = vkCreateCommandPool(sjt4::rndr::device, &cmdPoolInfo, nullptr, &sjt4::rndr::commandPool);
    if(res != VK_SUCCESS) {
        LOG("Failed to create a command pool. Code: %d" COMMA res);
        return res;
    }
    sjt4::rndr::cmdBuffers.resize(imagesCnt);
    VkCommandBufferAllocateInfo bufferAllocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = sjt4::rndr::commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = imagesCnt
    };
    res = vkAllocateCommandBuffers(sjt4::rndr::device, &bufferAllocInfo, sjt4::rndr::cmdBuffers.data());
    if(res != VK_SUCCESS) {
        LOG("Failed to allocate command buffers. Number of buffers: %d. Code: %d\n" COMMA imagesCnt COMMA res);
        return res;
    }

    sjt4::rndr::readSceneData(reinterpret_cast<const char*>(&sjt4::rndr::obj::data), 0);

    /*auto vertShader = FindResourceA(nullptr, MAKEINTRESOURCE(IDI_VERTEX_SHADER), RT_RCDATA);
    auto fragShader = FindResourceA(in_hInst, MAKEINTRESOURCE(IDI_FRAGMENT_SHADER), RT_RCDATA);
    if (!vertShader || !fragShader) {
        LOG("Failed to find resources. Error code: %lu. Shaders: %p, %p\n" COMMA GetLastError() COMMA vertShader COMMA fragShader);
        return VK_ERROR_INVALID_EXTERNAL_HANDLE;
    }
    auto pVertShader = LoadResource(nullptr, vertShader);
    auto pFragShader = LoadResource(nullptr, fragShader);
    if(!pVertShader || !pFragShader) {
        LOG("Failed to load resource. Error code: %lu. Shaders: %p, %p\n" COMMA GetLastError() COMMA pVertShader COMMA pFragShader);
    }*/

    if(!vertSpv || !fragSpv) {
        LOG("Failed to find shader data. %p %p\n" COMMA vertSpv COMMA fragSpv);
    }
    if(!vertSpv_size || !fragSpv_size) {
        LOG("Bad shader code size. Vert:%d, Frag:%d" COMMA vertSpv_size COMMA fragSpv_size);
    }
    VkShaderModuleCreateInfo vertShCI {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = NULL,
        .codeSize = vertSpv_size,//SizeofResource(nullptr, vertShader),
        .pCode = vertSpv//reinterpret_cast<const uint32_t*>(LockResource(pVertShader))
    };
    VkShaderModuleCreateInfo fragShCI {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = NULL,
        .codeSize = fragSpv_size,//SizeofResource(nullptr, fragShader),
        .pCode = fragSpv//reinterpret_cast<const uint32_t*>(LockResource(pFragShader))
    };
    

    /*
    VkCommandBufferBeginInfo bufBI {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = NULL,
        .pInheritanceInfo = nullptr
    };
    VkRenderPassBeginInfo rndrPassBI {};
    VkRenderingInfo bufRI {
        .
    };
    for(auto buffer : sjt4::rndr::cmdBuffers) {
        vkBeginCommandBuffer(buffer, &bufBI);
        vkCmdBeginRenderPass(buffer, );
        vkCmdBeginRendering(buffer, );
        vkCmdEndRendering();
        vkCmdEndRenderPass();
        vkEndCommandBuffer();
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, );
    }*/

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
            for(uint32_t j{}; j<itMemProperties.memoryHeapCount; ++j) {
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
        out_qFamilyIndx = (uint32_t)(itr-itQFamilies.begin());
        hasTheQueue = true; 
        break;
    }
    if(!hasTheQueue)
        return false;

    return true;
}

/*#include <iostream>
std::ostream &operator<<(std::ostream& out, const glm::vec3* vec) {
    std::printf("<%f, %f, %f>", vec->x, vec->y, vec->z);
    return out;
}
std::ostream& operator<<(std::ostream& out,const glm::mat3* mat) {
    out << (const glm::vec3*)mat << ' ' << (const glm::vec3*)((uint64_t)mat+12) << ' ' << (const glm::vec3*)((uint64_t)mat+24) << '\n'; // Fucking pointer arithmetic
    return out;
}*/
void sjt4::rndr::DrawableObject_t::readData(const char*& data) {
    static constexpr VkBufferCreateInfo uniBufCI {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = NULL,
        .size = sizeof(const glm::mat3),
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = NULL,
        .pQueueFamilyIndices = nullptr
    };
    vkCreateBuffer(sjt4::rndr::device, &uniBufCI, nullptr, &this->uniformBuf);
    
    VkMemoryRequirements bufMemReq;
    vkGetBufferMemoryRequirements(sjt4::rndr::device, this->uniformBuf, &bufMemReq);
    
    VkPhysicalDeviceMemoryProperties memProp;
    vkGetPhysicalDeviceMemoryProperties(sjt4::rndr::physicalDevice, &memProp);
    std::optional<uint32_t> memIndex{};
    for(uint32_t i{}; i<memProp.memoryTypeCount; ++i) {
        const auto& memType{memProp.memoryTypes[i]};
        if((memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && (bufMemReq.memoryTypeBits & (1 << i))) {
            memIndex.emplace(i);
        }
    }
    if(!memIndex.has_value()) {
        LOG("Failed to find appropriate memory type.\n");
        return;
    }
    const VkMemoryAllocateInfo uniformBufAI {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = bufMemReq.size,
        .memoryTypeIndex = memIndex.value()
    };
    vkAllocateMemory(sjt4::rndr::device, &uniformBufAI, nullptr, &this->uboMem);
    vkBindBufferMemory(sjt4::rndr::device, this->uniformBuf, this->uboMem, NULL);
    void* mappedMem{};
    vkMapMemory(sjt4::rndr::device, this->uboMem, NULL, uniBufCI.size, NULL, &mappedMem);
    std::memcpy(mappedMem, data, sizeof(glm::mat3));
    ///**/std::cout << reinterpret_cast<const glm::mat3*>(data) << '\n';
    data+= sizeof(glm::mat3);
    vkUnmapMemory(sjt4::rndr::device, this->uboMem);

    VkBufferCreateInfo vertBufCI {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = NULL,
        .size = ((*reinterpret_cast<const uint32_t*>(data))*sizeof(glm::vec3)*2),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    data+= sizeof(const uint32_t);
    vkCreateBuffer(sjt4::rndr::device, &vertBufCI, nullptr, &this->vertexBuf);

    vkGetBufferMemoryRequirements(sjt4::rndr::device, this->vertexBuf, &bufMemReq);
    memIndex.reset();
    for(uint32_t i{}; i<memProp.memoryTypeCount; ++i) {
        const auto& memType{memProp.memoryTypes[i]};
        if((memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && (bufMemReq.memoryTypeBits & (1 << i))) {
            memIndex.emplace(i);
        }
    }
    if(!memIndex.has_value()) {
        LOG("Failed to find appropriate memory type.\n");
        return;
    }

    VkMemoryAllocateInfo vertexBufAI {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = bufMemReq.size,
        .memoryTypeIndex = memIndex.value()
    };
    vkAllocateMemory(sjt4::rndr::device, &vertexBufAI, nullptr, &this->vboMem);
    vkBindBufferMemory(sjt4::rndr::device, this->vertexBuf, this->vboMem, NULL);
    /*for(uint32_t i{}; i<((*reinterpret_cast<const uint32_t*>(data-sizeof(uint32_t)))*2); ++i) {
        std::cout << reinterpret_cast<const glm::vec3*>(data+(sizeof(glm::vec3)*(i))) << '\n';
    }*/
    vkMapMemory(sjt4::rndr::device, this->vboMem, NULL, vertexBufAI.allocationSize, NULL, &mappedMem);
    std::memcpy(mappedMem, data, vertBufCI.size);
    data+= vertBufCI.size;
    vkUnmapMemory(sjt4::rndr::device, this->vboMem);
} 
bool sjt4::rndr::readSceneData(const char *data, uint8_t sceneTemplateIndex) {
     LOG("Number of availiable scene templates: %d\n" COMMA *reinterpret_cast<const uint8_t*>(data));
    if(*reinterpret_cast<const uint8_t*>(data) <= sceneTemplateIndex) {
        LOG("Requested scene index is out of range.\n");
        return false;
    }
    data+=sizeof(const uint8_t);
    
    for(uint8_t i{0}; i<sceneTemplateIndex; ++i)
        data+= *reinterpret_cast<const std::size_t*>(data);
    data+= sizeof(const std::size_t);
    auto objectCount {*reinterpret_cast<const uint16_t*>(data)};
    data+= sizeof(const uint16_t);
    sjt4::rndr::sceneObjects.resize(objectCount);
    for(uint16_t j{}; j<objectCount; ++j) {
        sjt4::rndr::sceneObjects[j].readData(data);
    }
    LOG("Objects in the loaded scene: %d\n" COMMA objectCount);
    return true;
}

sjt4::rndr::DrawableObject_t::~DrawableObject_t() {
    vkDestroyBuffer(sjt4::rndr::device, this->uniformBuf, nullptr);
    vkDestroyBuffer(sjt4::rndr::device, this->vertexBuf, nullptr);
    vkFreeMemory(sjt4::rndr::device, this->uboMem, nullptr);
    vkFreeMemory(sjt4::rndr::device, this->vboMem, nullptr);
}