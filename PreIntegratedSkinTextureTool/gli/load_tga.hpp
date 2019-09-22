/// @brief Include to load TGA textures from files or memory.
/// @file gli/load_tga.hpp

#pragma once

#include "texture.hpp"

namespace gli
{
	/// Loads a texture storage_linear from TGA file. Returns an empty storage_linear in case of failure.
	///
	/// @param Path Path of the file to open including filaname and filename extension
	texture load_tga(char const* Path);

	/// Loads a texture storage_linear from TGA file. Returns an empty storage_linear in case of failure.
	///
	/// @param Path Path of the file to open including filaname and filename extension
	texture load_tga(std::string const& Path);

	/// Loads a texture storage_linear from TGA memory. Returns an empty storage_linear in case of failure.
	///
	/// @param Data Pointer to the beginning of the texture container data to read
	/// @param Size Size of texture container Data to read
	texture load_tga(char const* Data, std::size_t Size);
}//namespace gli

#include "./core/load_tga.inl"
