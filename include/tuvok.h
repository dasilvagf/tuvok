#ifndef _TUVOK_H_
#define _TUVOK_H

#define GLFW_INCLUDE_VULKAN
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <GLFW/glfw3.h>

#ifdef DEBUG 
// use validation layers (like the debug layers in D3D11)
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

static void GLFW_error_function(int error_code, const char* error_msg)
{
	fprintf(stderr, error_msg);
}

typedef struct tuvok_t
{
	GLFWwindow* window;
	int32_t window_width, window_height;
	
	VkInstance vk_instance; // represents the system at a higher level 
	VkSurfaceKHR vk_context; // rendering context between vulkan and the window
	VkDebugUtilsMessengerEXT vk_debug; // debug utils instance 
	VkPhysicalDevice gpu; // represents a GPU with vulkan support
	VkDevice device; // logical representation of our gpu
	VkSwapchainKHR swap_chain; // represents the swap chain. really? G.G
    uint32_t n_swap_images; // # surfaces(vk_images) in the swap_images buffer
    VkImage* swap_images; // surfaces in the swap chain
    VkImageView* swap_views; // act like resource views in D3D
    VkFormat swap_fmt; // represent the format of surfaces in the swap chain
    VkExtent2D swap_extent; // represent the dimension (W/H) of the surfaces

	// queues store the commands we send to the gpu
	uint32_t use_common_queue;
	VkQueue rendering_queue; // rendering commands only
    VkQueue compute_queue; // asynchronous compute only
	VkQueue prensentation_queue; // presentation only	
	VkQueue common_queue; // supports rendering, compute and presentation

}tuvok;

tuvok* init_lib(uint32_t width, uint32_t height, const char* window_name);
void free_lib(tuvok* tvk);

#endif
