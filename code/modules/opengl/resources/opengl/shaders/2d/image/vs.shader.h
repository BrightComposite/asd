//---------------------------------------------------------------------------

#include <opengl/vertex_layout.h>

//---------------------------------------------------------------------------

static const char * const shader_code_2d_image_vs = R"SHADER(
/**
 *	!vertex: p2 t
 */
#version 330 core

layout(std140) uniform Area
{
	vec2  pos;
	vec2  size;
	float depth;
};

layout(std140) uniform ImageScale
{
	vec2 scale;
};

in vec2 position;
in vec2 texcoord;

out Vertex 
{
	vec2 texcoord;
} output;

void main(void)
{
	output.texcoord = texcoord * scale;
    gl_Position = vec4(position * size + pos, depth, 1.0);
}

)SHADER";

//---------------------------------------------------------------------------

static const ::asd::opengl::vertex_layout & shader_code_2d_image_layout = ::asd::opengl::vertex_layouts::p2t::get();

//---------------------------------------------------------------------------
