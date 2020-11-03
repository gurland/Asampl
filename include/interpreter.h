#pragma once

#include <type_traits>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <typeinfo>

#include <opencv2/opencv.hpp>

#include "tree.h"
#include "handler.h"

enum class ValueType {
	NUMBER,
	BOOL,
	STRING,
	UNDEFINED,
	VIDEO,
	AUDIO
};

class AbstractValue;
using ValuePtr = std::shared_ptr<AbstractValue>;

using Function = std::function<ValuePtr(const std::vector<ValuePtr>&)>;

class AbstractValue {
public:
	ValueType get_type() { return type_; }
	virtual ~AbstractValue() = default;

    virtual ValuePtr clone() const = 0;

    static ValuePtr from_literal(const AstNode* data_node);

    template<typename T>
    T& try_get();
protected:
	ValueType type_;
};

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

    ValuePtr clone() const override {
        return std::make_shared<Value<T>>(*this);
    }

	T& get_data() {
		return data_;
	}

	void set_data(const T &data) {
		data_ = data;
	}

private:
	T data_;
};

class UndefinedValue : public AbstractValue {
public:
    UndefinedValue() {
        type_ = ValueType::UNDEFINED;
    }

    ValuePtr clone() const override {
        return std::make_shared<UndefinedValue>();
    }
};

template<typename T>
T& AbstractValue::try_get() {
    return dynamic_cast<Value<T>&>(*this).get_data();
}

class Program {
public:
    ValuePtr get_abstract_variable_value_by_id(const std::string& id) {
		if (variables_.find(id) == variables_.end()) {
			error_ = "Variable with such id does not exist";
		}
		return variables_.at(id);
    }

	template<typename T>
	Value<T> *get_variable_value_by_id(const std::string &id) {
		if (variables_.find(id) == variables_.end()) {
			error_ = "Variable with such id does not exist";
		}
		return dynamic_cast<Value<T>>(variables_.at(id));
	}

	void add_variable(const std::string& id, const AstNode *data_node);
    void add_function(const std::string& id, Function func);

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

    ValuePtr evaluate_expression(const Tree* ast_tree);
private:


	//bool is_value(AstNodeType t);
private:
	std::string error_;

	std::unordered_map<std::string, ValuePtr> variables_;
    std::unordered_map<std::string, std::unique_ptr<Handler>> handlers_;
    std::unordered_map<std::string, Function> functions_;

	std::unordered_map<std::string, std::type_index> types_;
};

inline void Program::add_variable(const std::string& id, const AstNode *data_node) {
    auto value = AbstractValue::from_literal(data_node);
    if (value != nullptr) {
        variables_.emplace(id, std::move(value));
    } else {
		error_ = "Invalid type of data node";
    }
}

inline void Program::add_function(const std::string& id, Function func) {
    functions_.emplace(id, func);
}
