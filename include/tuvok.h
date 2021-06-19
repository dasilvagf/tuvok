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
VkBool32 VKAPI_PTR vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT*  pCallbackData,
		void* pUserData);
#else
#define USE_VALIDATION_LAYERS 0u
#endif

typedef struct tuvok_t
{
	GLFWwindow* window;
	VkInstance vk_instance; // represents the system at a higher level 
	VkDebugUtilsMessengerEXT vk_debug; // debug utils instance 
}tuvok;

tuvok* init_lib(uint32_t width, uint32_t height, const char* window_name);
void free_lib(tuvok* tvk);

#endif
