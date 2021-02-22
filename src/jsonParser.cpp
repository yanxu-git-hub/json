#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include "jsonThrowError.h"
#include "jsonParser.h"
#include "jsonValue.h"
#include <iostream>

using namespace std;

#define expect(context, ch)\
	do{\
		assert(*context == ch);\
		++context;\
	}while(0)
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')


namespace json {

	Parser::Parser(Value& val, const string& content) : val_(val), cur_(content.c_str()) {
		{
			val_.set_type(json::Null);
			parse_whitespace();
			parse_value();
			parse_whitespace();
			if (*cur_ != '\0') {
				val_.set_type(json::Null);
				throw(ThrowError("LEPT_PARSE_ROOT_NOT_SINGULAR"));
			}
		}
	}

	void Parser::parse_whitespace(){
		while (*cur_ == ' ' || *cur_ == '\t' || *cur_ == '\n' || *cur_ == '\r')
			++cur_;
	}

	void Parser::parse_literal(const char* literal, json_value_type t) {
		expect(cur_, literal[0]);
		size_t i;
		for (i = 0; literal[i + 1]; ++i) {
			if (cur_[i] != literal[i + 1])
				throw (ThrowError("LEPT_PARSE_INVALID_VALUE"));
		}
		cur_ += i;
		val_.set_type(t);
	}

	void Parser::parse_number() {

		const char* p = cur_;
		if (*p == '-') ++p;
		if (*p == '0') ++p;
		else {
			if (!ISDIGIT1TO9(*p)) throw (ThrowError("LEPT_PARSE_INVALID_VALUE"));
			for (p++; isdigit(*p); p++);
		}
		if (*p == '.') {
			if (!isdigit(*++p)) throw (ThrowError("LEPT_PARSE_INVALID_VALUE"));
			for (p++; isdigit(*p); p++);
		}
		if (*p == 'e' || *p == 'E') {
			++p;
			if (*p == '+' || *p == '-') ++p;
			if (!isdigit(*p)) throw (ThrowError("LEPT_PARSE_INVALID_VALUE"));
			for (p++; isdigit(*p); p++);
		}
		errno = 0;
		double v = strtod(cur_, NULL);
		if (errno == ERANGE && (v == HUGE_VAL || v == -HUGE_VAL))
			throw (ThrowError("LEPT_PARSE_NUMBER_TOO_BIG"));
		val_.set_number(v);
		cur_ = p;

	}


	void Parser::parse_string()
	{
		string s;
		parse_string_raw(s);
		val_.set_string(s);
	}

	void Parser::parse_string_raw(string& tmp)
	{
		expect(cur_, '\"');
		const char* p = cur_;
		unsigned u = 0, u2 = 0;
		while (*p != '\"') {
			if (*p == '\0')
				throw(ThrowError("LEPT_PARSE_MISS_QUOTATION_MARK"));
			if (*p == '\\' && ++p) {
				switch (*p++) {
				case '\"': tmp += '\"'; break;
				case '\\': tmp += '\\'; break;
				case '/': tmp += '/'; break;
				case 'b': tmp += '\b'; break;
				case 'f': tmp += '\f'; break;
				case 'n': tmp += '\n'; break;
				case 'r': tmp += '\r'; break;
				case 't': tmp += '\t'; break;
				case 'u':
					parse_hex4(p, u);
					if (u >= 0xD800 && u <= 0xDBFF) {
						if (*p++ != '\\')
							throw(ThrowError("LEPT_PARSE_INVALID_UNICODE_SURROGATE"));
						if (*p++ != 'u')
							throw(ThrowError("LEPT_PARSE_INVALID_UNICODE_SURROGATE"));
						parse_hex4(p, u2);
						if (u2 < 0xDC00 || u2 > 0xDFFF)
							throw(ThrowError("LEPT_PARSE_INVALID_UNICODE_SURROGATE"));
						u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
					}
					parse_encode_utf8(tmp, u);
					break;
				default: throw (ThrowError("LEPT_PARSE_INVALID_STRING_ESCAPE"));
				}
			}
			
			else if ((unsigned char)*p < 0x20) {
				throw (ThrowError("LEPT_PARSE_INVALID_STRING_CHAR"));
			}
			else tmp += *p++;
		}
		cur_ = ++p;
	}

	void Parser::parse_hex4(const char*& p, unsigned& u)
	{
		u = 0;
		for (int i = 0; i < 4; ++i) {
			char ch = *p++;
			u <<= 4;
			if (isdigit(ch))
				u |= ch - '0';
			else if (ch >= 'A' && ch <= 'F')
				u |= ch - ('A' - 10);
			else if (ch >= 'a' && ch <= 'f')
				u |= ch - ('a' - 10);
			else throw(ThrowError("LEPT_PARSE_INVALID_UNICODE_HEX"));
		}
	}

	void Parser::parse_encode_utf8(string& str, unsigned u) const
	{
		if (u <= 0x7F)
			str += static_cast<char> (u & 0xFF);
		else if (u <= 0x7FF) {
			str += static_cast<char> (0xC0 | ((u >> 6) & 0xFF));
			str += static_cast<char> (0x80 | (u & 0x3F));
		}
		else if (u <= 0xFFFF) {
			str += static_cast<char> (0xE0 | ((u >> 12) & 0xFF));
			str += static_cast<char> (0x80 | ((u >> 6) & 0x3F));
			str += static_cast<char> (0x80 | (u & 0x3F));
		}
		else {
			assert(u <= 0x10FFFF);
			str += static_cast<char> (0xF0 | ((u >> 18) & 0xFF));
			str += static_cast<char> (0x80 | ((u >> 12) & 0x3F));
			str += static_cast<char> (0x80 | ((u >> 6) & 0x3F));
			str += static_cast<char> (0x80 | (u & 0x3F));
		}
	}


	void Parser::parse_array()
	{
		expect(cur_, '[');
		parse_whitespace();
		vector<Value> tmp;
		if (*cur_ == ']') {
			++cur_;
			val_.array_assign(tmp);
			return;
		}
		for (;;) {
			try {
				parse_value();
			}
			catch (ThrowError) {
				val_.set_type(json::Null);
				throw;
			}
			tmp.push_back(val_);
			parse_whitespace();
			if (*cur_ == ',') {
				++cur_;
				parse_whitespace();
			}
			else if (*cur_ == ']') {
				++cur_;
				val_.array_assign(tmp);
				return;
			}
			else {
				val_.set_type(json::Null);
				throw(ThrowError("LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET"));
			}
		}
	}
	
	void Parser::parse_object()
	{
		expect(cur_, '{');
		parse_whitespace();
		vector<pair<string, Value>> tmp;
		string key;
		if (*cur_ == '}') {
			++cur_;
			val_.object_assign(tmp);
			return;
		}
		for (;;) {
			if (*cur_ != '\"') throw(ThrowError("LEPT_PARSE_MISS_KEY"));
			try {
				parse_string_raw(key);
			}
			catch (ThrowError) {
				throw(ThrowError("LEPT_PARSE_MISS_KEY"));
			}
			parse_whitespace();
			if (*cur_++ != ':') throw(ThrowError("LEPT_PARSE_MISS_COLON"));
			parse_whitespace();
			try {
				parse_value();
			}
			catch (ThrowError) {
				val_.set_type(json::Null);
				throw;
			}
			tmp.push_back(make_pair(key, val_));
			val_.set_type(json::Null);
			key.clear();
			parse_whitespace();
			if (*cur_ == ',') {
				++cur_;
				parse_whitespace();
			}
			else if (*cur_ == '}') {
				++cur_;
				val_.object_assign(tmp);
				return;
			}
			else {
				val_.set_type(json::Null);
				throw(ThrowError("LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET"));
			}
		}
	}
	


	void Parser::parse_value() {
		switch (*cur_) {
		case 'n': parse_literal("null", json::Null);  return;
		case 't': parse_literal("true", json::True);  return;
		case 'f': parse_literal("false", json::False); return;
		case '\"': parse_string(); return;
		case '[': parse_array();  return;
		case '\0': throw(ThrowError("LEPT_PARSE_EXPECT_VALUE"));
		case '{': parse_object(); return;
		default:parse_number(); return;
		}
	}



	Stringify::Stringify(const Value& val, string& content)
		: content_(content)
	{
		content_.clear();
		stringify_value(val);
	}

	void Stringify::stringify_value(const Value& val)
	{
		switch (val.get_type()) {
		case json::Null:  content_ += "null";  break;
		case json::True:  content_ += "true";  break;
		case json::False: content_ += "false"; break;
		case json::Number: {
			char buffer[32] = { 0 };
			sprintf_s(buffer, "%.17g", val.get_number());
			content_ += buffer;
		}
		break;
		case json::String:stringify_string(val.get_string());
			break;
		case json::Array:
			content_ += '[';
			for (int i = 0; i < val.array_get_size(); ++i) {
				if (i > 0) content_ += ',';
				stringify_value(val.array_get_element(i));
			}
			content_ += ']';
			break;
		case json::Object:
			content_ += '{';
			for (int i = 0; i < val.object_get_size(); ++i) {
				if (i > 0) content_ += ',';
				stringify_string(val.object_get_key(i));
				content_ += ':';
				stringify_value(val.object_get_value(i));
			}
			content_ += '}';
			break;
		}
	}
	void Stringify::stringify_string(const std::string& str)
	{
		content_ += '\"';
		for (auto it = str.begin(); it < str.end(); ++it) {
			unsigned char ch = *it;
			switch (ch) {
			case '\"': content_ += "\\\""; break;
			case '\\': content_ += "\\\\"; break;
			case '\b': content_ += "\\b";  break;
			case '\f': content_ += "\\f";  break;
			case '\n': content_ += "\\n";  break;
			case '\r': content_ += "\\r";  break;
			case '\t': content_ += "\\t";  break;
			default:
				if (ch < 0x20) {
					char buffer[7] = { 0 };
					sprintf_s(buffer, "\\u%04X", ch);
					content_ += buffer;
				}
				else
					content_ += *it;
			}
		}
		content_ += '\"';
	}



}