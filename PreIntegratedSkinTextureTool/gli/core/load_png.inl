#include "file.hpp"
#include "lodepng.h"
#include <cstdio>
#include <cassert>

namespace gli {
	namespace detail
	{
		inline texture load_png(char const* Data, std::size_t Size)
		{
			LodePNGState state;

			unsigned error;
			unsigned width, height;
			unsigned char* image = NULL;

			lodepng_state_init(&state);

			error = lodepng_decode(&image, &width, &height, &state, (unsigned char *)Data, Size);
			if (error != 0) {
				free(image);
				lodepng_state_cleanup(&state);
				return texture();
			}

			texture Texture(
				TARGET_2D,
				FORMAT_BGRA8_UNORM_PACK8,
				texture::extent_type(std::max<texture::size_type>(width, 1), std::max<texture::size_type>(height, 1), 1),
				1,
				1,
				1);

			std::memcpy(Texture.data(0, 0, 0), image, Texture.size(0));

			free(image);
			lodepng_state_cleanup(&state);

			return Texture;
		}
	}
	//namespace detail

	inline texture load_png(char const* Data, std::size_t Size)
	{
		return detail::load_png(Data, Size);
	}

	inline texture load_png(char const* Filename)
	{
		FILE* File = detail::open_file(Filename, "rb");
		if (!File)
			return texture();

		long Beg = std::ftell(File);
		std::fseek(File, 0, SEEK_END);
		long End = std::ftell(File);
		std::fseek(File, 0, SEEK_SET);

		std::vector<char> Data(static_cast<std::size_t>(End - Beg));

		std::fread(&Data[0], 1, Data.size(), File);
		std::fclose(File);

		return load_png(&Data[0], Data.size());
	}

	inline texture load_png(std::string const& Filename)
	{
		return load_png(Filename.c_str());
	}
}
//namespace gli
