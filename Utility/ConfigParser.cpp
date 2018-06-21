#include "ConfigParser.h"
#include <regex>
#include <iostream>

namespace Utility
{
	ConfigParser::ConfigParser()
	{
	}


	ConfigParser::~ConfigParser()
	{
		std::regex reg("(buildversion=[0-9]+)");
	}

	DataDefine::FileListData ConfigParser::Load(const std::string& data)
	{
		const auto r = _ParserVersion(data);

		_ParserPathAndMD5(r);

		_Debug();

		return _FileListData;
	}

	std::string ConfigParser::_ParserVersion(const std::string& data)
	{
		const std::regex reg("buildversion=([0-9]+)");

		std::smatch sm;

		if (regex_search(data, sm, reg))
		{
			_FileListData.Version = std::stoi(sm[1].str());
			return sm.suffix().str();
		}
		return "";
	}

	void ConfigParser::_ParserPathAndMD5(const std::string& data)
	{
		//const std::regex reg(R"(([\w\/.]+)\|([0-9a-fA-F]{32}))");
		const std::regex reg(R"(([0-9a-fA-F]{32})\|([\w\\.]+))");

		std::smatch sm;

		auto result{data};

		while (regex_search(result, sm, reg))
		{
			//_FileListData.Contents.emplace(sm[1].str(), sm[2].str());
			result = sm.suffix().str();
		}
	}

	void ConfigParser::_Debug()
	{
		std::cout << std::endl;
		std::cout << "parser result:" << std::endl;
		std::cout << "version = " << _FileListData.Version << std::endl;
		std::cout << "==============contents=========== " << std::endl;
		for (auto& c : _FileListData.Contents)
		{
			std::cout << "md5 = " << c.first << std::endl;
			std::cout << "path = " << c.second.Path << std::endl;
		}
	}
}
