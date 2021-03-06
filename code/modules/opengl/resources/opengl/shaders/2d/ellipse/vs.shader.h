//---------------------------------------------------------------------------

#include <opengl/vertex_layout.h>

//---------------------------------------------------------------------------

static const char * const shader_code_2d_ellipse_vs = R"SHADER(
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

layout(std140) uniform Viewport
{
	vec2 viewport;
};

in vec2 position;
in vec2 texcoord;

out Vertex 
{
	vec2 texcoord;
	vec4 ratio;
} output;

void main(void)
{
	output.ratio.w = output.ratio.z = max(size.x, size.y);
	output.ratio.xy = output.ratio.zw / size;
	output.texcoord = texcoord;

	gl_Position = vec4((position * output.ratio.zw + pos) * vec2(1.0, viewport.x / viewport.y), depth, 1.0);
}

)SHADER";

//---------------------------------------------------------------------------

static const ::asd::opengl::vertex_layout & shader_code_2d_ellipse_layout = ::asd::opengl::vertex_layouts::p2t::get();

//---------------------------------------------------------------------------
