#ifndef _TUVOK_H_
#define _TUVOK_H

#define GLFW_INCLUDE_VULKAN
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GLFW/glfw3.h>

// use validation layers (like the debug layers in D3D11)
#ifdef DEBUG 
#define USE_VALIDATION_LAYERS 1u

// handle the debug info sended to us
static VkBool32 VKAPI_PTR vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT*  pCallbackData,
		void* pUserData);


static PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_ext;
static PFN_vkDestroyDebugUtilsMessengerEXT vk_destroy_debug_ext;

#else
#define USE_VALIDATION_LAYERS 0u
#endif

typedef struct tuvok_t
{
	GLFWwindow* window;
	VkInstance vk_instance; // represents the system at a higher level 
	VkDebugUtilsMessengerEXT vk_debug; // debug utils instance 
	VkPhysicalDevice gpu; // represents a GPU with vulkan support
	VkDevice device; // logical representation of our gpu
	VkQueue queue; // queues store the commands we send to the gpu
}tuvok;

tuvok* init_lib(uint32_t width, uint32_t height, const char* window_name);
void free_lib(tuvok* tvk);

#endif
