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
