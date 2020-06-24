// Asampl.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"

#include <iostream>
#include <fstream>

#include <string> 
#include "lexer.h"
#include "parser.h"
#include "print_ast.h"
#include "interpreter.h"
#include <opencv2/opencv.hpp>



int main(int argc, char *argv[])
{
	

	//std::fstream fileStream("D:/test.txt");
	if (argc < 2) {
		std::cout << "Error, no input file specified";
		return 0;
	}
	const char * fileName = argv[1];

	std::fstream fileStream(fileName);
	//std::cout << "File: "<< fileName << std::endl;

	if (!fileStream) {
		std::cout << "Error while trying to open input file";
		return 0;
	}

	std::vector<Lexem> lexem_sequence;
	int code = Lexer_splitTokens(&fileStream, &lexem_sequence);
	//LexemPrint(&lexem_sequence);

	if (code == -1) {
		std::cout << "Error while reading program file stream";
		return 0;
	}

	Tree* tree = parser_buid_tree(&lexem_sequence);
	if (!tree) {
		std::cout << "Error while parsing sequence of lexemes";
		return 0;
	}

	//std::ofstream myfile;
	//myfile.open("D:/tree.txt");
	//AstTree_prettyPrint(tree, myfile);
	//myfile.close();

	execute(tree);

		//tree->print();
		//std::cout << tree->children[0]->children[0]->value->name << std::endl;
	
	return 0;

}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
