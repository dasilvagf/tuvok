#include <stdio.h>
#include <tuvok.h>

int main (int argc, char** argv)
{
    // init our lib
	tuvok* lib_core = init_lib(800, 600, "tuvok lib - triangle sample");

    // load shaders
    tuvok_shader* vs = load_shader(lib_core, "triangle_vs.spv", "main", 1u, 
            VK_SHADER_STAGE_VERTEX_BIT);
    tuvok_shader* fs = load_shader(lib_core, "triangle_fs.spv", "main", 1u,
            VK_SHADER_STAGE_FRAGMENT_BIT);

    // create the render pipeline
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

    tuvok_pipeline* pipe = create_pipeline(lib_core, pipe_desc);
    if (!pipe)
    {
        free_shader(lib_core, vs);
        free_shader(lib_core, fs);
        free_lib(lib_core);
        return 0x0;
    }

	// render loop
	while(!glfwWindowShouldClose(lib_core->window))
	{
		glfwPollEvents();
	}

    // cleanup
    free_shader(lib_core, vs);
    free_shader(lib_core, fs);
	free_lib(lib_core);
	return 0x0;
}
