#pragma once
#include <string>
#include <stdexcept>

using namespace std;
namespace json {
	class ThrowError : public logic_error 
	{
	public:
		ThrowError(const string& errmsg) :logic_error(errmsg) {}
	};
}