#pragma once

#include <fstream>

namespace Pelican
{
	namespace IO
	{
		enum class FileWriteMode
		{
			Overwrite,
			Append
		};

		// Reads a file to a string.
		bool ReadFileSync(const std::string& path, std::string& outputBuf);

		// Writes a string to a file. Default write mode is overwrite.
		bool WriteFileSync(const std::string& path, const std::string& contents, FileWriteMode mode = FileWriteMode::Overwrite);
	}
}
