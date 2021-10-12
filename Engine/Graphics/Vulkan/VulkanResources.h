#pragma once
#include "VulkanCommonHeaders.h"
#include<fstream>

namespace Havana::Graphics::Vulkan
{
	static Utils::vector<char> ReadFile(const std::string& fileName)
	{
		// Open stream from given file at the end to get file size
		std::ifstream file(fileName, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open a file!");
		}

		size_t fileSize = (size_t)file.tellg();
		Utils::vector<char> fileBuffer(fileSize);

		// Go back to begining of file
		file.seekg(0);

		// Read file into our buffer
		file.read(fileBuffer.data(), fileSize);

		// Close the stream
		file.close();

		return fileBuffer;
	}
}