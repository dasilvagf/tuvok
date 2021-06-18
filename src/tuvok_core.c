#include "tuvok.h"

tuvok* init_lib(uint32_t width, uint32_t height, const char* window_name)
{
	tuvok* tvk = (tuvok*) malloc(sizeof(tuvok));
	memset(tvk, 0, sizeof(tuvok));

	//
	// init all our window and contexts using glfw
	//
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	tvk->window = glfwCreateWindow(width, height, window_name, NULL, NULL);

	//
	// init Vulkan (you mean Vulcan, right!?)
	//
	VkInstanceCreateInfo vk_ici = {};
	vk_ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	// get available Vulkan extensions 
	uint32_t ext_count = 0u;
	const char** vlk_ext = glfwGetRequiredInstanceExtensions(&ext_count);
	vk_ici.enabledExtensionCount = ext_count;
	vk_ici.ppEnabledExtensionNames = vlk_ext;

	if(vkCreateInstance(&vk_ici, NULL, tvk->vk_instance) != VK_SUCCESS)
	{
		// no vulkan, therefore we exit gracefully
		free_lib(tvk);
		return NULL;	
	}

	return tvk;
}

void free_lib(tuvok* tvk)
{
	if (tvk)
	{
		if (tvk->window)
			glfwDestroyWindow(tvk->window);
		if (tvk->vk_instance)
			vkDestroyInstance(*(tvk->vk_instance), NULL);

		glfwTerminate();
	}
}
