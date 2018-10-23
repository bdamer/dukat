#include "stdafx.h"
#include <dukat/dds.h>
#include <dukat/log.h>
#include <dukat/sysutil.h>
#include <dukat/texturecache.h>

// Formats
#define GL_COMPRESSED_RGBA_S3TC_DXT1 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5 0x83F3

//  DDS_header.dwFlags
#define DDSD_CAPS                   0x00000001 
#define DDSD_HEIGHT                 0x00000002 
#define DDSD_WIDTH                  0x00000004 
#define DDSD_PITCH                  0x00000008 
#define DDSD_PIXELFORMAT            0x00001000 
#define DDSD_MIPMAPCOUNT            0x00020000 
#define DDSD_LINEARSIZE             0x00080000 
#define DDSD_DEPTH                  0x00800000 

//  DDS_header.sPixelFormat.dwFlags
#define DDPF_ALPHAPIXELS            0x00000001 
#define DDPF_FOURCC                 0x00000004 
#define DDPF_INDEXED                0x00000020 
#define DDPF_RGB                    0x00000040 

//  DDS_header.sCaps.dwCaps1
#define DDSCAPS_COMPLEX             0x00000008 
#define DDSCAPS_TEXTURE             0x00001000 
#define DDSCAPS_MIPMAP              0x00400000 

//  DDS_header.sCaps.dwCaps2
#define DDSCAPS2_CUBEMAP            0x00000200 
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400 
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800 
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000 
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000 
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000 
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000 
#define DDSCAPS2_VOLUME             0x00200000 

constexpr uint32_t D3DFMT_DXT1 = dukat::mc_const('1', 'T', 'X', 'D');	//  DXT1 compression texture format
constexpr uint32_t D3DFMT_DXT2 = dukat::mc_const('2', 'T', 'X', 'D');	//  DXT2 compression texture format
constexpr uint32_t D3DFMT_DXT3 = dukat::mc_const('3', 'T', 'X', 'D');	//  DXT3 compression texture format 
constexpr uint32_t D3DFMT_DXT4 = dukat::mc_const('4', 'T', 'X', 'D');	//  DXT4 compression texture format
constexpr uint32_t D3DFMT_DXT5 = dukat::mc_const('5', 'T', 'X', 'D');	//  DXT5 compression texture format
constexpr uint32_t D3DFMT_DX10 = dukat::mc_const('D', 'X', '1', '0');	//  Extended DX10 header

#define D3D

#define PF_IS_DXT1(pf) \
  ((pf.flags & DDPF_FOURCC) && \
   (pf.four_cc == D3DFMT_DXT1))

#define PF_IS_DXT3(pf) \
  ((pf.flags & DDPF_FOURCC) && \
   (pf.four_cc == D3DFMT_DXT3))

#define PF_IS_DXT5(pf) \
  ((pf.flags & DDPF_FOURCC) && \
   (pf.four_cc == D3DFMT_DXT5))

#define PF_IS_BGRA8(pf) \
  ((pf.flags & DDPF_RGB) && \
   (pf.flags & DDPF_ALPHAPIXELS) && \
   (pf.rgb_bit_count == 32) && \
   (pf.r_bit_mask == 0xff0000) && \
   (pf.g_bit_mask == 0xff00) && \
   (pf.b_bit_mask == 0xff) && \
   (pf.a_bit_mask == 0xff000000U))

#define PF_IS_BGR8(pf) \
  ((pf.flags & DDPF_ALPHAPIXELS) && \
  !(pf.flags & DDPF_ALPHAPIXELS) && \
   (pf.rgb_bit_count == 24) && \
   (pf.r_bit_mask == 0xff0000) && \
   (pf.g_bit_mask == 0xff00) && \
   (pf.b_bit_mask == 0xff))

#define PF_IS_BGR5A1(pf) \
  ((pf.flags & DDPF_RGB) && \
   (pf.flags & DDPF_ALPHAPIXELS) && \
   (pf.rgb_bit_count == 16) && \
   (pf.r_bit_mask == 0x00007c00) && \
   (pf.g_bit_mask == 0x000003e0) && \
   (pf.b_bit_mask == 0x0000001f) && \
   (pf.a_bit_mask == 0x00008000))

#define PF_IS_BGR565(pf) \
  ((pf.flags & DDPF_RGB) && \
  !(pf.flags & DDPF_ALPHAPIXELS) && \
   (pf.rgb_bit_count == 16) && \
   (pf.r_bit_mask == 0x0000f800) && \
   (pf.g_bit_mask == 0x000007e0) && \
   (pf.b_bit_mask == 0x0000001f))

#define PF_IS_INDEX8(pf) \
  ((pf.flags & DDPF_INDEXED) && \
   (pf.rgb_bit_count == 8))


namespace dukat
{
	struct DDSPixelFormat
	{
		uint32_t size;
		uint32_t flags;
		uint32_t four_cc;
		uint32_t rgb_bit_count;
		uint32_t r_bit_mask;
		uint32_t g_bit_mask;
		uint32_t b_bit_mask;
		uint32_t a_bit_mask;
	};

	struct DDSHeader
	{
		uint32_t size;
		uint32_t flags;
		uint32_t height;
		uint32_t width;
		uint32_t pitch_or_linear_size;
		uint32_t depth;
		uint32_t mip_map_count;
		uint32_t reserved[11];
		DDSPixelFormat ddspf;
		uint32_t caps;
		uint32_t caps2;
		uint32_t caps3;
		uint32_t caps4;
		uint32_t reserved2;
	};

	struct DDSLoadInfo {
		bool compressed;
		bool swap;
		bool palette;
		unsigned int div_size;
		unsigned int block_bytes;
		GLenum internal_format;
		GLenum external_format;
		GLenum type;
	};

	const DDSLoadInfo load_info_dxt1 = { true, false, false, 4, 8, GL_COMPRESSED_RGBA_S3TC_DXT1 };
	const DDSLoadInfo load_info_dxt3 = { true, false, false, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT3 };
	const DDSLoadInfo load_info_dxt5 = { true, false, false, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT5 };
	const DDSLoadInfo load_Info_bgra8 = { false, false, false, 1, 4, GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE };
	const DDSLoadInfo load_Info_bgr8 = { false, false, false, 1, 3, GL_RGB8, GL_BGR, GL_UNSIGNED_BYTE };
	const DDSLoadInfo load_info_bgr5a1 = { false, true, false, 1, 2, GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV };
	const DDSLoadInfo load_info_bgr565 = { false, true, false, 1, 2, GL_RGB5, GL_RGB, GL_UNSIGNED_SHORT_5_6_5 };
	const DDSLoadInfo load_info_index8 = { false, false, true, 1, 1, GL_RGB8, GL_BGRA, GL_UNSIGNED_BYTE };

	bool load_dds_header(std::istream& is, DDSHeader& header)
	{
		// Check format
		char magic[5] = { 0 };
		is.read(reinterpret_cast<char*>(&magic), sizeof(char) * 4);
		if (strcmp(magic, "DDS ") != 0)
		{
			log->warn("Failed to load texture - not a DDS file.");
			return false;
		}

		// Load header
		is.read(reinterpret_cast<char*>(&header.size), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.flags), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.height), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.width), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.pitch_or_linear_size), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.depth), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.mip_map_count), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.reserved), sizeof(uint32_t) * 11);
		is.read(reinterpret_cast<char*>(&header.ddspf.size), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.ddspf.flags), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.ddspf.four_cc), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.ddspf.rgb_bit_count), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.ddspf.r_bit_mask), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.ddspf.g_bit_mask), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.ddspf.b_bit_mask), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.ddspf.a_bit_mask), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.caps), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.caps2), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.caps3), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.caps4), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&header.reserved2), sizeof(uint32_t));

		// Check for DX10
		if (header.ddspf.flags == DDPF_FOURCC && header.ddspf.four_cc == D3DFMT_DX10)
		{
			log->warn("Failed to load texture - unsupported version.");
			return false;
		}

		return true;
	}

	void load_compressed_dds(std::istream& is, const DDSHeader& header, const DDSLoadInfo* li, const GLenum target)
	{
		uint32_t x = header.width;
		uint32_t y = header.height;
		uint32_t mip_map_count = (header.flags & DDSD_MIPMAPCOUNT) ? header.mip_map_count : 1;
		size_t size = std::max(li->div_size, x) / li->div_size * std::max(li->div_size, y) / li->div_size * li->block_bytes;
		uint8_t* data = (uint8_t*)malloc(size);
		for (unsigned int ix = 0; ix < mip_map_count; ++ix)
		{
			is.read(reinterpret_cast<char*>(data), size);
			glCompressedTexImage2D(target, ix, li->internal_format, x, y, 0, (int)size, data);
			x = (x + 1) >> 1;
			y = (y + 1) >> 1;
			size = std::max(li->div_size, x) / li->div_size * std::max(li->div_size, y) / li->div_size * li->block_bytes;
		}
		free(data);
	}

	void load_indexed_dds(std::istream& is, const DDSHeader& header, const DDSLoadInfo* li, const GLenum target)
	{
		uint32_t x = header.width;
		uint32_t y = header.height;
		uint32_t mip_map_count = (header.flags & DDSD_MIPMAPCOUNT) ? header.mip_map_count : 1;
		size_t size = header.pitch_or_linear_size * y;
		uint8_t* data = (uint8_t*)malloc(size);
		unsigned int palette[256];
		unsigned int* unpacked = (unsigned int*)malloc(size*sizeof(unsigned int));
		is.read(reinterpret_cast<char*>(palette), sizeof(int) * 256);
		for (unsigned int ix = 0; ix < mip_map_count; ++ix)
		{
			is.read(reinterpret_cast<char*>(data), size);
			for (unsigned int zz = 0; zz < size; ++zz)
			{
				unpacked[zz] = palette[data[zz]];
			}
			glPixelStorei(GL_UNPACK_ROW_LENGTH, y);
			glTexImage2D(target, ix, li->internal_format, x, y, 0, li->external_format, li->type, unpacked);
			x = (x + 1) >> 1;
			y = (y + 1) >> 1;
			size = x * y * li->block_bytes;
		}
		free(data);
		free(unpacked);
	}

	void load_default_dds(std::istream& is, const DDSHeader& header, const DDSLoadInfo* li, const GLenum target)
	{
		uint32_t x = header.width;
		uint32_t y = header.height;
		uint32_t mip_map_count = (header.flags & DDSD_MIPMAPCOUNT) ? header.mip_map_count : 1;
		if (li->swap)
		{
			glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
		}
		size_t size = x * y * li->block_bytes;
		uint8_t* data = (uint8_t*)malloc(size);
		for (unsigned int ix = 0; ix < mip_map_count; ++ix)
		{
			is.read(reinterpret_cast<char*>(data), size);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, y);
			glTexImage2D(target, ix, li->internal_format, x, y, 0, li->external_format, li->type, data);
			x = (x + 1) >> 1;
			y = (y + 1) >> 1;
			size = x * y * li->block_bytes;
		}
		free(data);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	}

	std::unique_ptr<Texture> load_dds(const std::string& filename)
	{
		std::fstream fs(filename, std::fstream::in | std::fstream::binary);
		if (!fs)
		{
			throw std::runtime_error("Could not open file: " + filename);
		}
		return load_dds(fs);
	}

	std::unique_ptr<Texture> load_dds(std::istream& is)
	{
		DDSHeader header;
		if (!load_dds_header(is, header))
		{
			return nullptr;
		}

		const DDSLoadInfo* li;
		if (PF_IS_DXT1(header.ddspf))
		{
			li = &load_info_dxt1;
		}
		else if (PF_IS_DXT3(header.ddspf))
		{
			li = &load_info_dxt3;
		}
		else if (PF_IS_DXT5(header.ddspf))
		{
			li = &load_info_dxt5;
		}
		else if (PF_IS_BGRA8(header.ddspf))
		{
			li = &load_Info_bgra8;
		}
		else if (PF_IS_BGR8(header.ddspf))
		{
			li = &load_Info_bgr8;
		}
		else if (PF_IS_BGR5A1(header.ddspf))
		{
			li = &load_info_bgr5a1;
		}
		else if (PF_IS_BGR565(header.ddspf))
		{
			li = &load_info_bgr565;
		}
		else if (PF_IS_INDEX8(header.ddspf))
		{
			li = &load_info_index8;
		}
		else
		{
			log->warn("Invalid pixel format.");
			return nullptr;
		}

		void (*loader)(std::istream&, const DDSHeader&, const DDSLoadInfo*, const GLenum);
		if (li->compressed)
		{
			loader = &load_compressed_dds;
		}
		else if (li->palette)
		{
			loader = &load_indexed_dds;
		}
		else
		{
			loader = &load_default_dds;
		}

		// Create OpenGL texture
		auto texture = std::make_unique<Texture>(header.width, header.height);

		if ((header.caps & DDSCAPS_COMPLEX) == DDSCAPS_COMPLEX)
		{
			if ((header.caps2 & DDSCAPS2_CUBEMAP) == DDSCAPS2_CUBEMAP)
			{
				texture->target = GL_TEXTURE_CUBE_MAP;
				glBindTexture(GL_TEXTURE_CUBE_MAP, texture->id);
			
				// Set texture parameters
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0); 
				auto mip_map_count = (header.flags & DDSD_MIPMAPCOUNT) ? header.mip_map_count : 1;
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mip_map_count - 1);
				
				if (header.mip_map_count > 0)
				{
#if OPENGL_VERSION < 30	
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_FALSE);
#endif
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				}
				else
				{
#if OPENGL_VERSION < 30
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
#endif
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);					
				}
				
				loader(is, header, li, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
				loader(is, header, li, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
				loader(is, header, li, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
				loader(is, header, li, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
				loader(is, header, li, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
				loader(is, header, li, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
								
#if OPENGL_VERSION >= 30
				// TODO: Review - when do we want to generate mip maps for cube maps
				/*if (header.mip_map_count == 0)
				{
					glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 1);
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				}*/
#endif
				
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			}
			else
			{
				throw std::runtime_error("Unable to load DDS.");
			}
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, texture->id);
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
			loader(is, header, li, GL_TEXTURE_2D);

			// Set texture parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		return texture;
	}
}