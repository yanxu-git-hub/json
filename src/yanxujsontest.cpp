#include <iostream>
#include <string.h>
#include "json.h"
using namespace std;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;
static string status;

#define EXPECT_EQ_BASE(expect, actual) \
	do {\
		++test_count;\
		if (expect == actual)\
			++test_pass;\
		else {\
			cerr << __FILE__ << ":" << __LINE__ << ": expect: " << expect << " actual: " << actual << endl;\
			main_ret = 1;\
		}\
	} while(0)

#define TEST_LITERAL(expect, content)\
	do {\
		yanxujson::Json literal;\
		literal.set_boolean(false);\
		literal.parse(content, status);\
		EXPECT_EQ_BASE("LEPT_PARSE_OK", status);\
		EXPECT_EQ_BASE(expect, literal.get_type());\
	} while (0)

static void test_parse_literal()
{
	TEST_LITERAL(json::Null, "null");
	TEST_LITERAL(json::True, "true");
	TEST_LITERAL(json::False, "false");
}


#define TEST_NUMBER(expect, content)\
	do {\
		yanxujson::Json NUMBER;\
		NUMBER.parse(content, status);\
		EXPECT_EQ_BASE("LEPT_PARSE_OK", status);\
		EXPECT_EQ_BASE(json::Number, NUMBER.get_type());\
		EXPECT_EQ_BASE(expect, NUMBER.get_number());\
	} while(0)

static void test_parse_number()
{
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
}

#define TEST_STRING(expect, content)\
	do {\
		yanxujson::Json str;\
		str.parse(content, status);\
		EXPECT_EQ_BASE("LEPT_PARSE_OK", status); \
		EXPECT_EQ_BASE(json::String, str.get_type());\
		EXPECT_EQ_BASE(0, memcmp(expect, str.get_string().c_str(), str.get_string().size()));\
	} while(0)

#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE(0, memcmp(expect, actual.c_str(), actual.size()));

static void test_parse_string()
{

	TEST_STRING("", "\"\"");
	TEST_STRING("Hello", "\"Hello\"");

	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");

	TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
	TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}



static void test_parse_array()
{
	yanxujson::Json arr;

	arr.parse("[ ]", status);
	EXPECT_EQ_BASE("LEPT_PARSE_OK", status);
	EXPECT_EQ_BASE(json::Array, arr.get_type());
	EXPECT_EQ_BASE(0, arr.array_get_size());

	arr.parse("[ null , false , true , 123 , \"abc\" ]", status);
	EXPECT_EQ_BASE("LEPT_PARSE_OK", status);
	EXPECT_EQ_BASE(json::Array, arr.get_type());
	EXPECT_EQ_BASE(5, arr.array_get_size());
	EXPECT_EQ_BASE(json::Null, arr.array_get_element(0).get_type());
	EXPECT_EQ_BASE(json::False, arr.array_get_element(1).get_type());
	EXPECT_EQ_BASE(json::True, arr.array_get_element(2).get_type());
	EXPECT_EQ_BASE(json::Number, arr.array_get_element(3).get_type());
	EXPECT_EQ_BASE(json::String, arr.array_get_element(4).get_type());
	EXPECT_EQ_BASE(123.0, arr.array_get_element(3).get_number());
	EXPECT_EQ_STRING("abc", arr.array_get_element(4).get_string());

	arr.parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", status);
	EXPECT_EQ_BASE("LEPT_PARSE_OK", status);
	EXPECT_EQ_BASE(json::Array, arr.get_type());
	EXPECT_EQ_BASE(4, arr.array_get_size());
	for (int i = 0; i < 4; ++i) {
		yanxujson::Json a = arr.array_get_element(i);
		EXPECT_EQ_BASE(json::Array, a.get_type());
		EXPECT_EQ_BASE(i, a.array_get_size());
		for (int j = 0; j < i; ++j) {
			yanxujson::Json e = a.array_get_element(j);
			EXPECT_EQ_BASE(json::Number, e.get_type());
			EXPECT_EQ_BASE((double)j, e.get_number());

		}
	}
}



static void test_parse_object()
{
	yanxujson::Json obj;

	obj.parse(" { } ", status);
	EXPECT_EQ_BASE("LEPT_PARSE_OK", status);
	EXPECT_EQ_BASE(json::Object, obj.get_type());
	EXPECT_EQ_BASE(0, obj.object_get_size());

	obj.parse(" { "
		"\"n\" : null , "
		"\"f\" : false , "
		"\"t\" : true , "
		"\"i\" : 123 , "
		"\"s\" : \"abc\", "
		"\"a\" : [ 1, 2, 3 ],"
		"\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
		" } ", status);
	EXPECT_EQ_BASE("LEPT_PARSE_OK", status);
	EXPECT_EQ_BASE(7, obj.object_get_size());
	EXPECT_EQ_BASE("n", obj.object_get_key(0));
	EXPECT_EQ_BASE(json::Null, obj.object_get_value(0).get_type());
	EXPECT_EQ_BASE("f", obj.object_get_key(1));
	EXPECT_EQ_BASE(json::False, obj.object_get_value(1).get_type());
	EXPECT_EQ_BASE("t", obj.object_get_key(2));
	EXPECT_EQ_BASE(json::True, obj.object_get_value(2).get_type());
	EXPECT_EQ_BASE("i", obj.object_get_key(3));
	EXPECT_EQ_BASE(json::Number, obj.object_get_value(3).get_type());
	//EXPECT_EQ_BASE(json::Number, obj.object_get_value("i").get_type());
	EXPECT_EQ_BASE(123.0, obj.object_get_value(3).get_number());
	//EXPECT_EQ_BASE(123.0, obj.object_get_value("i").get_number());

	//obj.object_get_value("asd");

	EXPECT_EQ_BASE("s", obj.object_get_key(4));
	EXPECT_EQ_BASE(json::String, obj.object_get_value(4).get_type());
	EXPECT_EQ_BASE("abc", obj.object_get_value(4).get_string());
	EXPECT_EQ_BASE("a", obj.object_get_key(5));
	EXPECT_EQ_BASE(json::Array, obj.object_get_value(5).get_type());
	EXPECT_EQ_BASE(3, obj.object_get_value(5).array_get_size());
	for (int i = 0; i < 3; ++i) {
		yanxujson::Json v1 = obj.object_get_value(5).array_get_element(i);
		EXPECT_EQ_BASE(json::Number, v1.get_type());
		EXPECT_EQ_BASE(i + 1.0, v1.get_number());
	}
	EXPECT_EQ_BASE("o", obj.object_get_key(6));
	{
		yanxujson::Json v2 = obj.object_get_value(6);
		EXPECT_EQ_BASE(json::Object, v2.get_type());

		
		for (int i = 0; i < 3; ++i) {
			yanxujson::Json v3 = v2.object_get_value(i);
			EXPECT_EQ_BASE('1' + i, (v2.object_get_key(i))[0]);
			EXPECT_EQ_BASE(1, v2.object_get_key_length(i));
			EXPECT_EQ_BASE(json::Number, v3.get_type());
			EXPECT_EQ_BASE(i + 1.0, v3.get_number());
		}
	}

}

#define TEST_ERROR(error, content) \
	do {\
		yanxujson::Json LITERAL;\
		LITERAL.parse(content, status);\
		EXPECT_EQ_BASE(error, status);\
		EXPECT_EQ_BASE((json::Null), LITERAL.get_type());\
	} while(0)

static void test_parse_expect_value()
{
	TEST_ERROR("LEPT_PARSE_EXPECT_VALUE", "");
	TEST_ERROR("LEPT_PARSE_EXPECT_VALUE", " ");
}

static void test_parse_invalid_value()
{
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", "nul");
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", "?/");
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", "+0");
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", "+1");
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", ".123"); /* at least one digit before '.' */
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", "1.");   /* at least one digit after '.' */
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", "INF");
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", "inf");
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", "NAN");
	TEST_ERROR("LEPT_PARSE_INVALID_VALUE", "nan");
}

static void test_parse_root_not_singular()
{
	TEST_ERROR("LEPT_PARSE_ROOT_NOT_SINGULAR", "null x");
	TEST_ERROR("LEPT_PARSE_ROOT_NOT_SINGULAR", "nullll");

	TEST_ERROR("LEPT_PARSE_ROOT_NOT_SINGULAR", "0123");
	TEST_ERROR("LEPT_PARSE_ROOT_NOT_SINGULAR", "0x0");
	TEST_ERROR("LEPT_PARSE_ROOT_NOT_SINGULAR", "0x123");
}



static void test_parse_number_too_big()
{
	TEST_ERROR("LEPT_PARSE_NUMBER_TOO_BIG", "1e309");
	TEST_ERROR("LEPT_PARSE_NUMBER_TOO_BIG", "-1e309");
}



static void test_parse_invalid_unicode_surrogate() {
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_SURROGATE", "\"\\uD800\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_SURROGATE", "\"\\uDBFF\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_SURROGATE", "\"\\uD800\\\\\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_SURROGATE", "\"\\uD800\\uDBFF\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_SURROGATE", "\"\\uD800\\uE000\"");
}

static void test_parse_invalid_string_escape()
{

	TEST_ERROR("LEPT_PARSE_INVALID_STRING_ESCAPE", "\"\\v\"");
	TEST_ERROR("LEPT_PARSE_INVALID_STRING_ESCAPE", "\"\\'\"");
	TEST_ERROR("LEPT_PARSE_INVALID_STRING_ESCAPE", "\"\\0\"");
	TEST_ERROR("LEPT_PARSE_INVALID_STRING_ESCAPE", "\"\\x12\"");

}

static void test_parse_missing_quotation_mark()
{
	TEST_ERROR("LEPT_PARSE_MISS_QUOTATION_MARK", "\"");
	TEST_ERROR("LEPT_PARSE_MISS_QUOTATION_MARK", "\"abc");
}


static void test_parse_invalid_string_char()
{

	TEST_ERROR("LEPT_PARSE_INVALID_STRING_CHAR", "\"\x01\"");
	TEST_ERROR("LEPT_PARSE_INVALID_STRING_CHAR", "\"\x1F\"");

}
static void test_parse_invalid_unicode_hex()
{
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u0\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u01\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u012\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u/000\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\uG000\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u0/00\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u0G00\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u0/00\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u00G0\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u000/\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u000G\"");
	TEST_ERROR("LEPT_PARSE_INVALID_UNICODE_HEX", "\"\\u 123\"");
}



static void test_parse_miss_comma_or_square_bracket() 
{

	TEST_ERROR("LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET", "[1");
	TEST_ERROR("LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET", "[1}");
	TEST_ERROR("LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET", "[1 2");
	TEST_ERROR("LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET", "[[]");

}

static void test_parse_miss_key()
{
	TEST_ERROR("LEPT_PARSE_MISS_KEY", "{:1,");
	TEST_ERROR("LEPT_PARSE_MISS_KEY", "{1:1,");
	TEST_ERROR("LEPT_PARSE_MISS_KEY", "{true:1,");
	TEST_ERROR("LEPT_PARSE_MISS_KEY", "{false:1,");
	TEST_ERROR("LEPT_PARSE_MISS_KEY", "{null:1,");
	TEST_ERROR("LEPT_PARSE_MISS_KEY", "{[]:1,");
	TEST_ERROR("LEPT_PARSE_MISS_KEY", "{{}:1,");
	TEST_ERROR("LEPT_PARSE_MISS_KEY", "{\"a\":1,");
}

static void test_parse_miss_colon()
{
	TEST_ERROR("LEPT_PARSE_MISS_COLON", "{\"a\"}");
	TEST_ERROR("LEPT_PARSE_MISS_COLON", "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket()
{
	TEST_ERROR("LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET", "{\"a\":1");
	TEST_ERROR("LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET", "{\"a\":1]");
	TEST_ERROR("LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET", "{\"a\":1 \"b\"");
	TEST_ERROR("LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET", "{\"a\":{}");
}


static void test_parse() {
	test_parse_literal();
	test_parse_number();
	test_parse_string();
	test_parse_array();
	test_parse_object();

	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();

	test_parse_invalid_unicode_surrogate();
	test_parse_invalid_string_escape();
	test_parse_missing_quotation_mark();
	test_parse_invalid_string_char();
	test_parse_invalid_unicode_hex();

	test_parse_miss_comma_or_square_bracket();
	test_parse_miss_key();
	test_parse_miss_colon;
	test_parse_miss_comma_or_curly_bracket();

}


static void test_access_null()
{
	yanxujson::Json literal;
	literal.set_string("a");
	literal.set_null();
	EXPECT_EQ_BASE(json::Null, literal.get_type());
}

static void test_access_boolean()
{
	yanxujson::Json literal;
	literal.set_string("a");
	literal.set_boolean(false);
	EXPECT_EQ_BASE(json::False, literal.get_type());
}

static void test_access_number()
{
	yanxujson::Json num;
	num.set_string("a");
	num.set_number(1234.5);
	EXPECT_EQ_BASE(1234.5, num.get_number());
}

static void test_access_string()
{
	yanxujson::Json str;
	str.set_string("");
	EXPECT_EQ_STRING("", str.get_string());
	str.set_string("Hello");
	EXPECT_EQ_STRING("Hello", str.get_string());
}

static void test_access_array()
{
	yanxujson::Json arr, val;

	for (size_t j = 0; j < 5; j += 5) {
		arr.array_init();
		EXPECT_EQ_BASE(0, arr.array_get_size());
		for (int i = 0; i < 10; ++i) {
			val.set_number(i);
			arr.array_pushback_element(val);
		}

		EXPECT_EQ_BASE(10, arr.array_get_size());
		for (int i = 0; i < 10; ++i)
			EXPECT_EQ_BASE(static_cast<double>(i), arr.array_get_element(i).get_number());
	}

	arr.array_popback_element();
	EXPECT_EQ_BASE(9, arr.array_get_size());
	for (int i = 0; i < 9; ++i)
		EXPECT_EQ_BASE(static_cast<double>(i), arr.array_get_element(i).get_number());

	arr.array_erase_element(4, 0);
	EXPECT_EQ_BASE(9, arr.array_get_size());
	for (int i = 0; i < 9; ++i)
		EXPECT_EQ_BASE(static_cast<double>(i), arr.array_get_element(i).get_number());

	arr.array_erase_element(8, 1);
	EXPECT_EQ_BASE(8, arr.array_get_size());
	for (int i = 0; i < 8; ++i)
		EXPECT_EQ_BASE(static_cast<double>(i), arr.array_get_element(i).get_number());

	arr.array_erase_element(0, 2);
	EXPECT_EQ_BASE(6, arr.array_get_size());
	for (int i = 0; i < 6; ++i)
		EXPECT_EQ_BASE(static_cast<double>(i) + 2, arr.array_get_element(i).get_number());

	for (int i = 0; i < 2; ++i) {
		val.set_number(i);
		arr.array_insert_element(val, i);
	}

	EXPECT_EQ_BASE(8, arr.array_get_size());
	for (int i = 0; i < 8; ++i) {
		EXPECT_EQ_BASE(static_cast<double>(i), arr.array_get_element(i).get_number());
	}

	val.set_string("Hello");
	arr.array_pushback_element(val);

	arr.array_clear();
	EXPECT_EQ_BASE(0, arr.array_get_size());
}

static void test_access_object()
{
	yanxujson::Json obj, val;

	for (int j = 0; j <= 5; j += 5) {
		obj.object_init();
		EXPECT_EQ_BASE(0, obj.object_get_size());
		
		for (int i = 0; i < 10; ++i) {
			string key = "a";
			key[0] += i;
			val.set_number(i);
			obj.object_set_value(key, val);
		}
		
		EXPECT_EQ_BASE(10, obj.object_get_size());
		for (int i = 0; i < 10; ++i) {
			string key = "a";
			key[0] += i;
			auto index = obj.object_find_index(key);
			EXPECT_EQ_BASE(1, static_cast<int>(index >= 0));
			val = obj.object_get_value(index);
			EXPECT_EQ_BASE(static_cast<double>(i), val.get_number());
		}
		
	}

	auto index = obj.object_find_index("j");
	EXPECT_EQ_BASE(1, static_cast<int>(index >= 0));
	obj.object_remove_value(index);
	index = obj.object_find_index("j");

	EXPECT_EQ_BASE(1, static_cast<int>(index < 0));
	EXPECT_EQ_BASE(9, obj.object_get_size());

	index = obj.object_find_index("a");
	EXPECT_EQ_BASE(1, static_cast<int>(index >= 0));
	obj.object_remove_value(index);
	index = obj.object_find_index("a");
	EXPECT_EQ_BASE(1, static_cast<int>(index < 0));
	EXPECT_EQ_BASE(8, obj.object_get_size());

	for (int i = 0; i < 8; i++) {
		string key = "a";
		key[0] += i + 1;
		EXPECT_EQ_BASE((double)i + 1, obj.object_get_value(obj.object_find_index(key)).get_number());
	}

	val.set_string("Hello");
	obj.object_set_value("World", val);

	index = obj.object_find_index("World");
	EXPECT_EQ_BASE(1, static_cast<int>(index >= 0));
	val = obj.object_get_value(index);
	EXPECT_EQ_STRING("Hello", val.get_string());

	obj.object_clear();
	EXPECT_EQ_BASE(0, obj.object_get_size());

}

static void test_access()
{
	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
	test_access_array();
	test_access_object();
}



#define TEST_ROUNDTRIP(content)\
	do {\
		yanxujson::Json val;\
		val.parse(content, status);\
		EXPECT_EQ_BASE("LEPT_PARSE_OK", status);\
		val.stringify(status);\
		EXPECT_EQ_BASE(content, status);\
	} while(0)

static void test_stringify_number() {
	TEST_ROUNDTRIP("0");
	TEST_ROUNDTRIP("-0");
	TEST_ROUNDTRIP("1");
	TEST_ROUNDTRIP("-1");
	TEST_ROUNDTRIP("1.5");
	TEST_ROUNDTRIP("-1.5");
	TEST_ROUNDTRIP("3.25");
	TEST_ROUNDTRIP("1e+20");
	TEST_ROUNDTRIP("1.234e+20");
	TEST_ROUNDTRIP("1.234e-20");

	TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
	TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
	TEST_ROUNDTRIP("-4.9406564584124654e-324");
	TEST_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
	TEST_ROUNDTRIP("-2.2250738585072009e-308");
	TEST_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
	TEST_ROUNDTRIP("-2.2250738585072014e-308");
	TEST_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
	TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
	TEST_ROUNDTRIP("\"\"");
	TEST_ROUNDTRIP("\"Hello\"");
	TEST_ROUNDTRIP("\"Hello\\nWorld\"");
	TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
	TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
	TEST_ROUNDTRIP("[]");
	TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
	TEST_ROUNDTRIP("{}");
	TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify() {
	TEST_ROUNDTRIP("null");
	TEST_ROUNDTRIP("false");
	TEST_ROUNDTRIP("true");
	test_stringify_number();
	test_stringify_string();
	test_stringify_array();
	test_stringify_object();
}

#define TEST_EQUAL(json1, json2, equality)\
	do {\
		yanxujson::Json val1, val2;\
		val1.parse(json1, status);\
		EXPECT_EQ_BASE("LEPT_PARSE_OK", status);\
		val2.parse(json2, status);\
		EXPECT_EQ_BASE("LEPT_PARSE_OK", status);\
		EXPECT_EQ_BASE(equality, int(val1 == val2));\
	} while(0)

static void test_equal() {
	TEST_EQUAL("true", "true", 1);
	TEST_EQUAL("true", "false", 0);
	TEST_EQUAL("false", "false", 1);
	TEST_EQUAL("null", "null", 1);
	TEST_EQUAL("null", "0", 0);
	TEST_EQUAL("123", "123", 1);
	TEST_EQUAL("123", "456", 0);
	TEST_EQUAL("\"abc\"", "\"abc\"", 1);
	TEST_EQUAL("\"abc\"", "\"abcd\"", 0);
	TEST_EQUAL("[]", "[]", 1);
	TEST_EQUAL("[]", "null", 0);
	TEST_EQUAL("[1,2,3]", "[1,2,3]", 1);
	TEST_EQUAL("[1,2,3]", "[1,2,3,4]", 0);
	TEST_EQUAL("[[]]", "[[]]", 1);
	TEST_EQUAL("{}", "{}", 1);
	TEST_EQUAL("{}", "null", 0);
	TEST_EQUAL("{}", "[]", 0);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0);
	TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1);
	TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0);
}

static void test_copy() {
	yanxujson::Json val1, val2;
	val1.parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
	val2 = val1;
	EXPECT_EQ_BASE(1, int(val2 == val1));
}

static void test_move() {
	yanxujson::Json val1, val2, val3;
	val1.parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
	val2 = val1;
	val3 = std::move(val2);
	EXPECT_EQ_BASE(json::Null, val2.get_type());
	EXPECT_EQ_BASE(1, int(val3 == val1));
}

static void test_swap() {
	yanxujson::Json val1, val2;
	val1.set_string("Hello");
	val2.set_string("World!");
	yanxujson::swap(val1, val2);
	EXPECT_EQ_BASE("World!", val1.get_string());
	EXPECT_EQ_BASE("Hello", val2.get_string());
}

int main() {
	test_parse();
	test_stringify();
	test_access();
	test_equal();
	test_copy();
	test_move();
	test_swap();
	//cout << test_pass << " " << test_count << " " << test_pass * 100.0 / test_count << "% passed" << endl;
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}

