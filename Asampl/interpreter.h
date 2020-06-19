#pragma once
#include "tree.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <opencv2/opencv.hpp>
#include <algorithm>  


typedef enum {
	ValueType_NUMBER,
	ValueType_BOOL,
	ValueType_STRING,
	ValueType_UNDEFIND,
	ValueType_VIDEO,
	ValueType_AUDIO
} DataType;

class Value {
public:
	DataType type;
	void * data;

	//temp video constructor
	Value(cv::VideoCapture data) {
		this->type = ValueType_VIDEO;
		this->data = new cv::VideoCapture(data);
	}

	//Direct constructor
	Value(DataType type, void * data) : type(type), data(data) {

	}

	//String
	Value(std::string data)  {
		this->type = ValueType_STRING;
		this->data = new std::string(data);
	}

	//BOOL
	Value(bool data)  {
		this->type = ValueType_BOOL;
		this->data = new bool(data);
	}

	//Number
	Value(double data) {
		this->type = ValueType_NUMBER;
		this->data = new double(data);
		//std::cout << *((double*)this->data) << std::endl;
	}

	//Undefined
	Value() {
		this->type = ValueType_UNDEFIND;
		this->data = NULL;
	}

	Value *clone() const { return new Value(*this); }

	void print() {
		switch (this->type) {
		case ValueType_STRING:
			std::cout << *(std::string *)this->data << "\n";
			break;
		case ValueType_BOOL:
			std::cout << *(bool *)this->data << "\n";
			break;
		case ValueType_NUMBER:
			std::cout << *(double *)this->data << "\n";
			break;
		default:
			std::cout << " Not implemented for this type!" << "\n";
			break;
		}
	}
};

/*
class Variables {
public:
	std::vector<Variable*> variables;

	Variables() {

	}

	bool containts(std::string name) {
		for (auto &variable : variables) {
			if (variable->name == name) {
				return true;
			}
		}
		return false;
	}

	Variable * get_variable_by_name(std::string name) {
		for (auto &variable : variables) {
			if (variable->name == name) {
				return variable;
			}
		}
		return NULL;
	}

	void change_variable_data(std::string name, DataType type, void * data) {


	}

	void add(Variable * variable_to_add) {
		Variable new_variable = *variable_to_add;

		variables.push_back(variable_to_add->clone());
	}

};

class Variable {
public:
	std::string name;
	DataType type;
	void* data;

	Variable() {
	}

	Variable(std::string name, DataType type, void * data) : name(name), type(type), data(data) {
	}

	Variable *clone() const { return new Variable(*this); }

	
};*/

class Program {
public:

	std::string error;

	std::map<std::string, Value *> variables;

	Value * get_variable_value_by_id(std::string varID) {
		if (this->variables.find(varID) == this->variables.end()) {
			this->error = "var id not found";
			return NULL;
		}
		return variables[varID];
	}

	void printVariables() {
		for (auto it = this->variables.cbegin(); it != this->variables.cend(); ++it)
		{
			std::cout << it->first;
			it->second->print();
		}
	}
};

int execute(Tree * astTree);

