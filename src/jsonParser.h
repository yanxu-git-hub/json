#pragma once
#include "jsonValue.h"

using namespace std;
namespace json {
	class Parser {
	public:
		Parser(Value& val, const string& content);
	private:
		void parse_whitespace();
		void parse_value();
		void parse_literal(const char* literal, json_value_type t);

		void parse_number();

		void parse_string();
		void parse_string_raw(string& tmp);
		void parse_hex4(const char*& p, unsigned& u);
		void parse_encode_utf8(string& s, unsigned u) const;

		void parse_array();

		void parse_object();

		Value &val_;
		const char* cur_;
	};

	class Stringify {
	public:
		Stringify(const Value& val, string& content);
	private:
		void stringify_value(const Value& val);
		void stringify_string(const string& str);

		std::string& content_;

	};
}

