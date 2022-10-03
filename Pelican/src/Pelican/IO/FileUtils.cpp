#include "PelicanPCH.h"
#include "FileUtils.h"

#include <string>
#include <fstream>

namespace Pelican
{
	namespace IO
	{
		bool ReadFileSync(const std::string& path, std::string& outputBuf)
		{
			std::ifstream file(path, std::ios::binary | std::ios::in);
			if (!file)
				return false;

			outputBuf.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
			file.close();

			return true;
		}

		bool WriteFileSync(const std::string& path, const std::string& contents, FileWriteMode mode)
		{
			std::ofstream file(path,
				mode == FileWriteMode::Append
				? std::ios::binary | std::ios::out | std::ios::app
				: std::ios::binary | std::ios::out);

			if (!file)
				return false;

			file << contents;
			file.close();

			return true;
		}
	}
}
