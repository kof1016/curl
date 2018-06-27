#include "FileWriter.h"
//#include <fstream>

namespace Utility
{
	FileWriter::FileWriter(std::string file_name)
	{
		_FileName = file_name;
	}

	FileWriter::~FileWriter()
	{
		_File = nullptr;
	}

	void FileWriter::OpenFile()
	{
		 //std::ofstream outfile("NewestVer.txt");

		_File = std::fopen(_FileName.c_str(), "w");
		if(!_File)
		{
			return;
		}
	}

	void FileWriter::CloseFile() const
	{
		fclose(_File);
	}

	void FileWriter::Write(char* buffer, size_t nmemb) const
	{
		fwrite(buffer, 1, nmemb, _File);
	}

	void FileWriter::Write(const char* buffer, size_t nmemb) const
	{
		fwrite(buffer, 1, nmemb, _File);
	}
}
