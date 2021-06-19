#include "tuvok.h"

tuvok* init_lib(uint32_t width, uint32_t height, const char* window_name)
{
	tuvok* tvk = (tuvok*) malloc(sizeof(tuvok));
	memset(tvk, 0, sizeof(tuvok));

	// --------------------------------------------------
	// init all our window and contexts using glfw
	// --------------------------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	tvk->window = glfwCreateWindow(width, height, window_name, NULL, NULL);

	// --------------------------------------------------
	// init Vulkan (you mean Vulcan, right!?)
	// --------------------------------------------------
	VkInstanceCreateInfo vk_ici = {};
	vk_ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	uint32_t has_validation_layer = 0u;

#if USE_VALIDATION_LAYERS
	// enumerate all avaliable Vulkan layers 
	uint32_t layer_count = 0u;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);

	char* instance_layers[VK_MAX_EXTENSION_NAME_SIZE];
	VkLayerProperties* vk_lps = (VkLayerProperties*) malloc(layer_count*sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layer_count, vk_lps);

	// check if the validation layer (a.k.a debug layer) is present
	for (uint32_t i = 0u; i < layer_count; ++i){
		has_validation_layer = (has_validation_layer)? has_validation_layer : 
			!(strcmp(vk_lps[i].layerName, "VK_LAYER_KHRONOS_validation")); 

		instance_layers[i] = (char*) malloc(strlen(vk_lps[i].layerName));
		strcpy(instance_layers[i], vk_lps[i].layerName);
	}

	if (has_validation_layer)
		fprintf(stdout, "TUVOK DEBUG MODE: Using vulkan verification layers\n");
	else
		fprintf(stderr, "TUVOK WARNING: Failed to find the VK_LAYER_KHRONOS_validation layer for debugging!\n");

	// pass all the layers to the vulkan instance
	vk_ici.enabledLayerCount = layer_count;
	vk_ici.ppEnabledLayerNames = (layer_count)? instance_layers : NULL;

	for (uint32_t i = 0u; i < layer_count; ++i)
		free(instance_layers[i]);
	free(vk_lps);

	// get available Vulkan extensions 
	uint32_t ext_count = 0u;
	const char** vk_ext = glfwGetRequiredInstanceExtensions(&ext_count);
	
	// add the debug utils to allow formating of the validation layer input
	// using the debug_utils extension VK_EXT_debug_utils
	char** tmp_ext = (char**) malloc((ext_count + 1)*sizeof(char*));
	for (uint32_t i = 0u; i < ext_count; ++i){
		tmp_ext[i] = (char*) malloc(strlen(vk_ext[i]));
		strcpy(tmp_ext[i], vk_ext[i]);
	}
	tmp_ext[ext_count] = (char*) malloc(strlen("VK_EXT_debug_utils"));
	strcpy(tmp_ext[ext_count], "VK_EXT_debug_utils");

	vk_ici.enabledExtensionCount = ext_count + 1;
	vk_ici.ppEnabledExtensionNames = tmp_ext;

	if(vkCreateInstance(&vk_ici, NULL, &tvk->vk_instance) != VK_SUCCESS)
	{
		free_lib(tvk);
		fprintf(stderr, "TUVOK ERROR: Failed to create vulkan instance!\n");
		return NULL;	
	}

	for (uint32_t i = 0u; i < ext_count + 1; ++i)
		free(tmp_ext[i]);
#else
	// get available Vulkan extensions 
	uint32_t ext_count = 0u;
	const char** vk_ext = glfwGetRequiredInstanceExtensions(&ext_count);
	
	vk_ici.enabledExtensionCount = ext_count;
	vk_ici.ppEnabledExtensionNames = vk_ext;
	
	if(vkCreateInstance(&vk_ici, NULL, &tvk->vk_instance) != VK_SUCCESS)
	{
		free_lib(tvk);
		fprintf(stderr, "TUVOK ERROR: Failed to create vulkan instance!\n");
		return NULL;	
	}
#endif
	


	return tvk;
}

void free_lib(tuvok* tvk)
{
	if (tvk)
	{
		if (tvk->window)
			glfwDestroyWindow(tvk->window);
		if (tvk->vk_instance)
			vkDestroyInstance(tvk->vk_instance, NULL);

		glfwTerminate();
	}
}

#if USE_VALIDATION_LAYERS
VkBool32 VKAPI_PTR vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT*  pCallbackData,
		void* pUserData)
{
	switch(messageSeverity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			{

			}break;

		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			{
				fprintf(stderr, "TUVOK WARNING: \t%s\n", pCallbackData->pMessage);
			}break;

		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			{
				fprintf(stderr, "TUVOK ERROR: \t%s\n", pCallbackData->pMessage);
			}break;
	};

	// should always return false (specs restriction)
	return VK_FALSE;
}
#endif
