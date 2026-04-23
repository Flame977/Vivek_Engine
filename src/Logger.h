#pragma once
#include<iostream>
#include <sstream>

class Logger
{
public:

	template<typename... Args>
	static void Log(Args&&... args)
	{
		std::ostringstream stream;
		(stream << ... << args);

		std::cout << stream.str() << std::endl;
	}
};

#define Log(...) Logger::Log(__VA_ARGS__)

