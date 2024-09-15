#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include <haru_meta.h>

bool is_device_suitable(VkInstance inst, VkPhysicalDevice device) {
  VkPhysicalDeviceProperties props;
  PFN_vkGetPhysicalDeviceProperties pfnGetPhysicalDeviceProperties =
      (PFN_vkGetPhysicalDeviceProperties)glfwGetInstanceProcAddress(
          inst, "vkGetPhysicalDeviceProperties");

  pfnGetPhysicalDeviceProperties(device, &props);

  return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
         props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

int main(void) {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW, exiting..\n");
    return -1;
  }

  if (glfwVulkanSupported() == GLFW_FALSE) {
    fprintf(stderr, "Vulkan isn't supported, exiting..\n");
    glfwTerminate();
    return -1;
  }

  PFN_vkCreateInstance pfnCreateInstance =
      (PFN_vkCreateInstance)glfwGetInstanceProcAddress(0, "vkCreateInstance");

  PFN_vkDestroyInstance pfnDestroyInstance =
      (PFN_vkDestroyInstance)glfwGetInstanceProcAddress(0, "vkDestroyInstance");

  VkInstance inst;
  VkApplicationInfo app_info;
  memset(&app_info, 0, sizeof(app_info));

  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = __PROJ_NAME__;
  app_info.applicationVersion = __PROJ_VERSION__;
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = __PROJ_VERSION__;
  app_info.apiVersion = __VULKAN_VERSION__;

  uint32_t inst_ext_count = 0;
  const char **instance_extensions =
      glfwGetRequiredInstanceExtensions(&inst_ext_count);
  if (instance_extensions == 0) {
    fprintf(stderr, "Couldn't get required instance extensions, exiting..\n");
    glfwTerminate();
    return -1;
  }

  VkInstanceCreateInfo create_info;
  memset(&create_info, 0, sizeof(create_info));

  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = inst_ext_count;
  create_info.ppEnabledExtensionNames = instance_extensions;

  if (pfnCreateInstance(&create_info, 0, &inst) != VK_SUCCESS) {
    fprintf(stderr, "Failed to create instance, exiting..\n");
    glfwTerminate();
    return -1;
  }

  PFN_vkEnumeratePhysicalDevices pfnEnumeratePhysicalDevices =
      (PFN_vkEnumeratePhysicalDevices)glfwGetInstanceProcAddress(
          inst, "vkEnumeratePhysicalDevices");

  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  uint32_t device_count = 0;
  VkResult enum_res;

  enum_res = pfnEnumeratePhysicalDevices(inst, &device_count, 0);
  if (enum_res != VK_SUCCESS) {
    fprintf(stderr, "Failed to get amount of devices, res: %d, exiting..\n",
            enum_res);
    pfnDestroyInstance(inst, 0);
    glfwTerminate();
    return -1;
  }

  if (device_count == 0) {
    fprintf(stderr, "No devices available, exiting\n");
    pfnDestroyInstance(inst, 0);
    glfwTerminate();
    return -1;
  }

  VkPhysicalDevice devices[8];

  if (pfnEnumeratePhysicalDevices(inst, &device_count, devices) != VK_SUCCESS) {
    fprintf(stderr, "Failed to enumerate physical devices, exiting..\n");
    pfnDestroyInstance(inst, 0);
    glfwTerminate();
    return -1;
  }

  for (uint32_t i = 0; i < device_count; i++) {
    if (is_device_suitable(inst, devices[i])) {
      // It assigns here.
      printf("devices[%d] is suitable!\n", i);
      physical_device = devices[i];
      break;
    }
  }

  // it didn't assign anyway??
  if (physical_device == VK_NULL_HANDLE) {
    fprintf(stderr, "Couldn't set device, exiting..\n");
    pfnDestroyInstance(inst, 0);
    glfwTerminate();
    return -1;
  }

  VkDeviceCreateInfo device_info;
  memset(&device_info, 0, sizeof(device_info));

  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  PFN_vkCreateDevice pfnCreateDevice =
      (PFN_vkCreateDevice)glfwGetInstanceProcAddress(inst, "vkCreateDevice");

  PFN_vkDestroyDevice pfnDestroyDevice =
      (PFN_vkDestroyDevice)glfwGetInstanceProcAddress(inst, "vkDestroyDevice");

  VkDevice device;
  VkResult create_device_result;

  create_device_result =
      pfnCreateDevice(physical_device, &device_info, 0, &device);
  if (create_device_result != VK_SUCCESS) {
    fprintf(stderr, "Failed to create device, VKRESULT: %d, exiting..\n",
            create_device_result);
    pfnDestroyInstance(inst, 0);
    glfwTerminate();
    return -1;
  }

  PFN_vkGetPhysicalDeviceQueueFamilyProperties
      pfnGetPhysicalDeviceQueueFamilyProperties =
          (PFN_vkGetPhysicalDeviceQueueFamilyProperties)
              glfwGetInstanceProcAddress(
                  inst, "vkGetPhysicalDeviceQueueFamilyProperties");

  uint32_t queue_familyc = 0;

  pfnGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_familyc, 0);
  if (queue_familyc == 0) {
    fprintf(stderr, "No queue families available, exiting..\n");
    pfnDestroyDevice(device, 0);
    pfnDestroyInstance(inst, 0);
    glfwTerminate();
    return -1;
  }

  uint32_t queue_familyi = 0;
  for (; queue_familyi != queue_familyc; queue_familyi++) {
    if (glfwGetPhysicalDevicePresentationSupport(inst, physical_device,
                                                 queue_familyi)) {
      break;
    }
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window = glfwCreateWindow(640, 480, __PROJ_NAME__, 0, 0);

  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(inst, window, 0, &surface) != VK_SUCCESS) {
    fprintf(stderr, "Can't create window surface, exiting..\n");
    glfwDestroyWindow(window);
    pfnDestroyDevice(device, 0);
    pfnDestroyInstance(inst, 0);
    glfwTerminate();
    return -1;
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  pfnDestroyDevice(device, 0);
  pfnDestroyInstance(inst, 0);
  glfwTerminate();

  return 0;
}

// Soz, helping a guy set up git and gpg
