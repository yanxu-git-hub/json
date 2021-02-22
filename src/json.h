#pragma once
#include <memory>
#include <string>
#include"jsonValue.h"

using namespace std;
namespace yanxujson {
	class Json {
	friend bool operator==(const Json& lhs, const Json& rhs);
	friend bool operator!=(const Json& lhs, const Json& rhs);

	public:
		Json();
		~Json();
		Json(const Json& rhs);
		Json operator=(const Json& rhs);
		Json(Json&& rhs);
		Json operator=(Json&& rhs);
		
		void swap(Json& rhs);
		
		void parse(const string& content, string& status);
		void parse(const std::string& content);
		void stringify(string& content) const;

		int get_type() const;
		void set_null();
		void set_boolean(bool);
		double get_number() const;
		void set_number(double);

		const string get_string() const;
		void set_string(const string& str);
		
		void array_init();
		size_t array_get_size() const;
		Json array_get_element(size_t index) const;
		void array_pushback_element(const Json& val);
		void array_popback_element();
		void array_insert_element(const Json& val, size_t index);
		void array_erase_element(size_t index, size_t count);
		void array_clear();

		void object_init();
		size_t object_get_size() const;
		const string& object_get_key(size_t index) const;
		size_t object_get_key_length(size_t index) const;
		Json object_get_value(size_t index) const;
		//Json object_get_value(const string& key) const;
		void object_set_value(const string& key, const Json& val);
		long long object_find_index(const string& key) const;
		void object_remove_value(size_t index);
		void object_clear();



	private:
		unique_ptr<json::Value> v;
	};
	bool operator==(const Json& lhs, const Json& rhs);
	bool operator!=(const Json& lhs, const Json& rhs);
	void swap(Json& lhs, Json& rhs);
}