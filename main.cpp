// Asampl.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

// #include "pch.h"

#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>

#include "lexer.h"
#include "parser.h"
 #include "interpreter.h"

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Error, no input file specified\n";
		return 0;
	}
    std::filesystem::path file_name = argv[1];

     std::filesystem::path handlers_directory;
     if (argc >= 3) {
         handlers_directory = argv[2];
     }

     std::filesystem::path libraries_directory;
     if (argc >= 4) {
         libraries_directory = argv[3];
     }

	std::fstream file_stream(file_name);

	if (!file_stream) {
		std::cerr << "Error while trying to open input file\n";
		return 0;
	}

	std::vector<token> lexem_sequence;
	int code = split_tokens(file_stream, lexem_sequence);

	if (code == -1) {
		std::cerr << "Error while spliting tokens\n";
		return 0;
	}

	as_tree *tree = buid_tree(lexem_sequence);

	if (!tree) {
		std::cerr << "Error while parsing tokens\n";
		return 0;
	}

	tree->print(std::cout);

    Asampl::Interpreter::Program program;
    program.add_handlers_directory(file_name.parent_path());
    program.add_libraries_directory(file_name.parent_path());

    program.add_handlers_directory(handlers_directory);
    if (!libraries_directory.empty()) {
        program.add_libraries_directory(libraries_directory);
    }
    //program.load_stdlib();
    program.execute(*tree);

	//Tree::free(tree);
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
