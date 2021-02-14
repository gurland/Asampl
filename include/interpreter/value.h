#pragma once

#include <memory>
#include <functional>

#include <opencv2/opencv.hpp>

#include "tree.h"
#include "interpreter/image.h"

namespace _asa_value_private {

template< typename T >
struct IsVector {
    constexpr static bool value = false;
};

template< typename T >
struct IsVector<std::vector<T>> {
    using VectorType = T;
    constexpr static bool value = true;
};

}

enum class ValueType {
	NUMBER,
	BOOL,
	STRING,
	UNDEFINED,
	AUDIO,
    IMAGE
};

class AbstractValue;
using ValuePtr = std::shared_ptr<AbstractValue>;

class AbstractValue {
public:
	ValueType get_type() { return type_; }
	virtual ~AbstractValue() = default;

    virtual std::string to_string() const = 0;
    virtual ValuePtr clone() const = 0;

    static ValuePtr from_literal(const AstNode* data_node);

    virtual bool is_undefined() {
        return false;
    }

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
		} else if constexpr (std::is_same_v<std::decay_t<T>, AsaImage>) {
			type_ = ValueType::IMAGE;
		}
	}

    std::string to_string() const override {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            return data_;
        } else if constexpr (std::is_same_v<std::decay_t<T>, AsaImage>) {
            const cv::Mat& data = data_.data;
            std::stringstream ss;
            ss << "{IMAGE " << data.size().width << ":" << data.size().height << "}";
            return ss.str();
        } else if constexpr (_asa_value_private::IsVector<std::decay_t<T>>::value) {
            if (data_.empty()) {
                return "[]";
            } else {
                std::stringstream ss;
                ss << "[";
                for (auto it = data_.begin(); it != data_.end(); ++it) {
                    ss << (*it)->to_string();
                    if (it != std::prev(data_.end())) {
                        ss << ", ";
                    }
                }
                ss << "]";
                return ss.str();
            }
        } else {
            return std::to_string(data_);
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

    std::string to_string() const override {
        return "undefined";
    }

    ValuePtr clone() const override {
        return std::make_shared<UndefinedValue>();
    }

    virtual bool is_undefined() override {
        return true;
    }
};

template<typename T>
T& AbstractValue::try_get() {
    return dynamic_cast<Value<T>&>(*this).get_data();
}
