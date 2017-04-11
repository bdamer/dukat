#include "stdafx.h"
#include "texture.h"
#include "surface.h"
#include "shaderprogram.h"
#include "dukat.h"

namespace dukat
{
	Texture::Texture(void) : id(0), w(0), h(0), profile(ProfileNearest), target(GL_TEXTURE_2D)
	{ 
	}

	Texture::Texture(int w, int h, TextureFilterProfile profile) : w(w), h(h), profile(profile), target(GL_TEXTURE_2D)
	{
		glGenTextures(1, &id);
	}

	Texture::Texture(const Surface& surface, TextureFilterProfile profile) : id(0), target(GL_TEXTURE_2D)
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
		case ProfileLinear:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case ProfileNearest:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		}
		
		glTexImage2D(target, 0, GL_RGBA, w, h, 0, format, type, surface.get_surface()->pixels);

		if (generate_map)
		{
#if OPENGL_VERSION >= 30
			glGenerateMipmap(target);
#endif
		}

#ifdef _DEBUG
		glBindTexture(target, 0);
#endif
	}

	void Texture::bind(GLenum texture, ShaderProgram* program)
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

	void Texture::unbind(void)
	{
		glBindTexture(target, 0);
	}
}