#pragma once
#include <string>
#include <vector>
#include <utility>

using namespace std;

namespace json {
	enum json_value_type : int {
		Null,
		True,
		False,
		Number,
		String,
		Array,
		Object
	};
	class Value {
		friend bool operator == (const Value& lhs, const Value& rhs);
		friend bool operator != (const Value& lhs, const Value& rhs);

	public:
		Value(){}
		Value(const Value& rhs)  { assign(rhs); }
		Value operator=(const Value& rhs);
		~Value();

		int get_type() const ;
		void set_type(json_value_type t);

		double get_number() const;
		void set_number(double);

		const string& get_string() const;
		void set_string(const string& str);

		size_t array_get_size() const;
		const Value& array_get_element(size_t index) const;
		void array_assign(const vector<Value>& arr);
		void array_pushback_element(const Value& val);
		void array_popback_element();
		void array_insert_element(const Value& val, size_t index);
		void array_erase_element(size_t index, size_t count);
		void array_clear();

		
		size_t object_get_size() const;
		const string& object_get_key(size_t index) const;
		size_t object_get_key_length(size_t index) const;
		const Value& object_get_value(size_t index) const;
		long long object_find_index(const string& key) const;
		//const Value& object_get_value(const string& key) const;
		void object_set_value(const string& key, const Value& val);
		void object_assign(const vector<pair<string, Value>>& obj);
		void object_remove_value(size_t index);
		void object_clear();

		void parse(const string& content);
		void stringify(string& content) const;



	private:
		void assign(const Value& rhs) ;
		void free() ;
		json_value_type type_;

		union {
			double num_;
			string str_;
			vector<Value> arr_;
			vector<pair<string, Value>> obj_;
		};
	};
	bool operator==(const Value& lhs, const Value& rhs);
	bool operator!=(const Value& lhs, const Value& rhs);
}