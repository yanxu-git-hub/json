# C++实现的轻量级json库

这是[从零开始的JOSN教程](https://github.com/miloyip/json-tutorial)的C++版本。在visual studio 2019上编译，仅支持UTF-8 JSON文本，以及double存储的JOSN number类型。

## API设计

### json 

面向用户，提供赋值，右值引用，josn字符串解析，获取数据类型，以及各种数据类型的具体操作。采用复合的设计方法，私有变量为一个指向Value类对象的智能指针来内部实现。

### json__value

Value类里是json功能接口。因为一个josn数据只有一种数据类型，私有变量里采用union对数字，字符串，数组，对象进行封装，减少额外空间的开销。

类外枚举出josn的数据类型。

### jsonThrowError

继承自logic_error，用于抛出解析json数据的异常。

### jsonParser（其实应该称为Parser_and_Generator）

Parser类用于json数据类型解析，Stringify用于生成json字符串。

### yanxujsontest.cpp

测试文件，与原作类似。

## 代码开发过程

为了和原作照应，同样分时间设计。

### Step1:null以及bool

#### json

写出三大函数，以及parse，set_null，set_boolean。`void parse(const string& content, string& status)`调用`void parse(const string& content)`，status是传出参数。解析无误返回"LEPT_PARSE_OK"，不然返回异常类型，由ThrowError类捕捉。

#### jsonValue

类外枚举出数据类型，类内写出三大函数，`get_type`，`set_type`以及`prase`。私有成员有数据类型，以及供拷贝函数和析构函数使用的`assign`，`free`。

#### jsonParse

Parse类，对外提供`Parser(Value& val, const string& content)`以供调用。由于每个json字符都可能在数据前后有空白，所以需要`parse_whitespace()`去除空白。两个私有成员变量：val_是json数值，cur表示指针当前所指位置。`parse_value`主体是一个switch语句，根据cur调用对应的解析函数。由于`“null”`，`“true”`,`"false"`都是长度固定的数值，统一采用`parse_literal`解析。若解析无误，则将其type设为对应type。

### Step2:number

#### json

提供`get_number`，`set_number`函数。

#### jsonValue

除了添加对应的函数外，由于null以及bool由其type就可以表示数值，而数字，字符串等等需要存储，采用union降低了额外存储空间。除此之外，assign需要添加number情况。

#### jsonParse

添加了`parse_number`函数。解析完毕后使用strtod检查number的数值是否越界。

### Step3:string

#### json

添加`get_string`，`set_string`函数。

#### jsonValue

从string开始，union内部的数据是非平凡的，需要调用析构函数。幸运的是接下来使用C++的标准容器接收数据类型，因此可以在判断其数据类型后对其进行相应析构函数调用。

其他功能添加如同number。

#### jsonParse

在函数设计上，之前的解析函数解析完成之后需要将数值或者类型自动。但是`parse_string`中的字符串解析功能在后面的对象解析也要使用到，所以`parse_string`中将解析和`set_string`拆开。`parse_string_raw(string& tmp)`作为传入传出参数。其中在处理时`'\'`需要注意转义字符串的处理。

额外注意的是，json需要处理Unicode，也就是实现\uXXXX的解析。根据原作，遇到 `\u` 转义时，调用 `parse_hex4` 解析 4 位十六进数字，存储为码点 `u`，然后调用`parse_encode_utf8(),`将其写入tmp。

### Step4:array

#### json

使用vector作为存储array的容器，因此可以很容易的根据vector的功能定义出`get_size`,`get_element`,`assign`,`pop`,`push`,等操作。

#### jsonValue

构造函数，赋值函数，以及对应json的接口都要实现。

#### jsonParser

数组是一个复合数据类型，在每一次的循环里将解析每一个数组元素，并存在在val中，创建临时数组vector<Value> tmp push每一次的解析，最后将tmp赋值给val。

### Step5:object

使用vector<pair<string, Value>>来存储object的容器。object与array高度相似，此处仅简单说明jsonParser中的`parser_object`函数。在每一次的对象元素解析里，定义临时变量`string key`去接收object中key，val接收值，传入临时数组`vector<pair<string, Value>> tmp`中，最后将tmp赋值给val。

### Step6:Generator

json数值转化为json文本，定义在Parser类里（应该另开辟一个类）。

`Stringify(const Value& val, string& content)` content存储解析结果，`stringify_value(const Value& val)`是其内部调用，`stringify_string(const string& str)`负责生成json string类型。

### Step7:swap,==,!=,右值引用

#### swap

使用std:swap

#### ==,!=

先判断json数据类型，在相等的情况下进一步判断number,string,array是否相等。object需要逐个键值比较。!=是==的取反

#### 右值引用

右值引用意为将数据的所有权从一个对象转移到另一个对象，如果是指针那么是浅拷贝。由于对一个智能指针做右值引用，因此将其所指向rhs指针所指资源，将rhs释放掉即可。

 



