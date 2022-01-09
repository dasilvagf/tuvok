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
	glfwSetErrorCallback(GLFW_error_function);

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
	// window context acquisition
	// --------------------------------------------------
	if (glfwCreateWindowSurface(tvk->vk_instance, tvk->window, NULL, &tvk->vk_context))
	{
		free_lib(tvk);
		fprintf(stderr, "TUVOK ERROR: Failed to create vulkan context!\n");
		return NULL;	
	}
	
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
			printf("\t\tGPU type: Integrated\n");
		else if (vk_dp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			printf("\t\tGPU type: Discrete\n");
		else
			printf("\t\tGPU type: Other\n");
	}

	VkPhysicalDeviceFeatures curr_vk_df = {};
	// get the first gpu that support what we want
	for (uint32_t i = 0u; i < gpus_count; ++i){
		VkPhysicalDeviceFeatures vk_df = {};
		vkGetPhysicalDeviceFeatures(vulkan_gpus[i], &vk_df);

		// check the features we want
		if (vk_df.geometryShader)
		{
			curr_vk_df = vk_df;
			tvk->gpu = vulkan_gpus[i];
			
            VkPhysicalDeviceProperties vk_dp = {};
            vkGetPhysicalDeviceProperties(vulkan_gpus[i], &vk_dp);
            strcpy(tvk->gpu_name, vk_dp.deviceName);
            break;
		}
	}
	free(vulkan_gpus);

    printf("TUVOK INFO: Selected GPU: %s \n", tvk->gpu_name);

	// --------------------------------------------------
	//    check if the GPU (physical device) support
	//    the extensions we want
	// --------------------------------------------------
	
	// put the extensions names here
	uint32_t n_ext = 1u;
	char** extensions = (char**) malloc(sizeof(char*)*n_ext); 
	extensions[0] = "VK_KHR_swapchain"; // swap-chain support
	extensions[1] = "VK_KHR_acceleration_structure"; // ray-tracing support

	// get the number of available extensions
	uint32_t n_avaliable_ext; 
	vkEnumerateDeviceExtensionProperties(tvk->gpu, NULL, &n_avaliable_ext, NULL);
	
	// get the availiable extensions
	VkExtensionProperties* avaliable_ext = (VkExtensionProperties*) 
		malloc(sizeof(VkExtensionProperties)*n_avaliable_ext);
	vkEnumerateDeviceExtensionProperties(tvk->gpu, NULL, &n_avaliable_ext, 
			avaliable_ext);

	// check if the extensions we want are present
	uint32_t* found_count = (uint32_t*) malloc(sizeof(uint32_t)*n_ext);
	memset(found_count, 0u, sizeof(uint32_t)*n_ext);

	for (uint32_t i = 0u; i < n_avaliable_ext; ++i){
		for (uint32_t j = 0u; j < n_ext; ++j)
			if (strcmp(extensions[j], avaliable_ext[i].extensionName) == 0)
				found_count[j] = 1u;
	}
	free(avaliable_ext);
	
	for (uint32_t j = 0u; j < n_ext; ++j)
		if (found_count[j] == 0u)
		{
			free(found_count);
			free_lib(tvk);
			fprintf(stderr, "TUVOK ERROR: Some of the required extensions are not avaliable!\n");
			return NULL;	
		}

	free(found_count);

	// --------------------------------------------------
	// get the Queue we'll use to submit our commands
	// --------------------------------------------------

	// no need to check as all vulkan gpus support at least one queue
	uint32_t queue_count;
	vkGetPhysicalDeviceQueueFamilyProperties(tvk->gpu, &queue_count, NULL);

	VkQueueFamilyProperties* queues_family = (VkQueueFamilyProperties*) 
		malloc(sizeof(VkQueueFamilyProperties)*queue_count);
	vkGetPhysicalDeviceQueueFamilyProperties(tvk->gpu, &queue_count, queues_family);

	// get a queue for rendering, compute and presentation
	// in case a queue support both 3 operations select this one 	
	printf("TUVOK INFO: Avaliable queue families\n");
	uint32_t queue_index_rendering;
	uint32_t queue_index_compute;
	uint32_t queue_index_presentation;
	
	for (uint32_t i = 0u; i < queue_count; ++i){
		if ((queues_family[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
		    (queues_family[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
		    (queues_family[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
		{
			printf("\tGraphics & Compute: %u\n", queues_family[i].queueCount);
			queue_index_rendering = i;
			queue_index_compute = i;
		}
		else if ((queues_family[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			 (queues_family[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
		{
			queue_index_rendering = i;
			printf("\tGraphics Only: %u\n", queues_family[i].queueCount);
		}
		else if (queues_family[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			queue_index_compute = i;
			printf("\tCompute Only: %u\n", queues_family[i].queueCount);
		}

		VkBool32 support_presentation;
		vkGetPhysicalDeviceSurfaceSupportKHR(tvk->gpu, i, tvk->vk_context,
				&support_presentation);
		if (support_presentation)
			queue_index_presentation = i;
	}
	free(queues_family);

	// check if one queue supports both rendering, compute and presentation
	if ((queue_index_rendering == queue_index_compute) && 
	    (queue_index_compute == queue_index_presentation))
		tvk->use_common_queue = 1u;

	// --------------------------------------------------
	// create our logical device representing our gpu 
	// and using the queue we select to send commands
	// --------------------------------------------------
	if (/*tvk->use_common_queue FOR NOW WE USE ONE QUEUE ANYWAYS*/ 1u)
	{
		VkDeviceQueueCreateInfo vk_qci = {};
		vk_qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		vk_qci.queueFamilyIndex = queue_index_rendering;
		vk_qci.queueCount = 1;
		float priority = 1.0f; // means 100% priority (we use only 1 queue so doesn't matter)
		vk_qci.pQueuePriorities = &priority;

		VkDeviceCreateInfo vk_dci = {};
		vk_dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		vk_dci.queueCreateInfoCount = 1;
		vk_dci.pQueueCreateInfos = &vk_qci;
		vk_dci.pEnabledFeatures = &curr_vk_df;
		vk_dci.enabledExtensionCount = n_ext;
		vk_dci.ppEnabledExtensionNames = extensions;

		if(vkCreateDevice(tvk->gpu, &vk_dci, NULL, &tvk->device) != VK_SUCCESS)
		{
			free_lib(tvk);
			fprintf(stderr, "TUVOK ERROR: Failed to create vulkan logical device!\n");
			return NULL;	
		}

		// get the queue associated with our device
		vkGetDeviceQueue(tvk->device, queue_index_rendering, 0, &tvk->common_queue);
	}
	else
	{
		//TODO: handle a seperated queue for presentation
		//TODO: handle a separated queue for asynchronous compute
		assert(0);
		
        free_lib(tvk);
		return NULL;	
	}
	free(extensions);

	// --------------------------------------------------
	//    create the swap chain for double-buffering
	// --------------------------------------------------
	
	// get the capabilities of the surface (context)
	VkSurfaceCapabilitiesKHR surf_caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(tvk->gpu, tvk->vk_context, &surf_caps);

	// get the surface formats that support presentation
	uint32_t n_surfaces = 0u;
	vkGetPhysicalDeviceSurfaceFormatsKHR(tvk->gpu, tvk->vk_context, &n_surfaces, NULL);
	if (!n_surfaces)
	{
		free_lib(tvk);
		fprintf(stderr, "TUVOK TODO ERROR: This device DO NOT support presentation!\n");
		return NULL;	
	}

	VkSurfaceFormatKHR* surfaces = (VkSurfaceFormatKHR*) 
		malloc(sizeof(VkSurfaceFormatKHR)*n_surfaces);
	vkGetPhysicalDeviceSurfaceFormatsKHR(tvk->gpu, tvk->vk_context, &n_surfaces, surfaces);

	// select the best avaliable surface for presentation
	VkSurfaceFormatKHR swap_surf = surfaces[0];
	for (uint32_t i = 0u; i < n_surfaces; ++i){
		if (surfaces[i].format == VK_FORMAT_B8G8R8A8_SRGB && 
			surfaces[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
		      swap_surf = surfaces[i];	
		      printf("TUVOK INFO: Using B8G8R8A8_SRGB SwapChain\n");
		}
	}
	free(surfaces);


	// get the presentation modes
	uint32_t n_presents = 0u;
	vkGetPhysicalDeviceSurfacePresentModesKHR(tvk->gpu, tvk->vk_context, &n_presents, NULL);
	
	VkPresentModeKHR* p_modes = (VkPresentModeKHR*)
		malloc(sizeof(VkPresentModeKHR)*n_presents);
	vkGetPhysicalDeviceSurfacePresentModesKHR(tvk->gpu, tvk->vk_context, &n_presents, p_modes);

	// check support for triple-buffering, if not availible fall down to double-buffering
	VkPresentModeKHR presentation_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0u; i < n_presents; ++i){
		if (p_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentation_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			printf("TUVOK INFO: Triple Buffering is supported!\n");
		}
	}
	free(p_modes);

	// set resolution of the swap-chain buffers
	glfwGetFramebufferSize(tvk->window, &tvk->window_width, &tvk->window_height);
	VkExtent2D vk_extent = {tvk->window_width, tvk->window_height};
	
	// clamp dimensions to the supported values
	vk_extent.width = (tvk->window_width < surf_caps.minImageExtent.width) ?
		surf_caps.minImageExtent.width : vk_extent.width;
	vk_extent.width = (tvk->window_width > surf_caps.maxImageExtent.width) ?
		surf_caps.maxImageExtent.width : vk_extent.width;

	vk_extent.height = (tvk->window_height < surf_caps.minImageExtent.height) ?
		surf_caps.minImageExtent.height : vk_extent.height;
	vk_extent.height = (tvk->window_height > surf_caps.maxImageExtent.height) ?
		surf_caps.maxImageExtent.height : vk_extent.height;

    // see how many images my swap-chain can hold
    uint32_t image_count = surf_caps.minImageCount; 
    if (surf_caps.maxImageCount > 0 && image_count > surf_caps.maxImageCount)
        image_count = surf_caps.maxImageCount;

    // finally create the swap-chain
    VkSwapchainCreateInfoKHR swp_cb = {};
    swp_cb.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swp_cb.surface = tvk->vk_context;
    swp_cb.minImageCount = image_count;
    swp_cb.imageFormat = swap_surf.format;
    swp_cb.imageColorSpace = swap_surf.colorSpace;
    swp_cb.imageExtent = vk_extent;
    swp_cb.imageArrayLayers = 1;
    swp_cb.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swp_cb.preTransform = surf_caps.currentTransform;
    swp_cb.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swp_cb.presentMode = presentation_mode;
    swp_cb.clipped = VK_TRUE;
    swp_cb.oldSwapchain = VK_NULL_HANDLE;

    // presentation and rendering use the same queue
    if (/*tvk->use_common_queue ASSUME WE USING COMOM QUEUES*/ 1u)
        swp_cb.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    else
    {
        //TODO: handle swap-chain presentation when I have separate queues
        // for presentation and rendering
		assert(0);
    }

    if (vkCreateSwapchainKHR(tvk->device, &swp_cb, NULL, &tvk->swap_chain) 
            != VK_SUCCESS)
    {
        free_lib(tvk);
		fprintf(stderr, "TUVOK ERROR: Swap-Chain Creation Falied!\n");
		return NULL;	
    }

    // set swap chain info for future use
    tvk->swap_fmt = swap_surf.format;
    tvk->swap_extent = vk_extent;

    // acquire the surfaces of the swap-chain
    vkGetSwapchainImagesKHR(tvk->device, tvk->swap_chain, &tvk->n_swap_images,
            NULL);
    tvk->swap_images = (VkImage*) malloc(sizeof(VkImage)*tvk->n_swap_images);
    tvk->swap_views  = (VkImageView*) 
        malloc(sizeof(VkImageView)*tvk->n_swap_images);

    vkGetSwapchainImagesKHR(tvk->device, tvk->swap_chain, &tvk->n_swap_images,
            tvk->swap_images);
 
    // create the image views to interpret the swap chain images
    VkImageViewCreateInfo vk_vcif = {};
    vk_vcif.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vk_vcif.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vk_vcif.format = tvk->swap_fmt;
    vk_vcif.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    vk_vcif.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    vk_vcif.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    vk_vcif.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    vk_vcif.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_vcif.subresourceRange.baseMipLevel = 0;
    vk_vcif.subresourceRange.levelCount = 1;
    vk_vcif.subresourceRange.baseArrayLayer = 0;
    vk_vcif.subresourceRange.layerCount = 1;

    for (uint32_t i = 0u; i < tvk->n_swap_images; ++i){
        vk_vcif.image = tvk->swap_images[i];
        if (vkCreateImageView(tvk->device, &vk_vcif, NULL, &tvk->swap_views[i])
                != VK_SUCCESS)
        {
            free_lib(tvk);
            fprintf(stderr, "TUVOK ERROR: Swap-Chain ImageViews Creation Falied!\n");
            return NULL;	
        }
    }

    return tvk;
}

void free_lib(tuvok* tvk)
{
	if (tvk)
	{
        printf("TUVOK INFO: Exiting...!\n");
		
        if (tvk->window)
			glfwDestroyWindow(tvk->window);
		
        if (tvk->swap_chain)
            vkDestroySwapchainKHR(tvk->device, tvk->swap_chain, NULL);

        if (tvk->swap_images)
            free(tvk->swap_images);

        if (tvk->swap_views)
        {
            for (uint32_t i = 0u; i < tvk->n_swap_images; ++i)
                vkDestroyImageView(tvk->device, tvk->swap_views[i], NULL);
            free(tvk->swap_views);
        }

		if (tvk->device)
			vkDestroyDevice(tvk->device, NULL);
#if USE_VALIDATION_LAYERS
		if (vk_destroy_debug_ext) vk_destroy_debug_ext(tvk->vk_instance, tvk->vk_debug, NULL);
#endif
		if (tvk->vk_context)
			vkDestroySurfaceKHR(tvk->vk_instance, tvk->vk_context, NULL);
		if (tvk->vk_instance)
			vkDestroyInstance(tvk->vk_instance, NULL);
		glfwTerminate();
	}

	free(tvk);
}

tuvok_shader* load_shader(const tuvok* tvk, const char* filename, const char* main_name, 
        uint8_t is_bytecode, VkShaderStageFlagBits stage)
{
    if (!is_bytecode)
        assert(0u);

    FILE* pFile;
    pFile = fopen(filename, "r");

    if (pFile)
    {
         // get file size;
         fseek (pFile , 0 , SEEK_END);
         long lSize = ftell (pFile);
         rewind (pFile);

         // read our bytecode
         char* bytecode_buffer = (char*) malloc(lSize);
         if (fread(bytecode_buffer, sizeof(char)/* a.k.a 1*/, lSize, pFile) != lSize)
         {
             fprintf(stderr, "TUVOK ERROR: Shader bytecode couldn't be loaded!\n");
             free(bytecode_buffer);
             fclose(pFile);
             return NULL;
         }

         // create our shader module from the blob bytecode
         tuvok_shader* shader = (tuvok_shader*) malloc(sizeof(tuvok_shader));
         VkShaderModuleCreateInfo vs_smci = {};
         vs_smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
         vs_smci.codeSize = lSize;
         vs_smci.pCode = (const uint32_t*)bytecode_buffer;

         if (vkCreateShaderModule(tvk->device, &vs_smci, NULL, &shader->shader_module) != VK_SUCCESS)
         {
             fprintf(stderr, "TUVOK ERROR: SPIR-V Shader module couldn't be created!\n");
             free(shader);
             free(bytecode_buffer);
             fclose(pFile);
             return NULL;
         }

         // info to the pipeline on how to read this bytecode
         shader->pipe_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
         shader->pipe_info.module = shader->shader_module;
         shader->pipe_info.stage = stage;
         shader->pipe_info.pName = main_name;

         free(bytecode_buffer);
         fclose(pFile);

         return shader;
    }

    return NULL;
}

void free_shader(const tuvok* tvk, tuvok_shader* shader)
{
    if (shader)
    {
        vkDestroyShaderModule(tvk->device, shader->shader_module, NULL);
        free(shader);
    }
}

// internal function, intended to be only used by create_pipeline
VkPipelineLayout* create_pipeline_layout(const tuvok* tvk)
{
    // JUST CREATE A EMPTY PIPE LAYOUT FOR NOW (TYEMPORARY)
    VkPipelineLayoutCreateInfo li = {};
    li.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkPipelineLayout* layout = (VkPipelineLayout*) malloc(sizeof(VkPipelineLayout));
    if (vkCreatePipelineLayout(tvk->device, &li, NULL, layout) != VK_SUCCESS)
    {
        free(layout);
        fprintf(stderr, "TUVOK ERROR: Pipeline Layout couldn't be created!\n");
        return NULL;
    }
    else
        return layout;
}

tuvok_pipeline* create_pipeline(const tuvok* tvk, tuvok_pipeline_desc desc)
{
    // describe vertex buffer data layout and how it will be pased to the VS
    VkPipelineVertexInputStateCreateInfo vi = {};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount = desc.n_vb_layouts;
    vi.pVertexBindingDescriptions = desc.vb_layouts_array;
    vi.vertexAttributeDescriptionCount = desc.n_vb_attributes;
    vi.pVertexAttributeDescriptions = desc.vb_attributes_array;

    // describe how to interprete the vertices (the infamous input assembler)
    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = desc.ia_topology;
    ia.primitiveRestartEnable = desc.ia_restart_enabled;

    // viewport and scissor rect
    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = desc.n_viewports;
    vp.pViewports = desc.viewports_array;
    vp.scissorCount = desc.n_scissor_rects;
    vp.pScissors = desc.scissor_rect_array;

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rs = {};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.depthClampEnable = desc.depth_clamp_enable;
    rs.rasterizerDiscardEnable = desc.disable_raster_stage;
    rs.polygonMode = desc.polygon_mode;
    rs.lineWidth = desc.line_width;
    rs.cullMode = desc.cull_mode;
    rs.frontFace = desc.front_face_winding;
    rs.depthBiasEnable = desc.depth_bias_enable;
    rs.depthBiasConstantFactor = desc.depth_bias_constant_factor;
    rs.depthBiasClamp = desc.depth_bias_clamp;
    rs.depthBiasSlopeFactor = desc.depth_bias_slope_factor;

    // multisampling
    VkPipelineMultisampleStateCreateInfo ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.sampleShadingEnable = desc.sample_shading_enable;
    ms.rasterizationSamples = desc.samples_per_pixel;
    ms.minSampleShading = desc.min_sample_shading;
    ms.pSampleMask = desc.sample_masks_array;
    ms.alphaToCoverageEnable = desc.alpha_to_coverage_enable;
    ms.alphaToOneEnable = desc.alpha_to_one_enable;

    // depth/stencil
    VkPipelineDepthStencilStateCreateInfo ds = {};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.flags = desc.optional_flags;
    ds.depthTestEnable = desc.depth_test_enable;
    ds.depthWriteEnable = desc.depth_write_enable;
    ds.depthCompareOp = desc.frag_compare_operation;
    ds.depthBoundsTestEnable = desc.depth_bounds_test_enable;
    ds.stencilTestEnable = desc.stencil_test_enable;
    ds.front = desc.stencil_front;
    ds.back = desc.stencil_back;
    ds.minDepthBounds = desc.min_depth_bound;
    ds.maxDepthBounds = desc.max_depth_bound;

    // color blending
    VkPipelineColorBlendStateCreateInfo bs = {};
    bs.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    bs.logicOpEnable = desc.use_logical_ops;
    bs.logicOp = desc.logical_op;
    bs.attachmentCount = desc.n_render_targets;
    bs.pAttachments = desc.color_blending_states;
    bs.blendConstants[0] = desc.blend_const_rgba[0];
    bs.blendConstants[1] = desc.blend_const_rgba[1];
    bs.blendConstants[2] = desc.blend_const_rgba[2];
    bs.blendConstants[3] = desc.blend_const_rgba[3];

    //
    // I'M here
    // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions


    
    // Finally create the pipeline
    tuvok_pipeline* pipe = (tuvok_pipeline*) malloc(sizeof(tuvok_pipeline));

    pipe->layout = create_pipeline_layout(tvk);
    if (pipe->layout == NULL)
    {
        free(pipe);
        return NULL;
    }

    return pipe;
}

void free_pipeline(const tuvok* tvk, tuvok_pipeline* pipe)
{
    if (tvk && pipe)
    {
        // free the pipeline layout
        vkDestroyPipelineLayout(tvk->device, *(pipe->layout), NULL);
        free(pipe->layout);


        free(pipe);
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
		default: {}break;
	};

	// should always return false (specs restriction)
	return VK_FALSE;
}
#endif
