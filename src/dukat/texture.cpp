#include "stdafx.h"
#include "texture.h"
#include "surface.h"
#include "shaderprogram.h"
#include "sysutil.h"

namespace dukat
{
	Texture::Texture(void) : target(GL_TEXTURE_2D), profile(ProfileNearest), w(0), h(0)
	{
		glGenTextures(1, &id);
	}

	Texture::Texture(TextureId id) : target(GL_TEXTURE_2D), id(id), profile(ProfileNearest), w(0), h(0)
	{
	}

	Texture::Texture(int w, int h, TextureFilterProfile profile) : target(GL_TEXTURE_2D), profile(profile), w(w), h(h)
	{
		glGenTextures(1, &id);
	}

	Texture::Texture(const Surface& surface, TextureFilterProfile profile) : target(GL_TEXTURE_2D), id(0)
	{
		w = surface.get_width();
		h = surface.get_height();
		load_data(surface, profile);
	}

	Texture::~Texture(void)
	{ 
		if (id != 0) 
		{
			glDeleteTextures(1, &id); 
		}
	}

	void Texture::load_data(const Surface& surface, TextureFilterProfile profile)
	{
		GLenum format = 0, type = 0;
		surface.query_pixel_format(format, type);

		// Handle surface that is smaller than texture
		auto min_width = std::min(w, surface.get_width());
		auto min_height = std::min(h, surface.get_height());

		load_data(GL_RGBA, min_width, min_height, surface.get_surface()->pixels, format, type, profile);
	}

	void Texture::load_data(int src_fmt, int src_width, int src_height, const GLvoid* src_data, GLenum format, GLenum type, TextureFilterProfile profile)
	{
		if (id == 0)
		{
			glGenTextures(1, &id);
		}
		glBindTexture(target, id);

		bool generate_map = false;
		switch (profile)
		{
		case ProfileMipMapped:
#if OPENGL_VERSION < 30
			glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
#else
			generate_map = true;
#endif
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case ProfileAnisotropic:
#ifdef OPENGL_CORE // only supported via extension on ES
#if OPENGL_VERSION < 30
			glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
#else
			generate_map = true;
#endif
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			GLfloat float_val;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &float_val);
			glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, float_val);
			break;
#endif
		case ProfileLinear:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case ProfileNearest:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		}

		glTexImage2D(target, 0, src_fmt, src_width, src_height, 0, format, type, src_data);

		if (generate_map)
		{
#if OPENGL_VERSION >= 30
			glGenerateMipmap(target);
#endif
		}

#ifdef _DEBUG
		glBindTexture(target, 0);
		gl_check_error();
#endif
	}

	void Texture::bind(GLenum texture, ShaderProgram* program) const
	{
		static std::string program_id = "u_tex0";
		glActiveTexture(GL_TEXTURE0 + texture);
		glBindTexture(target, id);
		if (program != nullptr)
		{
			program_id[5] = static_cast<char>(48 + texture);
			glUniform1i(program->attr(program_id), texture);
		}
	}

	void Texture::unbind(void) const
	{
		glBindTexture(target, 0);
	}

	GLint Texture::get_internal_format(void) const
	{
	    // Should be supported by ES31+, but fails...
#ifdef OPENGL_CORE
        bind(0);
		GLint format;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
		unbind();
		return format;
#else
		return 0;
#endif
	}
}