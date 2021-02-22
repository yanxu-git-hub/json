#include <iostream>
#include <assert.h>
#include <string>
#include "jsonParser.h"
#include "jsonValue.h"
#include "jsonThrowError.h"
using namespace std;
namespace json {
	Value Value::operator=(const Value& rhs) 
	{
		free();
		assign(rhs);
		return *this;
	}

	Value::~Value() 
	{
		free();
	}
	int Value::get_type() const 
	{
		return type_;
	}
	void Value::set_type(json_value_type t) 
	{
		free();
		type_ = t;
	}


	double Value::get_number() const 
	{
		assert(type_ == json::Number);
		return num_;
	}
	void Value::set_number(double digit) 
	{
		free();
		type_ = json::Number;
		num_ = digit;

	}


	const string& Value::get_string() const
	{
		assert(type_ == json::String);
		return str_;
	}
	void Value::set_string(const string& str)
	{
		if (type_ == json::String)
			str_ = str;
		else {
			free();
			type_ = json::String;
			new(&str_) string(str);
			//str_ = str;   与union有关，union只允许一个值初始化。
		}
	}


	size_t Value::array_get_size() const
	{
		assert(type_ == json::Array);
		return arr_.size();
	}
	const Value& Value::array_get_element(size_t index) const
	{
		assert(type_ == json::Array);
		return arr_[index];
	}
	void Value::array_assign(const vector<Value>& arr)
	{
		if (type_ == json::Array)
			arr_ = arr;
		else {
			free();
			type_ = json::Array;
			new(&arr_) std::vector<Value>(arr);
		}
	}
	void Value::array_pushback_element(const Value& val)
	{
		assert(type_ == json::Array);
		arr_.push_back(val);
	}
	void Value::array_popback_element()
	{
		assert(type_ == json::Array);
		arr_.pop_back();
	}
	void Value::array_insert_element(const Value& val, size_t index)
	{
		assert(type_ == json::Array);
		arr_.insert(arr_.begin() + index, val);
	}
	void Value::array_erase_element(size_t index, size_t count)
	{
		assert(type_ == json::Array);
		arr_.erase(arr_.begin() + index, arr_.begin() + index + count);
	}
	void Value::array_clear()
	{
		assert(type_ == json::Array);
		arr_.clear();
	}
	
	size_t Value::object_get_size() const 
	{
		assert(type_ == json::Object);
		return obj_.size();
	}
	const string& Value::object_get_key(size_t index) const
	{
		assert(type_ == json::Object);
		return obj_[index].first;
	}
	size_t Value::object_get_key_length(size_t index) const
	{
		assert(type_ == json::Object);
		return obj_[index].first.size();
	}
	const Value& Value::object_get_value(size_t index) const
	{
		assert(type_ == json::Object);
		return obj_[index].second;
	}
	long long Value::object_find_index(const string& key) const
	{
		assert(type_ == json::Object);
		for (int i = 0; i < obj_.size(); ++i) {
			if (obj_[i].first == key)
				return i;
		}
		return -1;
	}

	/*
	const Value& Value::object_get_value(const string& key) const
	{
		assert(type_ == json::Object);
		int i;
		for (i = 0; i < obj_.size(); ++i) {
			if (obj_[i].first == key) return obj_[i].second;
		}
		
		if(i==obj_.size()) {
			throw(ThrowError("INVILD KEY"));
		}
		
	}
	*/
	void Value::object_set_value(const string& key, const Value& val)
	{
		assert(type_ == json::Object);
		auto index = object_find_index(key);
		//cout << "index" << index << endl;
		//cout << "obj_.size();" << obj_.size() << endl;
		//if (index >= 0) obj_[index].second = val;
		if (index >= 0&&index< obj_.size()) obj_[index].second = val;
		else obj_.push_back(make_pair(key, val));
	}
	void Value::object_assign(const vector<pair<string, Value>>& obj)
	{
		if (type_ == json::Object)
			obj_ = obj;
		else {
			free();
			type_ = json::Object;
			new(&obj_) vector<pair<string, Value>>(obj);
		}
	}
	void Value::object_remove_value(size_t index)
	{
		assert(type_ == json::Object);
		obj_.erase(obj_.begin() + index, obj_.begin() + index + 1);
	}
	void Value::object_clear()
	{
		assert(type_ == json::Object);
		obj_.clear();
	}
	

	

	void Value::parse(const string& content)
	{
		Parser(*this, content);
	}

	void Value::stringify(string& content) const
	{
		Stringify(*this, content);
	}

	void Value::assign(const Value& rhs) 
	{
		type_ = rhs.type_;
		num_ = 0;
		switch (type_) {
		case json::Number: num_ = rhs.num_;
			break;
		case json::String: new(&str_) string(rhs.str_);
			break;
		case json::Array:  new(&arr_) vector<Value>(rhs.arr_);
			break;
		case json::Object:  new(&obj_) vector<pair<string, Value>>(rhs.obj_);
			break;
		}

	}
	void Value::free() 
	{
		switch (type_) {
		case json::String: str_.~string();
			break;
		case json::Array:  arr_.~vector<Value>();
			break;
		case json::Object: obj_.~vector<pair<string, Value>>();
			break;
		}
	}

	bool operator==(const Value& lhs, const Value& rhs)
	{
		if (lhs.type_ != rhs.type_)
			return false;
		switch (lhs.type_) {
		case json::String: return lhs.str_ == rhs.str_;
		case json::Number: return lhs.num_ == rhs.num_;
		case json::Array:  return lhs.arr_ == rhs.arr_;
		case json::Object:
			if (lhs.object_get_size() != rhs.object_get_size())
				return false;
			for (size_t i = 0; i < lhs.object_get_size(); ++i) {
				auto index = rhs.object_find_index(lhs.object_get_key(i));
				if (index < 0 || lhs.object_get_value(i) != rhs.object_get_value(index)) return false;
			}
			return true;
		}
		return true;
	}
	bool operator!=(const Value& lhs, const Value& rhs)
	{
		return !(lhs == rhs);
	}

}