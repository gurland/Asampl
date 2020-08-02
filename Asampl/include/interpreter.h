#pragma once

#include <type_traits>
#include <unordered_map>
#include <string>
#include <variant>

#include <opencv2/opencv.hpp>

#include "tree.h"

enum class ValueType {
	NUMBER,
	BOOL,
	STRING,
	UNDEFIND,
	VIDEO,
	AUDIO
};

class AbstractValue {
public:
	ValueType get_type() { return type_; }
	virtual ~AbstractValue() = 0;
protected:
	ValueType type_;
};

template<typename T>
class Value : public AbstractValue {
public:
	Value(const T& data) :
		data_(data)
	{
		if (std::is_arithmetic_v<T>) {
			puts("NUMBER");
			type_ = ValueType::NUMBER;
		}
		else if (std::is_same_v<T, std::string>) {
			puts("STRING");
			type_ = ValueType::STRING;
		}
		else if (std::is_same_v<T, bool>) {
			puts("BOOL");
			type_ = ValueType::BOOL;
		}
		else if (std::is_same_v<T, cv::VideoCapture>) {
			puts("VIDEO");
			type_ = ValueType::VIDEO;
		}
	}

	template<typename = std::enable_if_t<std::is_object_v<T>>>
	T &get_data() {
		return data_;
	}

	T get_data() {
		return data_;
	}

	virtual ~Value() {}

private:
	T data_;
};

class Program {
public:
	template<typename T>
	Value<T>* get_variable_value_by_id(const std::string& id) const try {
		if (variables_.find(id) == variables_.end()) {
			throw std::exception("Variabl with such id does not exist");
		}
		return dynamic_cast<Value<T>>(variables_.at(id));
	}
	catch (std::exception e) {
		error_ = e.what();
	}

	template<typename T>
	void add_variable(const T& data, const std::string& id) {
		variables_.emplace(id, std::make_unique<Value<T>>(data));
	}

	int execute(const Tree* ast_tree);
private:
	void execute_library_import(const Tree* ast_tree);
	void execute_handler_import(const Tree* ast_tree);
	void execute_renderer_declaration(const Tree* ast_tree);
	void execute_source_declaration(const Tree* ast_tree);
	void execute_set_declaration(const Tree* ast_tree);
	void execute_element_declaration(const Tree* ast_tree);
	void execute_tuple_declaration(const Tree* ast_tree);
	void execute_aggregate_declaration(const Tree* ast_tree);
	void execute_actions(const Tree* ast_tree);
private:
	std::string error_;
	std::unordered_map<std::string, std::unique_ptr<AbstractValue>> variables_;
};