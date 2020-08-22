#pragma once

#include <type_traits>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <typeinfo>

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

inline AbstractValue::~AbstractValue() {}

template<typename T>
class Value : public AbstractValue {
public:
	Value(const T &data) :
		data_(data)
	{
		if constexpr (std::is_arithmetic_v<T>) {
			type_ = ValueType::NUMBER;
		} else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
			type_ = ValueType::STRING;
		} else if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
			type_ = ValueType::BOOL;
		} else if constexpr (std::is_same_v<std::decay_t<T>, cv::VideoCapture>) {
			type_ = ValueType::VIDEO;
		}
	}

	const T &get_data() {
		return data_;
	}

	void set_data(const T &data) {
		data_ = data;
	}

	virtual ~Value() {}

private:
	T data_;
};


class Program {
public:
	template<typename T>
	Value<T> *get_variable_value_by_id(const std::string &id) const {
		if (variables_.find(id) == variables_.end()) {
			error_ = "Variable with such id does not exist";
		}
		return dynamic_cast<Value<T>>(variables_.at(id));
	}

	void add_variable(const std::string& id, const AstNode const *data_node);

	int execute(const Tree *ast_tree);
private:
	void execute_library_import(const Tree *ast_tree);
	void execute_handler_import(const Tree *ast_tree);
	void execute_renderer_declaration(const Tree *ast_tree);
	void execute_source_declaration(const Tree *ast_tree);
	void execute_set_declaration(const Tree *ast_tree);
	void execute_element_declaration(const Tree *ast_tree);
	void execute_tuple_declaration(const Tree *ast_tree);
	void execute_aggregate_declaration(const Tree *ast_tree);
	void execute_actions(const Tree *ast_tree);
private:


	//bool is_value(AstNodeType t);
private:
	std::string error_;

	std::unordered_map<std::string, std::unique_ptr<AbstractValue>> variables_;

	std::unordered_map<std::string, std::type_index> types_;
};

void Program::add_variable(const std::string& id, const AstNode const* data_node) {
	std::unique_ptr<AbstractValue> abs_val;
	switch (data_node->type_) {
	case AstNodeType::NUMBER:
		abs_val = std::make_unique<Value<double>>(stod(data_node->value_));
		break;
	case AstNodeType::BOOL:
		abs_val = std::make_unique<Value<bool>>(data_node->value_ == "true" ? true : false);
		break;
	case AstNodeType::STRING:
		abs_val = std::make_unique<Value<std::string>>(data_node->value_);
		break;
	}
	if (!abs_val.get()) {
		error_ = "Invalid type of data node";
	}
	variables_.emplace(id, std::move(abs_val));
}