//---------------------------------------------------------------------------

#include <opengl/shader.h>
#include <opengl/shaders/embedded.h>
#include <opengl/uniform.h>

#include <iostream>

//---------------------------------------------------------------------------

namespace asd
{
    namespace opengl
    {
        namespace shader_code
        {
            store & get(const std::string & name) {
                return shaders.at(name);
            }
        }

        shader_program::shader_program(context & ctx, const shader_code::store & code) : layout(code.layout), id(glCreateProgram()) {
            GLint status = GL_FALSE;
            int info_log_length = 0;

            asd::array_list<uint> ids(code.units.size);

            auto & uniforms = static_cast<uniform_component &>(get<uniform::component>(ctx));

            for(auto & unit : code.units) {
                auto shader_id = glCreateShader(unit.type);
                ids.push_back(shader_id);

                glShaderSource(shader_id, 1, reinterpret_cast<const GLchar * const *>(&unit.code), NULL);
                glCompileShader(shader_id);

                glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);

                if(status == GL_FALSE) {
#ifdef GL_DEBUG
                    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length);

                    if(info_log_length != 0) {
                        owned_data<char> buffer(static_cast<size_t>(info_log_length + 1));

                        glGetShaderInfoLog(id, info_log_length, nullptr, buffer.ptr);
                        buffer[info_log_length] = '\0';

                        std::cout << buffer.ptr << std::endl;
                    }
#endif

                    throw std::runtime_error("Can't compile GLSL shader!");
                }

                glAttachShader(id, shader_id);
            }

            uint k = 0;

            for(auto & e : code.layout.elements) {
                glBindAttribLocation(id, k, e.name);
                k += (e.units - 1) / 4 + 1;
            }

            glLinkProgram(id);

            glGetProgramiv(id, GL_LINK_STATUS, &status);

            if(status == GL_FALSE) {
#ifdef GL_DEBUG
                glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_length);

                if(info_log_length != 0) {
                    owned_data<char> buffer(static_cast<size_t>(info_log_length + 1));

                    glGetProgramInfoLog(id, info_log_length, nullptr, buffer.ptr);
                    buffer[info_log_length] = '\0';

                    std::cout << buffer.ptr << std::endl;
                }
#endif

                throw std::runtime_error("Can't compile GLSL shader!");
            }

            GLint blocksCount = 0;

            glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCKS, &blocksCount);

            GLint length;

            for (GLint i = 0; i < blocksCount; i++) {
                glGetActiveUniformBlockiv(id, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &length);
                asd::array_list<GLchar> name(length, '\0');
                glGetActiveUniformBlockName(id, i, length, NULL, &name[0]);

                auto index = uniforms.buffer_index(name.data());
                glUniformBlockBinding(id, i, index);
            }

            for(auto & shader_id : ids) {
                glDeleteShader(shader_id);
            }

            glUseProgram(0);
        }

        shader_program::shader_program(shader_program && program) throw() : layout(program.layout), id(program.id) {
            program.id = 0;
        }

        shader_program::~shader_program() {
            if(id != 0) {
                glDeleteProgram(id);
            }
        }

        void shader_program::apply() const {
            glUseProgram(id);
        }
    }
}

//---------------------------------------------------------------------------
