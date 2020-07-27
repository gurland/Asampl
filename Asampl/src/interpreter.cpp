#include "pch.h"
#include "interpreter.h"
#include <cassert>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <time.h> 
#include <ctime> 


static Value * execute_expression(Program * program, Tree * node);

void execute_action(Program * program, Tree * childNode);
void execute_block(Program * program, Tree * blockTreeNode);
void execute_if(Program * program, Tree * node);

void execute_while(Program * program, Tree * node);
void execute_switch(Program * program, Tree * node);

void execute_timeline(Program * program, Tree * node);
void execute_substitution(Program * program, Tree * node);
void execute_sequence(Program * program, Tree * node);
void execute_download(Program * program, Tree * node);
void execute_upload(Program * program, Tree * node);
void execute_render(Program * program, Tree * node);

void execute_print(Program * program, Tree * node);



void execute_library_import(Program * program, Tree * node);

void execute_handler_import(Program * program, Tree * node);
void execute_renderer_declaration(Program * program, Tree * node);
void execute_source_declaration(Program * program, Tree * node);
void execute_aggregate_declaration(Program * program, Tree * node);
void execute_set_declaration(Program * program, Tree * node);
void execute_element_declaration(Program * program, Tree * node);
void execute_tuple_declaration(Program * program, Tree * node);
void execute_aggregate_declaration(Program * program, Tree * node);

void execute_action(Program * program, Tree * node);

int tm_to_sec(std::tm tm) {
	return tm.tm_sec + tm.tm_min * 60 + tm.tm_hour * 360;
}


double is_number(Value * self) {
	assert(self->type == ValueType_NUMBER);
	return *((double*)self->data);
}

bool is_bool(Value * self) {
	assert(self->type == ValueType_BOOL);
	return *((bool*)self->data);
}

char * is_string(Value * self) {
	assert(self->type == ValueType_STRING);
	return ((char*)self->data);
}

typedef bool(*comparison_method)(Value * a, Value * b);

bool more_or_less(Value * a, Value * b) {

	if (a->type != b->type) assert(0 && "Not supposed");;
	switch (a->type) {
		case ValueType_NUMBER: return is_number(a) < is_number(b);
		default: assert(0 && "Not supposed");
	}
}

bool more_or_less_equal(Value * a, Value * b) {
	if (a->type != b->type) assert(0 && "Not supposed");;
	switch (a->type) {
		case ValueType_NUMBER: return is_number(a) <= is_number(b);
		case ValueType_BOOL: return is_bool(a) == is_bool(b);

		default: assert(0 && "Not supposed");
	}
}

bool equal(Value * a, Value * b) {
	if (a->type != b->type) return false;
	switch (a->type) {
		case ValueType_BOOL: return is_bool(a) == is_bool(a);
		case ValueType_NUMBER: return fabs(is_number(a) - is_number(b)) < 1e-6;
		case ValueType_STRING: return is_string(a) == is_string(b);

		case ValueType_UNDEFIND: return true;
		default: assert(0 && "Not supposed");
	}
}

bool value_to_bool(Value * self) {
	switch (self->type) {
	case ValueType_BOOL: return is_bool(self);
	case ValueType_NUMBER: return fabs(is_number(self)) > 1e-6;
	case ValueType_STRING: return is_string(self) != NULL;
	case ValueType_UNDEFIND: return false;
	default: assert(0 && "Not implemented");
	}
}

bool compare(Program * program, Tree * node, comparison_method funk) {
	Tree * firstChild = node->children_[0];
	Value * firstValue = execute_expression(program, firstChild);
	if (!program->error.empty()) return NULL;

	Tree * secondChild = node->children_[1];
	Value * secondValue = execute_expression(program, secondChild);
	if (!program->error.empty()) return NULL;

	bool res = funk(firstValue, secondValue);
	return res;
}


static Value * execute_expression(Program * program, Tree * node) {

	AstNode * astNode = node->node_;

	switch (astNode->type_)
	{

	case AstNodeType::NUMBER: {
		double number = atof(astNode->value_.c_str());
		//std::cout << number << std::endl;
		return new Value(number);
	}

	case AstNodeType::STRING: {
		return new Value(astNode->value_);
	}

	case AstNodeType::BOOL: {
		return new Value(astNode->value_ == "true" ? true : false);
	}

	case AstNodeType::ADD: {
		if (!node->children_.empty()) {
			Tree * firstChild = node->children_[0];
			Value * firstValue = execute_expression(program, firstChild);

			if (!program->error.empty()) return NULL;
			if (firstValue->type != ValueType_NUMBER) {
				program->error = "Invalid operation";
				return NULL;
			}

			if (node->children_.size() == 1) {
				return firstValue;
			}

			else {
				Tree * secondChild = node->children_[1];
				Value * secondValue = execute_expression(program, secondChild);

				if (!program->error.empty()) return NULL;
				if (secondValue->type != ValueType_NUMBER) {
					program->error = "Invalid operation";
					return NULL;
				}
				double res = is_number(firstValue) + is_number(secondValue);
			
				return new Value(res);
			}
		}
		return new Value();
	}

	case AstNodeType::MUL: {
		if (!node->children_.empty()) {
			Tree * firstChild = node->children_[0];
			Value * firstValue = execute_expression(program, firstChild);
			if (!program->error.empty()) return NULL;
			if (firstValue->type != ValueType_NUMBER) {
				program->error = "Invalid operation";
				return NULL;
			}
			Tree * secondChild = node->children_[1];
			Value * secondValue = execute_expression(program, secondChild);
			if (!program->error.empty()) return NULL;
			if (secondValue->type != ValueType_NUMBER) {
				program->error = "Invalid operation";
				return NULL;
			}
			double res = is_number(firstValue) * is_number(secondValue);
			return new Value(res);
		}
		return new Value();
	}
	case AstNodeType::DIV: {
		if (!node->children_.empty()) {
			Tree * firstChild = node->children_[0];
			Value * firstValue = execute_expression(program, firstChild);

			if (!program->error.empty()) return NULL;
			if (firstValue->type != ValueType_NUMBER) {
				program->error = "Invalid operation";
				return NULL;
			}
			Tree * secondChild = node->children_[1];
			Value * secondValue = execute_expression(program, secondChild);
			if (!program->error.empty()) return NULL;
			if (secondValue->type != ValueType_NUMBER) {
				program->error = "Invalid operation";
				return NULL;
			}
			if (fabs(is_number(secondValue)) < 1e-6) {
				program->error = "Invalid operation";
				return NULL;
			}

			double res = is_number(firstValue) / is_number(secondValue);

			return new Value(res);
		}
		return new Value();
	}

	case AstNodeType::SUB: {
		if (!node->children_.empty()) {
			Tree * firstChild = node->children_[0];
			Value * firstValue = execute_expression(program, firstChild);
			if (!program->error.empty()) return NULL;
			if (firstValue->type != ValueType_NUMBER) {
				program->error = "Invalid operation";
				return NULL;
			}

			if (node->children_.size() == 1) {
				double res = -is_number(firstValue);
				return new Value(res);
			}
			else {
				Tree * secondChild = node->children_[1];
				Value * secondValue = execute_expression(program, secondChild);
				if (!program->error.empty()) return NULL;
				if (secondValue->type != ValueType_NUMBER) {
					program->error = "Invalid operation";
					return NULL;
				}

				double res = is_number(firstValue) - is_number(secondValue);
				return new Value(res);
			}
		}
		return new Value();
	}

	case AstNodeType::AND: {
		Tree * firstChild = node->children_[0];
		Value * firstValue = execute_expression(program, firstChild);
		if (!program->error.empty()) return NULL;

		Tree * secondChild = node->children_[1];
		Value * secondValue = execute_expression(program, secondChild);
		if (!program->error.empty()) return NULL;

		bool res = value_to_bool(firstValue) && value_to_bool(secondValue);

		return new Value(res);
	}
	case AstNodeType::OR: {
		Tree * firstChild = node->children_[0];
		Value * firstValue = execute_expression(program, firstChild);
		if (!program->error.empty()) return NULL;
		Tree * secondChild = node->children_[1];
		Value * secondValue = execute_expression(program, secondChild);
		if (!program->error.empty()) return NULL;

		bool res = value_to_bool(firstValue) || value_to_bool(secondValue);

		return new Value(res);
	}
	case AstNodeType::EQUAL: {
		bool eq = compare(program, node, equal);
		return  new Value(eq);
	}
	case AstNodeType::NOTEQUAL: {
		bool eq = compare(program, node, equal);
		return  new Value(!eq);
	}
	case AstNodeType::LESS: {
		bool eq = compare(program, node, more_or_less);
		return  new Value(eq);
	}
	case AstNodeType::MORE: {
		bool eq = compare(program, node, more_or_less);
		return  new Value(!eq);
	}
	case AstNodeType::LESS_OR_EQUAL: {
		bool eq = compare(program, node, more_or_less_equal);
		return  new Value(eq);
	}
	case AstNodeType::MORE_OR_EQUAL: {
		bool eq = compare(program, node, more_or_less_equal);
		return  new Value(!eq);
	}
	case AstNodeType::ASSIGN: {
		Tree * firstChildNode = node->children_[0];
		AstNode * firstChild = firstChildNode->node_;

		if (firstChild->type_ != AstNodeType::ID) {
			program->error = "Can't assign to rvalue";
			return NULL;
		}
		std::string varId = firstChild->value_;
		//
		Tree * secondChild = node->children_[1];
		Value * secondValue = execute_expression(program, secondChild);
		if (!program->error.empty()) return NULL;
		//
		if (program->variables.find(varId) == program->variables.end()) {
			program->error = "Var for assign not found";
			delete(secondValue);
			return NULL;
		}
		
		//Value * oldValue = Dict_set(program->variables, varId, Value_newCopy(secondValue));
		Value * oldValue = program->variables[varId];
		program->variables[varId] = secondValue->clone();

		delete(oldValue);
		return secondValue;
		break;
	}

	case AstNodeType::ID: {

		std::string varId = astNode->value_;
	/*	if (!node->children_.empty()) {
			Tree * argListNode = node->children_[0];
			AstNode * argList = argListNode->value;
			assert(argList->type == AstNodeType::ARGLIST);
			//
			if (program->variables.find(varId) == program->variables.end()) {
				program->error = strdup("Call unknown function");
				return NULL;
			}
			//
			List * arguments = List_new();
			for (int i = 0; i < List_count(argListNode->children_); i++) {
				Tree * argListChildNode = List_at(argListNode->children_, i);
				Value * argumentValue = eval(program, argListChildNode);

				if (program->error) {
					// @todo free all arguments values
					List_free(arguments);
					return NULL;
				}
				List_add(arguments, argumentValue);
			}
			// call function 

			StdFunction * func = Dict_get(program->functions, (char *)varId);
			Value * retValue = func->ptr(program, arguments);

			for (int i = 0; i < List_count(arguments); i++) {
				Value * argListChildNode = List_at(arguments, i);
				if (argListChildNode->type != ValueType_ARRAY) {
					Value_free(argListChildNode);
				}
			}

			List_free(arguments);
			return retValue;
		}*/
		Value * varValue = program->get_variable_value_by_id(varId);
		if (!program->error.empty()) return NULL;
		return varValue->clone();
	}

	case AstNodeType::ARGLIST: {
		
	}

	default:
		assert("Not implemented");
		
		break;
	}
	return NULL;
}

void execute_if(Program * program, Tree * node) {

	Tree * exprNode = node->children_[0];

	Value * testValue = execute_expression(program, exprNode);

	if (!program->error.empty()) return;

	bool _bool = value_to_bool(testValue);

	delete(testValue);
	if (_bool) {
		execute_block(program, node->children_[1]);
	}
	else if (node->children_.size() > 2) {
		execute_block(program, node->children_[2]);
	}
}

void execute_switch(Program * program, Tree * node) {
	Tree * exprNode = node->children_[0];
	Value * switch_value = execute_expression(program, exprNode);
	if (!program->error.empty()) return;


	for (int i = 1; i < node->children_.size(); i++) {
		Tree * childNode = node->children_[i];
		AstNode * child = childNode->node_;
		if (child->type_ == AstNodeType::CASE) {
			Value * case_value = execute_expression(program, childNode->children_[0]);
			if (!program->error.empty()) return;
			if (equal(switch_value, case_value)) {
				execute_block(program, childNode->children_[1]);
				if (!program->error.empty()) return;
				return;
			}	
		}
	}

	if (node->children_[node->children_.size() - 1]->node_->type_ == AstNodeType::DEFAULT) {
		int num = node->children_.size() - 1;
		execute_block(program, node->children_[num]);

		if (!program->error.empty()) return;
		return;
	}
}

void execute_while(Program * program, Tree * node) {
	Tree * exprNode = node->children_[0];
	while (true) {
		Value * value = execute_expression(program, exprNode);
		if (!program->error.empty()) return;
		bool _bool = value_to_bool(value);

		delete(value);
		if (!_bool) break;
		//
		execute_block(program, node->children_[1]);
		if (!program->error.empty()) return;
	}
}

void execute_print(Program * program, Tree * node) {
	Tree * exprNode = node->children_[0];
	Value * valueToPrint = execute_expression(program, exprNode);
	valueToPrint->print();
	delete(valueToPrint);
}

void execute_timeline(Program * program, Tree * node)
{
	Tree * conditionNode = node->children_[0];

	Tree * actionsNode = node->children_[1];
	


	switch (conditionNode->node_->type_) {

	case AstNodeType::TIMELINE_EXPR:

		execute_block(program, actionsNode);
		break;

	case AstNodeType::TIMELINE_AS: {

		Value * value = execute_expression(program, conditionNode->children_[0]);

		if (!program->error.empty()) return;

		if (value->type != ValueType_STRING) {
			program->error = "Invalid timeline argument";
			return;
		}

		std::string value_d = *(std::string *)value->data;
		delete(value);

		struct std::tm timer = { 0 };
		std::istringstream ss(value_d);

		ss >> std::get_time(&timer, "%H:%M:%S");

		std::time_t begin;
		std::time_t now;

		time(&begin);

		while (true) {
			time(&now);
			if ( difftime(now, begin) > tm_to_sec(timer)) break;

			execute_block(program, actionsNode);
		}

	}
	break;

	case AstNodeType::TIMELINE_UNTIL:

		while (true) {
			Value * testValue = execute_expression(program, conditionNode->children_[0]);
			bool testBool = value_to_bool(testValue);
			delete(testValue);

			if (!testBool) break;

			execute_block(program, actionsNode);
			if (!program->error.empty()) return;
		}
	}

}

void execute_substitution(Program * program, Tree * node)
{
}

void execute_sequence(Program * program, Tree * node)
{
}

void execute_download(Program * program, Tree * node)
{

	Tree * variableNode = node->children_[0];
	AstNode * _variableNode = variableNode->node_;

	Tree * sourceNode = node->children_[1];
	Value * value = execute_expression(program, sourceNode);
	if (!program->error.empty()) return;
	if (value->type != ValueType_STRING) {
		program->error = "Invalid download argument";
		return;
	}
	std::string source = *(std::string *)value->data;
	delete(value);


	std::string varId = _variableNode->value_;

	cv::VideoCapture cap(source);
	//cv::VideoCapture cap("E:/video.mp4");

	if (!cap.isOpened()) {
		program->error = "Error opening stream or file";
		return;
	}
	/*while (1) {
		cv::Mat frame;
		cap >> frame;

		if (frame.empty()) break;

		cv::imshow("Frame", frame);
		char c = (char)cv::waitKey(25);
		if (c == 27)
			break;

	}*/


	Value * varValue = new Value(cap);

	if (program->variables.find(varId) != program->variables.end()) {
		program->error = "Duplicate variable id";
		delete(varValue);
		return;
	}
	program->variables.emplace(varId, varValue);
	
}

void execute_upload(Program * program, Tree * node)
{
	Value * sourceValue = execute_expression(program, node->children_[0]);
	if (!program->error.empty()) return;
	if (sourceValue->type != ValueType_VIDEO) return;

	cv::VideoCapture cap = *(cv::VideoCapture *)sourceValue->data;
	

	Tree * pathNode = node->children_[1];
	Value * _pathNode = execute_expression(program, pathNode);

	if (!program->error.empty()) return;
	if (_pathNode->type != ValueType_STRING) {
		program->error = "Invalid download argument";
		return;
	}
	std::string path = *(std::string *)_pathNode->data;
	delete(_pathNode);

	int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);


	
	cv::VideoWriter video(path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, cv::Size(frame_width, frame_height));
	while (1)
	{
		cv::Mat frame;
		cap >> frame;

		if (frame.empty())
			break;
		video.write(frame);

		char c = (char)cv::waitKey(1);
		if (c == 27)
			break;
	}
	video.release();
}

void execute_render(Program * program, Tree * node)
{
	Value * sourceValue = execute_expression(program, node->children_[0]);
	if (!program->error.empty()) return;

	//Value * handlerValue = execute_expression(program, node->children_[1]);
	//if (!program->error.empty()) return;

	if (sourceValue->type != ValueType_VIDEO) return;

	cv::Mat frame;

	*(cv::VideoCapture *)sourceValue->data >> frame;

	if (frame.empty()) return;

	imshow("Frame", frame);

	char c = (char)cv::waitKey(25);
	if (c == 27)
		return;
}



void execute_block(Program * program, Tree * blockTreeNode) {
	for (int i = 0; i < blockTreeNode->children_.size(); i++) {
		Tree * childNode = blockTreeNode->children_[i];

		AstNode * child = childNode->node_;
		execute_action(program, childNode);
		if (!program->error.empty()) break;
	}
}

void execute_library_import(Program * program, Tree * node)
{
}

void execute_handler_import(Program * program, Tree * node)
{
}

void execute_renderer_declaration(Program * program, Tree * node)
{
}

void execute_source_declaration(Program * program, Tree * node)
{
	for (auto &child : node->children_) {


		Tree * fChildNode = child->children_[0];
		AstNode * fChild = fChildNode->node_;

		Tree * sChildNode = child->children_[1];
		AstNode * sChild = sChildNode->node_;
		//
		std::string varId = fChild->value_;

		Value * varValue = execute_expression(program, sChildNode);

		if (!program->error.empty()) break;
		//
		if (program->variables.find(varId) != program->variables.end()) {
			program->error = "Duplicate variable id";
			delete(varValue);
			break;
		}

		program->variables.emplace(varId, varValue);
	}
}


void execute_set_declaration(Program * program, Tree * node)
{
}

void execute_element_declaration(Program * program, Tree * node)
{
	

	for (auto &child : node->children_) {


		Tree * fChildNode = child->children_[0];
		AstNode * fChild = fChildNode->node_;

		Tree * sChildNode = child->children_[1];
		AstNode * sChild = sChildNode->node_;
		//
		std::string varId = fChild->value_;

		Value * varValue = execute_expression(program, sChildNode);

		if (!program->error.empty()) break;
		//
		if (program->variables.find(varId) != program->variables.end()) {
			program->error = "Duplicate variable id";
			delete(varValue);
			break;
		}

		program->variables.emplace(varId, varValue);
	}

}


void execute_tuple_declaration(Program * program, Tree * node)
{
}

void execute_aggregate_declaration(Program * program, Tree * node)
{
}


void execute_action(Program * program, Tree * childNode) {
	AstNode * child = childNode->node_;
	switch (child->type_) {

	case AstNodeType::IF: {
		execute_if(program, childNode);
		if (!program->error.empty()) return;
		break;
	}
	case AstNodeType::WHILE: {

		execute_while(program, childNode);
		if (!program->error.empty()) return;
		break;
	}
	case AstNodeType::SWITCH: {

		execute_switch(program, childNode);
		if (!program->error.empty()) return;
		break;
	}

	case AstNodeType::SEQUENCE: {
		
		execute_sequence(program, childNode);
		if (!program->error.empty()) return;
		break;
	}

	case AstNodeType::DOWNLOAD: {
		execute_download(program, childNode);
		if (!program->error.empty()) return;
		break;
	}
	case AstNodeType::UPLOAD: {
		execute_upload(program, childNode);
		if (!program->error.empty()) return;
		break;
	}
	case AstNodeType::RENDER: {
		execute_render(program, childNode);
		if (!program->error.empty()) return;
		break;
	}
	case AstNodeType::TIMELINE: {
		execute_timeline(program, childNode);
		if (!program->error.empty()) return;
		break;
	}
	case AstNodeType::SUBSTITUTION: {
		execute_substitution(program, childNode);
		if (!program->error.empty()) return;
		break;
	}
	case AstNodeType::BLOCK: {
		execute_block(program, childNode);
		if (!program->error.empty()) return;
		break;
	}
	case AstNodeType::PRINT: {
		execute_print(program, childNode);
		if (!program->error.empty()) return;
		break;
	}
	default: {
		Value * val = execute_expression(program, childNode);
		if (!program->error.empty()) return;
		delete(val);
	}
	}
}


int execute(Tree * astTree)
{
	AstNode * astNode = astTree->node_;
	assert(astNode->type_ == AstNodeType::PROGRAM);
	Program program;

	for (auto &child : astTree->children_) {
		AstNode * childNode = child->node_;

		switch (childNode->type_) {
		case AstNodeType::LIBRARIES: {
		//	std::cout << "AstNodeType::LIBRARIES"  << std::endl;
			execute_library_import(&program, child);
			if (!program.error.empty()) break;
			break;
		}
		case AstNodeType::HANDLERS: {
			//std::cout << "AstNodeType::HANDLERS" << std::endl;
			execute_handler_import(&program, child);
			if (!program.error.empty()) break;
			break;
		}
		case AstNodeType::RENDERERS: {
			//std::cout << "AstNodeType::RENDERERS" << std::endl;
			execute_renderer_declaration(&program, child);
			if (!program.error.empty()) break;
			break;
		}
		case AstNodeType::SOURCES: {
			//std::cout << "AstNodeType::SOURCES" << std::endl;
			execute_source_declaration(&program, child);
			if (!program.error.empty()) break;
			break;
		}
		case AstNodeType::SETS: {
			//std::cout << "AstNodeType::SETS" << std::endl;
			execute_set_declaration(&program, child);
			if (!program.error.empty()) break;
			break;
		}
		case AstNodeType::ELEMENTS: {
			//std::cout << "AstNodeType::ELEMENTS" << std::endl;
			execute_element_declaration(&program, child);
			if (!program.error.empty()) break;
			break;
		}
		case AstNodeType::TUPLES: {
			//std::cout << "AstNodeType::TUPLES" << std::endl;
			execute_tuple_declaration(&program, child);
			if (!program.error.empty()) break;
			break;
		}
		case AstNodeType::AGGREGATES: {
			//std::cout << "AstNodeType::AGGREGATES" << std::endl;
			execute_aggregate_declaration(&program, child);
			if (!program.error.empty()) break;
			break;
		}
		case AstNodeType::ACTIONS: {
			//unsigned int start_time = clock();
			//std::cout << "AstNodeType::ACTIONS" << std::endl;
			execute_block(&program, child);
			//unsigned int end_time = clock();
			//unsigned int search_time = end_time - start_time;
			//std::cout << "Time " << search_time << std::endl;
			if (!program.error.empty()) break;
			break;
		}



		default: {
			program.error = "Unrecognized section";
			if (!program.error.empty()) break;
			break;
		}
		}

	}
	
	if (!program.error.empty()) {
		std::cout << "Runtime error: " << program.error << std::endl;
		return 1;
	}
	return 0;


}

