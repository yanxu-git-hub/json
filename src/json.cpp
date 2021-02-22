/*
size_t array_get_size() const;
		Json array_get_element(size_t index) const;
		void array_assign();
		void array_pushback_element(const Json& val);
		void array_popback_element();
		void array_insert_element(const Json& val, size_t index);
		void array_erase_element(size_t index, size_t count);
		void array_clear();*/
#include "json.h"
#include "jsonValue.h"
#include "jsonThrowError.h"
using namespace std;
namespace yanxujson {

	bool operator==(const Json& lhs, const Json& rhs)
	{
		return *lhs.v == *rhs.v;
	}
	bool operator!=(const Json& lhs, const Json& rhs)
	{
		return *lhs.v != *rhs.v;
	}
	
	
	Json::Json() : v(new json::Value) { }
	Json::~Json() { }
	Json::Json(const Json& rhs)
	{
		v.reset(new json::Value(*(rhs.v)));
	}
	Json Json::operator=(const Json& rhs)
	{
		v.reset(new json::Value(*(rhs.v)));
		return *this;
	}
	Json::Json(Json&& rhs)
	{
		v.reset(rhs.v.release());
	}
	Json Json::operator=(Json&& rhs)
	{
		v.reset(rhs.v.release());
		return *this;
	}

	void Json::swap(Json& rhs)
	{
		using std::swap;
		swap(v, rhs.v);
	}
	void swap(Json& lhs, Json& rhs)
	{
		lhs.swap(rhs);
	}

	void Json::parse(const std::string& content, std::string& status)
	{
		try {
			parse(content);
			status = "LEPT_PARSE_OK";
		}
		catch (const json::ThrowError& msg) {
			status = msg.what();
		}
		catch (...) {
		}
	}

	void Json::parse(const string& content)
	{
		v->parse(content);
	}


	void Json::stringify(string& content) const
	{
		v->stringify(content);
	}


	int Json::get_type() const
	{
		if (v == nullptr)
			return json::Null;
		return v->get_type();
	}

	void Json::set_null()
	{
		v->set_type(json::Null);
	}
	void Json::set_boolean(bool b)
	{
		if (b) v->set_type(json::True);
		else v->set_type(json::False);
	}

	double Json::get_number() const
	{
		return v->get_number();
	}
	void Json::set_number(double digit)
	{
		return v->set_number(digit);
	}

	const string Json::get_string() const
	{
		return v->get_string();
	}
	void Json::set_string(const std::string& str)
	{
		v->set_string(str);
	}

	size_t Json::array_get_size() const
	{
		return v->array_get_size();
	}

	Json Json::array_get_element(size_t index) const
	{
		Json ret;
		ret.v.reset(new json::Value(v->array_get_element(index)));
		return ret;
	}

	void Json::array_init()
	{
		v->array_assign(vector<json::Value>{});
	}

	void Json::array_pushback_element(const Json& val)
	{
		v->array_pushback_element(*val.v);
	}
	void Json::array_popback_element()
	{
		v->array_popback_element();
	}
	void Json::array_insert_element(const Json& val, size_t index)
	{
		v->array_insert_element(*val.v, index);
	}
	void Json::array_erase_element(size_t index, size_t count)
	{
		v->array_erase_element(index, count);
	}
	void Json::array_clear() 
	{
		v->array_clear();
	}


	void Json::object_init()
	{
		v->object_assign(vector<pair<string, json::Value>>{});
	}
	size_t Json::object_get_size() const
	{
		return v->object_get_size();
	}
	const string& Json::object_get_key(size_t index) const
	{
		return v->object_get_key(index);
	}
	size_t Json::object_get_key_length(size_t index) const
	{
		return v->object_get_key_length(index);
	}
	Json Json::object_get_value(size_t index) const
	{
		Json ret;
		ret.v.reset(new json::Value(v->object_get_value(index)));
		return ret;
	}

	/*
	Json Json::object_get_value(const string& key) const
	{
		Json ret;
		ret.v.reset(new json::Value(v->object_get_value(key)));
		return ret;
	}
	*/

	void Json::object_set_value(const string& key, const Json& val)
	{
		v->object_set_value(key, *val.v);
	}
	long long Json::object_find_index(const string& key) const
	{
		return v->object_find_index(key);
	}
	void Json::object_remove_value(size_t index)
	{
		v->object_remove_value(index);
	}
	void Json::object_clear() 
	{
		v->object_clear();
	}
}