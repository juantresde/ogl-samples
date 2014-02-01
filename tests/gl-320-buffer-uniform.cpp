///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "test.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("gl-320/buffer-uniform.vert");
	char const * FRAG_SHADER_SOURCE("gl-320/buffer-uniform.frag");

	GLsizei const VertexCount(4);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionData[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f)
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2,
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MATERIAL,
			MAX
		};
	}//namespace buffer

	namespace shader
	{
		enum type
		{
			VERT,
			FRAG,
			MAX
		};
	}//namespace shader
}//namespace

class gl_320_buffer_uniform : public test
{
public:
	gl_320_buffer_uniform(int argc, char* argv[]) :
		test(argc, argv, "gl-320-buffer-uniform", test::CORE, 3, 2),
		VertexArrayName(0),
		ProgramName(0),
		UniformTransform(0),
		UniformMaterial(0)
	{}

private:
	std::array<GLuint, shader::MAX> ShaderName;
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint ProgramName;
	GLuint VertexArrayName;
	GLint UniformTransform;
	GLint UniformMaterial;

	bool initProgram()
	{
		bool Validated = true;

		glf::compiler Compiler;
	
		// Create program
		if(Validated)
		{
			ShaderName[shader::VERT] = Compiler.create(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERT_SHADER_SOURCE, "--version 150 --profile core");
			ShaderName[shader::FRAG] = Compiler.create(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE, "--version 150 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, ShaderName[shader::VERT]);
			glAttachShader(ProgramName, ShaderName[shader::FRAG]);
			glBindAttribLocation(ProgramName, glf::semantic::attr::POSITION, "Position");
			glBindFragDataLocation(ProgramName, glf::semantic::frag::COLOR, "Color");

			glLinkProgram(ProgramName);
			Validated = Validated && glf::checkProgram(ProgramName);
		}

		// Get variables locations
		if(Validated)
		{
			UniformMaterial = glGetUniformBlockIndex(ProgramName, "material");
			UniformTransform = glGetUniformBlockIndex(ProgramName, "transform");

			glUniformBlockBinding(ProgramName, UniformTransform, glf::semantic::uniform::TRANSFORM0);
			glUniformBlockBinding(ProgramName, UniformMaterial, glf::semantic::uniform::MATERIAL);
		}

		GLint ActiveUniformBlocks(0);
		glGetProgramiv(ProgramName, GL_ACTIVE_UNIFORM_BLOCKS, &ActiveUniformBlocks);

		for(GLint i = 0; i < ActiveUniformBlocks; ++i)
		{
			char Name[128];
			memset(Name, '\0', sizeof(Name));
			GLsizei Length(0);

			glGetActiveUniformBlockName(ProgramName, i, GLsizei(sizeof(Name)), &Length, Name);

			std::string StringName(Name);

			Validated = Validated && (StringName == std::string("material") || StringName == std::string("transform"));
		}

		GLint ActiveUniform(0);
		glGetProgramiv(ProgramName, GL_ACTIVE_UNIFORMS, &ActiveUniform);

		for(GLint i = 0; i < ActiveUniformBlocks; ++i)
		{
			char Name[128];
			memset(Name, '\0', sizeof(Name));
			GLsizei Length(0);

			glGetActiveUniformName(ProgramName, i, GLsizei(sizeof(Name)), &Length, Name);

			std::string StringName(Name);

			Validated = Validated && (
				StringName == std::string("material.Diffuse") || 
				StringName == std::string("transform.MVP"));
		}
	
		return Validated && glf::checkError("initProgram");
	}

	bool initVertexArray()
	{
		// Build a vertex array object
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(glf::semantic::attr::POSITION);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return glf::checkError("initVertexArray");
	}

	bool initBuffer()
	{
		// Generate buffer objects
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBlockSize = 0;

		{
			glGetActiveUniformBlockiv(ProgramName, UniformTransform, GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSize);

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		{
			glm::vec4 Diffuse(1.0f, 0.5f, 0.0f, 1.0f);

			glGetActiveUniformBlockiv(
				ProgramName, 
				UniformMaterial,
				GL_UNIFORM_BLOCK_DATA_SIZE,
				&UniformBlockSize);

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::MATERIAL]);
			glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, &Diffuse[0], GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		return glf::checkError("initBuffer");
	}

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();

		return Validated && glf::checkError("begin");
	}

	bool end()
	{
		for(std::size_t i = 0; 0 < shader::MAX; ++i)
			glDeleteShader(ShaderName[i]);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
				GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
			glm::mat4 MVP = Projection * this->view() * Model;
		
			*Pointer = MVP;
		
			// Make sure the uniform buffer is uploaded
			glUnmapBuffer(GL_UNIFORM_BUFFER);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		glViewport(0, 0, static_cast<GLsizei>(WindowSize.x), static_cast<GLsizei>(WindowSize.y));
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		glUseProgram(ProgramName);

		// Attach the buffer to UBO binding point glf::semantic::uniform::TRANSFORM0
		glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		// Attach the buffer to UBO binding point glf::semantic::uniform::MATERIAL
		glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::MATERIAL, BufferName[buffer::MATERIAL]);

		glBindVertexArray(VertexArrayName);
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 1, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_buffer_uniform Test(argc, argv);
	Error += Test();

	return Error;
}
