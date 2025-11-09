#version 330 core

in vec3 a_vertex;
in vec3 a_normal;
in vec4 a_color;
in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_viewprojection;
uniform vec3 u_camera_position;

// Output to fragment shader
out vec3 v_world_position;

void main()
{
	// Calculate world position (needed for ray computation in fragment shader)
	v_world_position = (u_model * vec4(a_vertex, 1.0)).xyz;
	
	// Calculate screen position
	gl_Position = u_viewprojection * vec4(v_world_position, 1.0);
}

