#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <queue>
#include <functional>
#include <pthread.h>
#include <semaphore.h>

extern "C" {
    #include "type.h"
    #include "plane.h"
    // TODO link with plane.c
    void planeInit(zftype init, uftype dma, vftype safe, yftype main, xftype info, wftype draw) {}
    void planeAddarg(const char *str) {}
    int planeInfo(enum Configure cfg) {return 0;}
    void planeSafe(enum Proc proc, enum Wait wait, enum Configure hint) {}
    void planeMain() {}
    void planeReady(struct Pierce *pierce) {}
    // following are called from plane.c
    void vulkanInit(); // init
    void vulkanDma(struct Center *center); // dma
    void vulkanSafe(); // safe
    void vulkanMain(enum Proc proc, enum Wait wait); // main
    int vulkanInfo(enum Configure query); // info
    void vulkanDraw(enum Micro shader, int base, int limit); // draw
}

struct Input {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Input);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Input, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Input, color);

        return attributeDescriptions;
    }
};
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

typedef void VoidFunc();
struct MainState {
    std::queue<std::function<VoidFunc> > func;
    bool dmaCalled;
    bool drawCalled;
    bool framebufferResized;
    bool escapePressed;
    bool enterPressed;
    bool otherPressed;
    bool windowMoving;
    double mouseLastx;
    double mouseLasty;
    int windowLastx;
    int windowLasty;
    int argc;
    char **argv;
    struct Center *center;
} mainState = {
    .dmaCalled = false,
    .drawCalled = true,
    .framebufferResized = false,
    .escapePressed = false,
    .enterPressed = false,
    .otherPressed = false,
    .windowMoving = false,
    .mouseLastx = 0.0,
    .mouseLasty = 0.0,
    .windowLastx = 0,
    .windowLasty = 0,
    .argc = 0,
    .argv = 0,
    .center = 0,
};
void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    struct MainState *mainState = (struct MainState *)glfwGetWindowUserPointer(window);
    mainState->framebufferResized = true;
}
void keypressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    struct MainState *mainState = (struct MainState *)glfwGetWindowUserPointer(window);
    if (action != GLFW_PRESS || mods != 0) {
        return;
    }
    if (key == GLFW_KEY_ENTER) {
        mainState->enterPressed = true;
    } else {
        mainState->enterPressed = false;
    }
    if (key == GLFW_KEY_ESCAPE) {
        mainState->escapePressed = true;
        mainState->otherPressed = false;
    } else if (mainState->otherPressed) {
        mainState->escapePressed = false;
        mainState->otherPressed = false;
    } else {
        mainState->otherPressed = true;
    }
}
void mouseClicked(GLFWwindow* window, int button, int action, int mods) {
    struct MainState *mainState = (struct MainState *)glfwGetWindowUserPointer(window);
    if (action != GLFW_PRESS) {
        return;
    }
    mainState->windowMoving = !mainState->windowMoving;
    if (mainState->windowMoving) {
        glfwGetCursorPos(window,&mainState->mouseLastx,&mainState->mouseLasty);
        glfwGetWindowPos(window,&mainState->windowLastx,&mainState->windowLasty);
    }
}
void mouseMoved(GLFWwindow* window, double xpos, double ypos) {
    struct MainState *mainState = (struct MainState *)glfwGetWindowUserPointer(window);
    double mouseNextx, mouseNexty;
    int windowNextx, windowNexty;
    glfwGetCursorPos(window,&mouseNextx,&mouseNexty);
    if (mainState->windowMoving) {
        windowNextx = mainState->windowLastx + (mouseNextx - mainState->mouseLastx);
        windowNexty = mainState->windowLasty + (mouseNexty - mainState->mouseLasty);
        glfwSetWindowPos(window,windowNextx,windowNexty);
        mainState->windowLastx = windowNextx; mainState->windowLasty = windowNexty;
    }
}

GLFWcursor *initMoveCursor(bool e, bool t, bool r, bool b, bool l) {
    int dim = 11;
    int hot = dim/2;
    unsigned char pixels[dim * dim * 4];
    memset(pixels, 0x00, sizeof(pixels));

    for (int k = 0; k < dim; k++) for (int j = 0; j < dim; j++) for (int i = 0; i < 4; i++) {
        if (k == 0 || k == dim-1) pixels[k*dim*4+j*4+i] = 0xff;
        if (j == 0 || j == dim-1) pixels[k*dim*4+j*4+i] = 0xff;
        if (k == hot-2 && j >= hot-1 && j <= hot+1) pixels[k*dim*4+j*4+i] = 0xff;
        if (j == hot-2 && k >= hot-1 && k <= hot+1) pixels[k*dim*4+j*4+i] = 0xff;
        if (k == hot+2 && j >= hot-1 && j <= hot+1) pixels[k*dim*4+j*4+i] = 0xff;
        if (j == hot+2 && k >= hot-1 && k <= hot+1) pixels[k*dim*4+j*4+i] = 0xff;
        if (e && k >= hot-1 && k <= hot+1 && j >= hot-1 && j <= hot+1) pixels[k*dim*4+j*4+i] = 0xff;
        if (t && k > hot+2 && j == hot) pixels[k*dim*4+j*4+i] = 0xff;
        if (r && j > hot+2 && k == hot) pixels[k*dim*4+j*4+i] = 0xff;
        if (b && k < hot-2 && j == hot) pixels[k*dim*4+j*4+i] = 0xff;
        if (l && j < hot-2 && k == hot) pixels[k*dim*4+j*4+i] = 0xff;
    }

    GLFWimage image;
    image.width = dim;
    image.height = dim;
    image.pixels = pixels;

    return glfwCreateCursor(&image, hot, hot);
}
GLFWcursor *initRotateCursor(bool e) {
    int dim = 11;
    int hot = dim/2;
    unsigned char pixels[dim * dim * 4];
    memset(pixels, 0x00, sizeof(pixels));

    for (int k = 0; k < dim; k++) for (int j = 0; j < dim; j++) for (int i = 0; i < 4; i++) {
        int diffx = j-hot;
        int diffy = k-hot;
        int exact = hot*hot;
        int square = diffx*diffx + diffy*diffy;
        bool center = k >= hot-1 && k <= hot+1 && j >= hot-1 && j <= hot+1;
        if (square < exact+5 && !center) pixels[k*dim*4+j*4+i] = 0xff;
        if (e && center) pixels[k*dim*4+j*4+i] = 0xff;
    }

    GLFWimage image;
    image.width = dim;
    image.height = dim;
    image.pixels = pixels;

    return glfwCreateCursor(&image, hot, hot);
}
GLFWcursor *initTranslateCursor(bool e) {
    int dim = 11;
    int hot = dim/2;
    unsigned char pixels[dim * dim * 4];
    memset(pixels, 0x00, sizeof(pixels));

    for (int k = 0; k < dim; k++) for (int j = 0; j < dim; j++) for (int i = 0; i < 4; i++) {
        int diffx = (j>hot?j-hot:hot-j);
        int diffy = (k>hot?k-hot:hot-k);
        int sum = diffx + diffy;
        bool center = k >= hot-1 && k <= hot+1 && j >= hot-1 && j <= hot+1;
        if (!center && sum < hot+1) pixels[k*dim*4+j*4+i] = 0xff;
        if (e && center) pixels[k*dim*4+j*4+i] = 0xff;
    }

    GLFWimage image;
    image.width = dim;
    image.height = dim;
    image.pixels = pixels;

    return glfwCreateCursor(&image, hot, hot);
}
GLFWcursor *initRefineCursor() {
    int dim = 11;
    int hot = dim/2;
    unsigned char pixels[dim * dim * 4];
    memset(pixels, 0x00, sizeof(pixels));

    for (int k = 0; k < dim; k++) for (int j = 0; j < dim; j++) for (int i = 0; i < 4; i++) {
        int diffx = j-hot;
        int diffy = k-hot;
        if (diffx == diffy) pixels[k*dim*4+j*4+i] = 0xff;
        if (diffx == -diffy) pixels[k*dim*4+j*4+i] = 0xff;
        if (j == hot) pixels[k*dim*4+j*4+i] = 0xff;
        if (k == hot) pixels[k*dim*4+j*4+i] = 0xff;
    }

    GLFWimage image;
    image.width = dim;
    image.height = dim;
    image.pixels = pixels;

    return glfwCreateCursor(&image, hot, hot);
}
GLFWcursor *initSculptCursor(bool e) {
    int dim = 11;
    int hot = dim/2;
    unsigned char pixels[dim * dim * 4];
    memset(pixels, 0x00, sizeof(pixels));

    for (int k = 0; k < dim; k++) for (int j = 0; j < dim; j++) for (int i = 0; i < 4; i++) {
        bool center = k >= hot-2 && k <= hot+2 && j >= hot-2 && j <= hot+2;
        if ((e || !center) && j == hot) pixels[k*dim*4+j*4+i] = 0xff;
        if ((e || !center) && k == hot) pixels[k*dim*4+j*4+i] = 0xff;
    }

    GLFWimage image;
    image.width = dim;
    image.height = dim;
    image.pixels = pixels;

    return glfwCreateCursor(&image, hot, hot);
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

bool checkValidationLayerSupport(const std::vector<const char*> validationLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> getRequiredExtensions(bool enableValidationLayers) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

std::optional<uint32_t> findGraphicsFamily(VkPhysicalDevice device, VkSurfaceKHR surface) {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            presentFamily = i;
        }

        if (graphicsFamily.has_value() && presentFamily.has_value()) {
            break;
        }

        i++;
    }
    return graphicsFamily;
}
std::optional<uint32_t> findPresentFamily(VkPhysicalDevice device, VkSurfaceKHR surface) {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            presentFamily = i;
        }

        if (graphicsFamily.has_value() && presentFamily.has_value()) {
            break;
        }

        i++;
   }
    return presentFamily;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> deviceExtensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VkSurfaceCapabilitiesKHR querySurfaceCapabilities(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surfaceCapabilities);
    return surfaceCapabilities;
}
std::vector<VkSurfaceFormatKHR> querySurfaceFormats(VkPhysicalDevice device, VkSurfaceKHR surface) {
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, surfaceFormats.data());
    }
    return surfaceFormats;
}
std::vector<VkPresentModeKHR> queryPresentModes(VkPhysicalDevice device, VkSurfaceKHR surface) {
    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
    }
    return presentModes;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*> deviceExtensions) {
    if (!checkDeviceExtensionSupport(device,deviceExtensions)) {
        return false;
    }
    if (querySurfaceFormats(device,surface).empty()) {
        return false;
    }
    if (queryPresentModes(device,surface).empty()) {
        return false;
    }
    if (!findGraphicsFamily(device,surface).has_value()) {
        return false;
    }
    if (!findPresentFamily(device,surface).has_value()) {
        return false;
    }
    return true;
}

VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*> deviceExtensions) {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device,surface,deviceExtensions)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    return physicalDevice;
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, uint32_t graphicsFamily, uint32_t presentFamily, const std::vector<const char*> validationLayers, const std::vector<const char*> deviceExtensions, bool enableValidationLayers) {
    VkDevice device;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicsFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    if (presentFamily != graphicsFamily) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = presentFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    return device;
}

VkSwapchainKHR createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D swapChainExtent, uint32_t minImageCount, uint32_t queueFamilyIndices[]) {

    VkSwapchainKHR swapChain;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = minImageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = querySurfaceCapabilities(physicalDevice,surface).currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    return swapChain;
}

VkImageView createImageView(VkDevice device, VkFormat swapChainImageFormat, VkImage swapChainImage) {
    VkImageView swapChainImageView;
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChainImage;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = swapChainImageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image views!");
    }
    return swapChainImageView;
}

VkRenderPass createRenderPass(VkDevice device, VkFormat swapChainImageFormat) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    return renderPass;
}

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device) {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    return descriptorSetLayout;
}

VkPipelineLayout createPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    return pipelineLayout;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkPipeline createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout, const char *vertexShader, const char *fragmentShader) {
    auto vertShaderCode = readFile(vertexShader);
    auto fragShaderCode = readFile(fragmentShader);

    VkShaderModule vertShaderModule = createShaderModule(device,vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(device,fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Input::getBindingDescription();
    auto attributeDescriptions = Input::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);

    return graphicsPipeline;
}

VkFramebuffer createFramebuffer(VkDevice device, VkExtent2D swapChainExtent, VkImageView swapChainImageView, VkRenderPass renderPass) {
    VkFramebuffer swapChainFramebuffer;
    VkImageView attachments[] = {
        swapChainImageView
    };
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;
    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
    return swapChainFramebuffer;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t graphicsFamily) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsFamily;

    VkCommandPool commandPool;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
    return commandPool;
}

VkCommandBuffer allocateCommandBuffer(VkDevice device, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    return commandBuffer;
}

void *createMapped(VkDevice device, VkDeviceMemory memory, VkDeviceSize size) {
    void *mapped;
    vkMapMemory(device, memory, 0, size, 0, &mapped);
    return mapped;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkBuffer createBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }
    return buffer;
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VkDeviceMemory createMemory(VkPhysicalDevice physicalDevice, VkDevice device, VkBuffer buffer, VkMemoryPropertyFlags properties) {
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    VkDeviceMemory memory;
    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    return memory;
}

VkDescriptorPool createDescriptorPool(VkDevice device, int count) {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(count);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(count);

    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    return descriptorPool;
}

VkDescriptorSet createDescriptorSet(VkDevice device, VkBuffer uniformBuffer, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet descriptorSet;
    if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    return descriptorSet;
}

VkCommandBuffer createCommandBuffer(VkDevice device, VkCommandPool commandPool) {
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)1;
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    return commandBuffer;
}

VkSemaphore createSemaphore(VkDevice device) {
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }
    return semaphore;
}

VkFence createFence(VkDevice device) {
    VkFence fence;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    }
    return fence;
}

void vulkanInit() {
    for (int arg = 0; arg < mainState.argc; arg++) planeAddarg(mainState.argv[arg]);
}
void vulkanDma(struct Center *center) {
    // TODO mainState.func.push(std::function(
}
void vulkanSafe() {
    glfwPostEmptyEvent();
}
void vulkanMain(enum Proc proc, enum Wait wait) {
    // TODO trust plane.c to call this
}
int vulkanInfo(enum Configure query) {
    return 0; // TODO
}
void vulkanDraw(enum Micro shader, int base, int limit) {
    // TODO mainState.func.push(std::function(
}

struct OpenState {
    GLFWwindow* window;
    GLFWcursor* moveCursor[2][2][2][2][2];
    GLFWcursor* rotateCursor[2];
    GLFWcursor* translateCursor[2];
    GLFWcursor* refineCursor;
    GLFWcursor* sculptCursor[2];
    GLFWcursor* standardCursor;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    bool enableValidationLayers;
    OpenState(uint32_t WIDTH, uint32_t HEIGHT, const std::vector<const char*> validationLayers, bool enableValidationLayers) {
        glfwInit();
        VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
        [](VkDebugUtilsMessengerCreateInfoEXT& debugInfo) {
            debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugInfo.pfnUserCallback = debugCallback;
        } (debugInfo);
        [](VkInstance& instance, bool enableValidationLayers, VkDebugUtilsMessengerCreateInfoEXT debugInfo, const std::vector<const char*> validationLayers) {
            if (enableValidationLayers && !checkValidationLayerSupport(validationLayers))
                throw std::runtime_error("validation layers requested, but not available!");
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0; 
            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            auto extensions = getRequiredExtensions(enableValidationLayers);
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugInfo;}
            else {
                createInfo.enabledLayerCount = 0;
                createInfo.pNext = nullptr;}
            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
                throw std::runtime_error("failed to create instance!");
        } (instance,enableValidationLayers,debugInfo,validationLayers);
        if (enableValidationLayers) [](VkDebugUtilsMessengerEXT& debugMessenger, VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT debugInfo) {
            if (CreateDebugUtilsMessengerEXT(instance, &debugInfo, nullptr, &debugMessenger) != VK_SUCCESS)
                throw std::runtime_error("failed to set up debug messenger!");
        } (debugMessenger,instance,debugInfo);
        [](GLFWwindow*& window, const uint32_t WIDTH, const uint32_t HEIGHT) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        } (window,WIDTH,HEIGHT);
        glfwSetWindowUserPointer(window, &mainState);
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetKeyCallback(window, keypressCallback);
        glfwSetMouseButtonCallback(window, mouseClicked);
        glfwSetCursorPosCallback(window, mouseMoved);
        for (int t = 0; t < 2; t++) for (int b = 0; b < 2; b++)
        for (int l = 0; l < 2; l++) for (int r = 0; r < 2; r++)
        for (int e = 0; e < 2; e++) moveCursor[e][t][r][b][l] = initMoveCursor(e,t,r,b,l);
        for (int e = 0; e < 2; e++) rotateCursor[e] = initRotateCursor(e);
        for (int e = 0; e < 2; e++) translateCursor[e] = initTranslateCursor(e);
        refineCursor = initRefineCursor();
        for (int e = 0; e < 2; e++) sculptCursor[e] = initSculptCursor(e);
        standardCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        glfwSetCursor(window,moveCursor[true][true][true][true][true]);
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface!");
        this->enableValidationLayers = enableValidationLayers;
    }
    ~OpenState() {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        if (enableValidationLayers) DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        for (int t = 0; t < 2; t++) for (int b = 0; b < 2; b++)
        for (int l = 0; l < 2; l++) for (int r = 0; r < 2; r++)
        for (int e = 0; e < 2; e++) glfwDestroyCursor(moveCursor[e][t][r][b][l]);
        for (int e = 0; e < 2; e++) glfwDestroyCursor(rotateCursor[e]);
        for (int e = 0; e < 2; e++) glfwDestroyCursor(translateCursor[e]);
        glfwDestroyCursor(refineCursor);
        for (int e = 0; e < 2; e++) glfwDestroyCursor(sculptCursor[e]);
        glfwDestroyCursor(standardCursor);
        glfwDestroyWindow(window);
        vkDestroyInstance(instance, nullptr);
        glfwTerminate();
    }
};

struct LoadState {
    static const int NUM_QUEUE_FAMILIES = 2;
    VkPhysicalDevice physicalDevice;
    uint32_t* queueFamilyIndices;
    uint32_t minImageCount;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkFormat swapChainImageFormat;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    LoadState(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*> validationLayers ,const std::vector<const char*> deviceExtensions, bool enableValidationLayers, const int MAX_BUFFERS_AVAILABLE) {
        physicalDevice = pickPhysicalDevice(instance,surface,deviceExtensions);
        queueFamilyIndices = new uint32_t[NUM_QUEUE_FAMILIES];
        queueFamilyIndices[0] = findGraphicsFamily(physicalDevice,surface).value();
        queueFamilyIndices[1] = findPresentFamily(physicalDevice,surface).value();
        minImageCount = querySurfaceCapabilities(physicalDevice,surface).minImageCount + 1;
        if (querySurfaceCapabilities(physicalDevice,surface).maxImageCount > 0 &&
            minImageCount > querySurfaceCapabilities(physicalDevice,surface).maxImageCount)
            minImageCount = querySurfaceCapabilities(physicalDevice,surface).maxImageCount;
        surfaceFormat = chooseSwapSurfaceFormat(querySurfaceFormats(physicalDevice,surface));
        presentMode = chooseSwapPresentMode(queryPresentModes(physicalDevice,surface));
        swapChainImageFormat = surfaceFormat.format;
        device = createLogicalDevice(physicalDevice,queueFamilyIndices[0],queueFamilyIndices[1],validationLayers,deviceExtensions,enableValidationLayers);
        vkGetDeviceQueue(device, queueFamilyIndices[0], 0, &graphicsQueue);
        vkGetDeviceQueue(device, queueFamilyIndices[1], 0, &presentQueue);
        renderPass = createRenderPass(device,swapChainImageFormat);
        descriptorSetLayout = createDescriptorSetLayout(device);
        pipelineLayout = createPipelineLayout(device,descriptorSetLayout);
        graphicsPipeline = createGraphicsPipeline(device,renderPass,pipelineLayout,"vulkan.vsv","vulkan.fsv");
        commandPool = createCommandPool(device, findGraphicsFamily(physicalDevice,surface).value());
        descriptorPool = createDescriptorPool(device, MAX_BUFFERS_AVAILABLE);
    }
    ~LoadState() {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroyDevice(device, nullptr);
    }
};

template<class Buffer> struct BufferQueue {
    std::queue<Buffer*> queue;
    std::queue<int> ident;
    std::queue<Buffer*> pool;
    std::function<Buffer*(int)> make;
    Buffer *ready;
    int size;
    BufferQueue(std::function<Buffer*(int)> make) {
        this->make = make;
        ready = 0;
        size = 0;
    }
    ~BufferQueue() {
        while (!queue.empty()) {delete queue.front(); queue.pop();}
        while (!pool.empty()) {delete pool.front(); pool.pop();}
        if (ready) delete ready;
    }
    Buffer &get() {
        while (!queue.empty() && queue.front()->done()) {
            pool.push(ready); ready = queue.front(); queue.pop();}
        if (!ready) {throw std::runtime_error("no buffer to get!");}
        return *ready;
    }
    void set(int loc, int siz, void *ptr) {
        if (loc+siz > size) {
            while (!pool.empty()) {delete pool.front(); pool.pop();}
            size = loc+siz;}
        if (pool.empty()) pool.push(make(size));
        pool.front()->start(loc,siz,ptr);
        queue.push(pool.front());
        pool.pop();
    }
};

struct FetchBuffer {
    VkDevice device;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    VkDeviceSize vertexBufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
    void *stagingMapped;
    VkFence inFlightFence;
    VkCommandBuffer commandBuffer;
    FetchBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, const void *ptr, int siz) {
        this->device = device;
        this->graphicsQueue = graphicsQueue;
        this->commandPool = commandPool;
        vertexBufferSize = siz;
        stagingBuffer = createBuffer(device,vertexBufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        stagingMemory = createMemory(physicalDevice,device,stagingBuffer,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);
        vertexBuffer = createBuffer(device,vertexBufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        vertexMemory = createMemory(physicalDevice,device,vertexBuffer,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vkBindBufferMemory(device, vertexBuffer, vertexMemory, 0);
        stagingMapped = createMapped(device,stagingMemory,vertexBufferSize);
        memcpy(stagingMapped, ptr, (size_t) vertexBufferSize);
        inFlightFence = createFence(device);
        commandBuffer = allocateCommandBuffer(device,commandPool);
    }
    ~FetchBuffer() {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        vkDestroyFence(device, inFlightFence, nullptr);
        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
    }
    void setup() {
        vkResetFences(device, 1, &inFlightFence);
        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
    }
    bool ready() {
        VkResult result;

        result = vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, 0);
        if (result == VK_ERROR_DEVICE_LOST) {
            throw std::runtime_error("device lost on wait for fetch fence!");
        }
        if (result != VK_SUCCESS && result != VK_TIMEOUT) {
            throw std::runtime_error("failed to wait for fetch fences!");
        }
        if (result == VK_TIMEOUT) {
            return false;
        }
        return true;
    }
};

struct ChangeBuffer {
    VkDevice device;
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformMemory;
    void* uniformMapped;
    VkDescriptorSet descriptorSet;
    ChangeBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {
        this->device = device;
        VkDeviceSize uniformBufferSize = sizeof(UniformBufferObject);
        uniformBuffer = createBuffer(device,uniformBufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        uniformMemory = createMemory(physicalDevice,device,uniformBuffer,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkBindBufferMemory(device, uniformBuffer, uniformMemory, 0);
        uniformMapped = createMapped(device,uniformMemory,uniformBufferSize);
        descriptorSet = createDescriptorSet(device,uniformBuffer,descriptorSetLayout,descriptorPool);
    }
    ~ChangeBuffer() {
        vkFreeMemory(device, uniformMemory, nullptr);
        vkDestroyBuffer(device, uniformBuffer, nullptr);
    }
};

struct StoreBuffer {
    StoreBuffer() {
    }
    ~StoreBuffer() {
    }
};

struct ThreadState {
    sem_t protect;
    sem_t semaphore;
    pthread_t thread;
    bool finish;
    ThreadState() {
        finish = false;
        if (sem_init(&protect, 0, 1) != 0 ||
            sem_init(&semaphore, 0, 0) != 0 ||
            pthread_create(&thread,0,fenceThread,this) != 0) {
            std::cerr << "failed to create thread!" << std::endl;
        }
    }
    ~ThreadState() {
        if (sem_wait(&protect) != 0) {
            std::cerr << "cannot wait for protect!" << std::endl;
            std::terminate();
        }
        finish = true;
        if (sem_post(&protect) != 0) {
            std::cerr << "cannot post to protect!" << std::endl;
            std::terminate();
        }
        if (sem_post(&semaphore) != 0 ||
            pthread_join(thread,0) != 0 ||
            sem_destroy(&semaphore) != 0 ||
            sem_destroy(&protect) != 0) {
            std::cerr << "failed to join thread!" << std::endl;
            std::terminate();
        }
    }
    static void *fenceThread(void *ptr) {
        struct ThreadState *arg = (ThreadState*)ptr;
        while (1) {
           if (sem_wait(&arg->protect) != 0) {
                throw std::runtime_error("cannot wait for protect!");
            }
            bool finish = arg->finish;
            if (sem_post(&arg->protect) != 0) {
                throw std::runtime_error("cannot post to protect!");
            }
            if (finish) {
                break;
            }
            if (sem_wait(&arg->semaphore) != 0) {
                throw std::runtime_error("cannot wait for semaphore!");
            }
        }
        return 0;
    }
};

struct DrawState {
    VkDevice device;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    VkCommandBuffer commandBuffer;
    DrawState(VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout, VkQueue graphicsQueue, VkQueue presentQueue) {
        this->device = device;
        this->renderPass = renderPass;
        this->graphicsPipeline = graphicsPipeline;
        this->pipelineLayout = pipelineLayout;
        this->graphicsQueue = graphicsQueue;
        this->presentQueue = presentQueue;
        imageAvailableSemaphore = createSemaphore(device);
        renderFinishedSemaphore = createSemaphore(device);
        inFlightFence = createFence(device);
        commandBuffer = createCommandBuffer(device,commandPool);
    }
    ~DrawState() {
        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        vkDestroyFence(device, inFlightFence, nullptr);
    }
    VkResult draw(VkExtent2D swapChainExtent, VkSwapchainKHR swapChain, std::vector<VkFramebuffer> &swapChainFramebuffers, void *uniformMapped, VkDescriptorSet descriptorSet, VkBuffer vertexBuffer, uint32_t size) {
        VkResult result;

        result = vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, 0);
        if (result == VK_ERROR_DEVICE_LOST) {
            throw std::runtime_error("device lost on wait for fence!");
        }
        if (result != VK_SUCCESS && result != VK_TIMEOUT) {
            throw std::runtime_error("failed to wait for fences!");
        }
        if (result == VK_TIMEOUT) {
            return result;
        }

        uint32_t imageIndex;
        result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return result;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
        memcpy(uniformMapped, &ubo, sizeof(ubo));

        vkResetFences(device, 1, &inFlightFence);
        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        vkCmdDraw(commandBuffer, size, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        return result;
    }
};

struct SwapState {
    VkDevice device;
    VkExtent2D swapChainExtent;
    VkSwapchainKHR swapChain;
    uint32_t imageCount;
    std::vector<VkImage> swapChainImages;
    SwapState(GLFWwindow* window, VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, uint32_t minImageCount, uint32_t* queueFamilyIndices) {
        this->device = device;
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwWaitEvents();
            glfwGetFramebufferSize(window, &width, &height);
        }
        swapChainExtent = chooseSwapExtent(window,querySurfaceCapabilities(physicalDevice,surface));
        swapChain = createSwapChain(physicalDevice,device,surface,surfaceFormat,presentMode,swapChainExtent,minImageCount,queueFamilyIndices);
        uint32_t imageCount;
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
    }
    ~SwapState() {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }
};

struct FrameState {
    VkDevice device;
    VkImageView swapChainImageView;
    VkFramebuffer swapChainFramebuffer;
    FrameState(VkDevice device, VkFormat swapChainImageFormat, VkImage swapChainImage, VkExtent2D swapChainExtent, VkRenderPass renderPass) {
        this->device = device;
        swapChainImageView = createImageView(device,swapChainImageFormat,swapChainImage);
        swapChainFramebuffer = createFramebuffer(device,swapChainExtent,swapChainImageView,renderPass);
    }
    ~FrameState() {
        vkDestroyFramebuffer(device, swapChainFramebuffer, nullptr);
        vkDestroyImageView(device, swapChainImageView, nullptr);
    }
};

int main(int argc, char **argv) {
    mainState.argc = argc;
    mainState.argv = argv;
    try {
        planeInit(vulkanInit,vulkanDma,vulkanSafe,vulkanMain,vulkanInfo,vulkanDraw);
        // TODO move following to vulkanMain that calls vulkanOpen vulkanLoad vulkanBuffer vulkanDispatch
        const uint32_t WIDTH = 800;
        const uint32_t HEIGHT = 600;
        const int MAX_FRAMES_IN_FLIGHT = 2;
        const int MAX_BUFFERS_AVAILABLE = 7;
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        #ifdef NDEBUG
        const bool enableValidationLayers = false;
        #else
        const bool enableValidationLayers = true;
        #endif
        const std::vector<Input> vertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        };

        OpenState* openState = new OpenState(WIDTH,HEIGHT,validationLayers,enableValidationLayers);
        GLFWwindow* window = openState->window;
        VkInstance instance = openState->instance;
        VkSurfaceKHR surface = openState->surface;

        LoadState* loadState = new LoadState(instance,surface,validationLayers,deviceExtensions,enableValidationLayers,MAX_BUFFERS_AVAILABLE);
        VkPhysicalDevice physicalDevice = loadState->physicalDevice;
        uint32_t* queueFamilyIndices = loadState->queueFamilyIndices;
        uint32_t minImageCount = loadState->minImageCount;
        VkSurfaceFormatKHR surfaceFormat = loadState->surfaceFormat;
        VkPresentModeKHR presentMode = loadState->presentMode;
        VkFormat swapChainImageFormat = loadState->swapChainImageFormat;
        VkDevice device = loadState->device;
        VkQueue graphicsQueue = loadState->graphicsQueue;
        VkQueue presentQueue = loadState->presentQueue;
        VkRenderPass renderPass = loadState->renderPass;
        VkDescriptorSetLayout descriptorSetLayout = loadState->descriptorSetLayout;
        VkPipelineLayout pipelineLayout = loadState->pipelineLayout;
        VkPipeline graphicsPipeline = loadState->graphicsPipeline;
        VkCommandPool commandPool = loadState->commandPool;
        VkDescriptorPool descriptorPool = loadState->descriptorPool;

        FetchBuffer *fetchBuffer = new FetchBuffer(physicalDevice, device, graphicsQueue, commandPool, vertices.data(), sizeof(vertices[0]) * vertices.size());
        VkBuffer vertexBuffer = fetchBuffer->vertexBuffer;
        VkFence inFlightFence = fetchBuffer->inFlightFence;
        fetchBuffer->setup();
        VkResult result = vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, 0);
        if (result == VK_ERROR_DEVICE_LOST) {
            throw std::runtime_error("device lost on wait for fence!");
        }

        std::vector<ChangeBuffer*> changeBuffers(MAX_BUFFERS_AVAILABLE);
        for (int i = 0; i < changeBuffers.size(); i++)
            changeBuffers[i] = new ChangeBuffer(physicalDevice,device,descriptorSetLayout,descriptorPool);
        std::vector<VkDescriptorSet> descriptorSets(MAX_BUFFERS_AVAILABLE);
        for (int i = 0; i < descriptorSets.size(); i++) descriptorSets[i] = changeBuffers[i]->descriptorSet;
        std::vector<void*> uniformMapped(MAX_BUFFERS_AVAILABLE);
        for (int i = 0; i < uniformMapped.size(); i++) uniformMapped[i] = changeBuffers[i]->uniformMapped;

        std::vector<StoreBuffer*> storeBuffers(MAX_BUFFERS_AVAILABLE);
        for (int i = 0; i < storeBuffers.size(); i++)
            storeBuffers[i] = new StoreBuffer();

        struct ThreadState *threadState = new ThreadState();

        std::vector<DrawState*> drawState(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < drawState.size(); i++) drawState[i] = new DrawState(device,commandPool,renderPass,graphicsPipeline,pipelineLayout,graphicsQueue,presentQueue);
        std::vector<VkSemaphore> imageAvailableSemaphores(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < imageAvailableSemaphores.size(); i++) imageAvailableSemaphores[i] = drawState[i]->imageAvailableSemaphore;
        std::vector<VkSemaphore> renderFinishedSemaphores(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < renderFinishedSemaphores.size(); i++) renderFinishedSemaphores[i] = drawState[i]->renderFinishedSemaphore;
        std::vector<VkFence> inFlightFences(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < inFlightFences.size(); i++) inFlightFences[i] = drawState[i]->inFlightFence;
        std::vector<VkCommandBuffer> commandBuffers(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < commandBuffers.size(); i++) commandBuffers[i] = drawState[i]->commandBuffer;

        struct SwapState *swapState = 0;
        VkExtent2D swapChainExtent;
        VkSwapchainKHR swapChain;
        std::vector<FrameState*> frameState;
        std::vector<VkFramebuffer> swapChainFramebuffers;
        uint32_t currentFrame = 0;
        uint32_t currentBuffer = 0;
        while (!mainState.escapePressed || !mainState.enterPressed) {
            if (!swapState) {
                swapState = new SwapState(window,physicalDevice,device,surface,surfaceFormat,presentMode,minImageCount,queueFamilyIndices);
                swapChainExtent = swapState->swapChainExtent;
                swapChain = swapState->swapChain;
                std::vector<VkImage> swapChainImages = swapState->swapChainImages;
                frameState.resize(swapChainImages.size());
                for (int i = 0; i < frameState.size(); i++) frameState[i] = new FrameState(device,swapChainImageFormat,swapChainImages[i],swapChainExtent,renderPass);
                swapChainFramebuffers.resize(frameState.size());
                for (int i = 0; i < frameState.size(); i++) swapChainFramebuffers[i] = frameState[i]->swapChainFramebuffer;
            }
            glfwWaitEventsTimeout(0.01);
            // TODO sem protect mainState.func.front()() and mainState.func.pop()
            if (mainState.dmaCalled) {
                // TODO change one of the buffers
            }
            if (!mainState.drawCalled) {
                continue;
            }
            VkResult result;
            result = drawState[currentFrame]->draw(swapChainExtent,swapChain,swapChainFramebuffers,uniformMapped[currentBuffer],descriptorSets[currentBuffer],vertexBuffer,static_cast<uint32_t>(vertices.size()));
            if (result == VK_ERROR_OUT_OF_DATE_KHR || mainState.framebufferResized) {
                mainState.framebufferResized = false;
                vkDeviceWaitIdle(device);
                for (int i = 0; i < frameState.size(); i++) delete frameState[i];
                delete swapState;
                swapState = 0;
            } else if (result == VK_TIMEOUT) {
                continue;
            } else if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to present swap chain image!");
            }
            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
            currentBuffer = (currentBuffer + 1) % MAX_BUFFERS_AVAILABLE;
        }
        vkDeviceWaitIdle(device);
        for (int i = 0; i < frameState.size(); i++) delete frameState[i];
        delete swapState;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) delete drawState[i];
        delete threadState;
        for (size_t i = 0; i < MAX_BUFFERS_AVAILABLE; i++) delete storeBuffers[i];
        for (size_t i = 0; i < MAX_BUFFERS_AVAILABLE; i++) delete changeBuffers[i];
        delete fetchBuffer;
        delete loadState;
        delete openState;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
