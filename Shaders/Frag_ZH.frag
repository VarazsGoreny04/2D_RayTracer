#version 430

uniform vec3 color;

vec2 textureCoord;

in vec3 vs_out_pos;
in vec3 vs_out_norm;
in vec2 vs_out_tex;

out vec4 fs_out_col;

void main()
{
	fs_out_col = vec4(color, 1);
}
