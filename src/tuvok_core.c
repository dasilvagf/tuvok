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

	char* debug_layer = NULL;
	VkLayerProperties* vk_lps = (VkLayerProperties*) malloc(layer_count*sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layer_count, vk_lps);

	// check if the validation layer (a.k.a debug layer) is present
	for (uint32_t i = 0u; i < layer_count; ++i)
		if (!(strcmp(vk_lps[i].layerName, "VK_LAYER_KHRONOS_validation")))
		{
			debug_layer = (char*) malloc(sizeof(char)*strlen(vk_lps[i].layerName));
			strcpy(debug_layer, vk_lps[i].layerName);
			has_validation_layer = 1u;
			break;
		}

	if (has_validation_layer)
		fprintf(stdout, "TUVOK DEBUG MODE: Using vulkan verification layers\n");
	else
		fprintf(stderr, "TUVOK WARNING: Failed to find the VK_LAYER_KHRONOS_validation layer for debugging!\n");

	// pass all the layers to the vulkan instance
	vk_ici.enabledLayerCount = has_validation_layer;
	vk_ici.ppEnabledLayerNames = (has_validation_layer)? &debug_layer : NULL;

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
	
	// struct for specifying how vulkan will send the msg to us
	VkDebugUtilsMessengerCreateInfoEXT vk_dmsg = {};
	vk_dmsg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	vk_dmsg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	vk_dmsg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	vk_dmsg.pfnUserCallback = vk_debug_callback; // callback function (like MsgProc on Windows)
	//vk_dmsg.pNext = &vk_dmsg;

	// load the functions from the debug extension
	vk_create_debug_ext = (PFN_vkCreateDebugUtilsMessengerEXT) 
		vkGetInstanceProcAddr(tvk->vk_instance, "vkCreateDebugUtilsMessengerEXT");
	vk_destroy_debug_ext = (PFN_vkDestroyDebugUtilsMessengerEXT) 
		vkGetInstanceProcAddr(tvk->vk_instance, "vkDestroyDebugUtilsMessengerEXT");

	if (!vk_create_debug_ext || !vk_destroy_debug_ext)
	{
		free_lib(tvk);
		fprintf(stderr, "TUVOK ERROR: Couldn't load the debug functions from the VK_EXT_debug_utils extension!\n");
		return NULL;	
	}

	if (vk_create_debug_ext(tvk->vk_instance, &vk_dmsg, NULL, &tvk->vk_debug) != VK_SUCCESS)
	{
		free_lib(tvk);
		fprintf(stderr, "TUVOK ERROR: Couldn't create the debug functions from the VK_EXT_debug_utils extension!\n");
		return NULL;	
	}


	// TODO: (GABRIEL) 
	// leaking here: seems like I cont free those guys, maybe vulkan dind't strcpy
	// and is actually using my strings bytes
	/*
	for (uint32_t i = 0u; i < ext_count + 1; ++i)
		free(tmp_ext[i]);
	free(debug_layer);
	*/
	free(vk_lps);
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
	// --------------------------------------------------
	// get a GPU with Vulkan support
	// --------------------------------------------------
	tvk->gpu = VK_NULL_HANDLE;
	
	// get the # of gpus with vulkan support in the system 
	uint32_t gpus_count = 0u;
	vkEnumeratePhysicalDevices(tvk->vk_instance, &gpus_count, NULL);
	if (!gpus_count)
	{
		free_lib(tvk);
		fprintf(stderr, "TUVOK ERROR: The system don't posses a vulkan capable GPU!\n");
		return NULL;	
	}
	VkPhysicalDevice* vulkan_gpus = (VkPhysicalDevice*) malloc(sizeof(VkPhysicalDevice)*gpus_count);
	vkEnumeratePhysicalDevices(tvk->vk_instance, &gpus_count, vulkan_gpus);

	printf("TUVOK INFO: Avaliable GPUs with vulkan support\n");
	for (uint32_t i = 0u; i < gpus_count; ++i){
		VkPhysicalDeviceProperties vk_dp = {};
		vkGetPhysicalDeviceProperties(vulkan_gpus[i], &vk_dp);

		printf("\tGPU name: %s\n", vk_dp.deviceName);
		if (vk_dp.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			printf("\tGPU type: Integrated\n");
		else if (vk_dp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			printf("\tGPU type: Discrete\n");
		else
			printf("\tGPU type: Other\n");
	}

	VkPhysicalDeviceFeatures curr_vk_df = {};
	// get the first gpu that support what want
	for (uint32_t i = 0u; i < gpus_count; ++i){
		VkPhysicalDeviceFeatures vk_df = {};
		vkGetPhysicalDeviceFeatures(vulkan_gpus[i], &vk_df);

		// check the features we want
		if (vk_df.geometryShader)
		{
			curr_vk_df = vk_df;
			tvk->gpu = vulkan_gpus[i];
			break;
		}
	}
	free(vulkan_gpus);

	// --------------------------------------------------
	// get the Queue we'll use to submit our commands
	// --------------------------------------------------

	// no need to check as all vulkan gpus support at least one queue
	uint32_t queue_count;
	vkGetPhysicalDeviceQueueFamilyProperties(tvk->gpu, &queue_count, NULL);

	VkQueueFamilyProperties* queues_family = (VkQueueFamilyProperties*) 
		malloc(sizeof(VkQueueFamilyProperties)*queue_count);
	vkGetPhysicalDeviceQueueFamilyProperties(tvk->gpu, &queue_count, queues_family);

	// get the first queue which supports both compute and graphics
	printf("TUVOK INFO: Avaliable queue families\n");
	uint32_t queue_index;
	for (uint32_t i = 0u; i < queue_count; ++i){
		if ((queues_family[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
		    (queues_family[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
		    (queues_family[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
		{
			printf("\tGraphics, Compute and Transfer: %u\n", queues_family[i].queueCount);
			queue_index = i;
		}
		else if (queues_family[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			printf("\tGraphics Only: %u\n", queues_family[i].queueCount);
		else if (queues_family[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			printf("\tCompute Only: %u\n", queues_family[i].queueCount);
		else if (queues_family[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			printf("\tTransfer Only: %u\n", queues_family[i].queueCount);
	}
	free(queues_family);

	// --------------------------------------------------
	// create out logical device representing our gpu 
	// and using the queue we select to send commands
	// --------------------------------------------------
	VkDeviceQueueCreateInfo vk_qci = {};
	vk_qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vk_qci.queueFamilyIndex = queue_index;
	vk_qci.queueCount = 1;
	float priority = 1.0f; // means 100% priority (we use only 1 queue so doesn't matter)
	vk_qci.pQueuePriorities = &priority;

	VkDeviceCreateInfo vk_dci = {};
	vk_dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vk_dci.queueCreateInfoCount = 1;
	vk_dci.pQueueCreateInfos = &vk_qci;
	vk_dci.pEnabledFeatures = &curr_vk_df;

	if(vkCreateDevice(tvk->gpu, &vk_dci, NULL, &tvk->device) != VK_SUCCESS)
	{
		free_lib(tvk);
		fprintf(stderr, "TUVOK ERROR: Failed to create vulkan logical device!\n");
		return NULL;	
	}

	// get the queue associated with our device
	vkGetDeviceQueue(tvk->device, queue_index, 0, &tvk->queue);
	
	return tvk;
}

void free_lib(tuvok* tvk)
{
	if (tvk)
	{
		if (tvk->window)
			glfwDestroyWindow(tvk->window);
		

		if (tvk->device)
			vkDestroyDevice(tvk->device, NULL);
#if USE_VALIDATION_LAYERS
		if (vk_destroy_debug_ext) vk_destroy_debug_ext(tvk->vk_instance, tvk->vk_debug, NULL);
#endif
		if (tvk->vk_instance)
			vkDestroyInstance(tvk->vk_instance, NULL);
		glfwTerminate();
	}

	free(tvk);
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
		default: {}break;
	};

	// should always return false (specs restriction)
	return VK_FALSE;
}
#endif
