#include <stdio.h>
#include <tuvok.h>

int main (int argc, char** argv)
{
    //
    // init our lib
	//
    tuvok* lib_core = init_lib(800, 600, "tuvok lib - triangle sample");

    //
    // load shaders
    //
    tuvok_shader* vs = load_shader(lib_core, "triangle_vs.spv", "main", 1u, 
            VK_SHADER_STAGE_VERTEX_BIT);
    tuvok_shader* fs = load_shader(lib_core, "triangle_fs.spv", "main", 1u,
            VK_SHADER_STAGE_FRAGMENT_BIT);

    //
    // create the render pipeline
    //
    tuvok_pipeline_desc pipe_desc = {};

    // input assembler 
    pipe_desc.ia_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipe_desc.ia_restart_enabled = VK_FALSE;

    // viewport and scissor rect
    VkViewport vp = {};
    vp.x = 0.0f;
    vp.y = 0.0f;
    vp.width = 800.0f;
    vp.height = 600.0f;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    pipe_desc.n_viewports = 1u;
    pipe_desc.viewports_array = &vp;

    VkRect2D rect = {};
    rect.extent.width = 800.0f;
    rect.extent.height = 600.0f;
    pipe_desc.n_scissor_rects = 1u;
    pipe_desc.scissor_rect_array = &rect;

    // rasterizer
    pipe_desc.depth_clamp_enable = VK_FALSE;
    pipe_desc.disable_raster_stage = VK_FALSE;
    pipe_desc.polygon_mode = VK_POLYGON_MODE_FILL;
    pipe_desc.line_width = 1.0f;
    pipe_desc.cull_mode = VK_CULL_MODE_BACK_BIT;
    pipe_desc.front_face_winding = VK_FRONT_FACE_CLOCKWISE;
    pipe_desc.depth_bias_enable = VK_FALSE;

    // multisampling
    pipe_desc.sample_shading_enable = VK_FALSE; 
    pipe_desc.samples_per_pixel = VK_SAMPLE_COUNT_1_BIT;

    // depth/stencil
    pipe_desc.depth_test_enable = VK_FALSE;
    pipe_desc.stencil_test_enable = VK_FALSE;

    // color blending
    VkPipelineColorBlendAttachmentState blend = {};
    blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blend.blendEnable = VK_FALSE; 

    pipe_desc.n_render_targets = 1u;
    pipe_desc.use_logical_ops = VK_FALSE;
    pipe_desc.color_blending_states = &blend;
 
    tuvok_pipeline* pipe = create_pipeline(lib_core, pipe_desc);
    if (!pipe)
    {
        free_shader(lib_core, vs);
        free_shader(lib_core, fs);
        free_lib(lib_core);
        return 0x0;
    }

    //
    // create render pass
    //

    VkAttachmentDescription att_array = {};
    att_array.format = lib_core->swap_fmt; // format of thw swap buffer
    att_array.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling
    att_array.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear the buffer beforing rendering
    att_array.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // then save the value after rendering
    // ignore stencil
    att_array.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att_array.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // use of the image attached as swap_chain
    att_array.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // we don't know the format before rendering
    att_array.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // will be used as swap chain

    VkAttachmentReference attref_array = {};
    attref_array.attachment = 0u; // we have only one attachament, so we reference the frist one 
    attref_array.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout optmized for color buffers

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // this pass will be for rendering
    subpass.colorAttachmentCount = 1u; // as said before, we have only one attachament in this case
    subpass.pColorAttachments = &attref_array;

    // simple render pass with just a collorbuffer
    tuvok_renderpass_desc renderpass_desc = {};
    renderpass_desc.n_attacments = 1u;
    renderpass_desc.n_render_subpasses = 1u;
    renderpass_desc.attachaments_array = &att_array;
    renderpass_desc.subpass_desc_array = &subpass;

    VkRenderPass* renderpass = create_renderpass(lib_core, renderpass_desc);
    if (!renderpass)
    {
        free_pipeline(lib_core, pipe);
        free_shader(lib_core, vs);
        free_shader(lib_core, fs);
        free_lib(lib_core);
        return 0x0;
    }

    //
	// render loop
	//
    while(!glfwWindowShouldClose(lib_core->window))
	{
		glfwPollEvents();
	}

    //
    // cleanup
    //
    free_renderpass(lib_core, renderpass);
    free_pipeline(lib_core, pipe);
    free_shader(lib_core, vs);
    free_shader(lib_core, fs);
	free_lib(lib_core);
	return 0x0;
}
