#include <stdio.h>
#include "include/tuvok.h"

int main (int argc, char* argv)
{
	tuvok* lib_core = init_lib(800, 600, "tuvok lib - sample");

	// render loop
	while(!glfwWindowShouldClose(lib_core->window))
	{
		glfwPollEvents();

	}

	free_lib(lib_core);
	return 0x0;
}
