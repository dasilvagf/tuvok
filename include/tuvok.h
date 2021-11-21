#ifndef _TUVOK_H_
#define _TUVOK_H

#define GLFW_INCLUDE_VULKAN
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
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
	char gpu_name[256];
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

typedef struct tuvok_shader_t
{
    VkShaderModule shader_module; // contains shader in SPIR-V bytecode
    VkPipelineShaderStageCreateInfo pipe_info; // imformation for the pipeline on how to interpret the bytecode
}tuvok_shader;

typedef struct tuvok_pipeline_desc_t
{
    // describe vertex buffer layouts
    uint32_t n_vb_layouts;
    VkVertexInputBindingDescription* vb_layouts_array; 
    uint32_t n_vb_attributes;
    VkVertexInputAttributeDescription* vb_attributes_array;

    // input assembly
    VkPrimitiveTopology ia_topology;
    VkBool32 ia_restart_enabled;

    // viewport and scissor rect
    uint32_t n_viewports;
    VkViewport* viewports_array;
    uint32_t n_scissor_rects;
    VkRect2D* scissor_rect_array;

}tuvok_pipeline_desc;

typedef struct tuvok_pipeline_t
{

}tuvok_pipeline;

/*
 *      init and shutdown the lib
 */
tuvok* init_lib(uint32_t width, uint32_t height, const char* window_name);
void free_lib(tuvok* tvk);

/*
 *      shader bytecode loading
 */
tuvok_shader* load_shader(const tuvok* tvk, const char* filename, const char* main_name, uint8_t is_bytecode, 
        VkShaderStageFlagBits stage);
void free_shader(const tuvok* tvk, tuvok_shader* shader);

/*
 *      fixed-function and programmanle pipeline configs       
 */
tuvok_pipeline* create_pipeline(const tuvok* tvk, tuvok_pipeline_desc desc);
void free_pipeline(const tuvok* tvk, tuvok_pipeline* pipe);
void set_pipeline(const tuvok* tvk, const tuvok_pipeline* pipe);


#endif
