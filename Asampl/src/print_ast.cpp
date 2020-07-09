#include "pch.h"
#include "print_ast.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>



static void PrintPretty(Tree * node, std::string intend, int root, int last, std::ofstream& filestream);

void AstTree_prettyPrint(Tree * astTree, std::ofstream& filestream) {
	
	std::string indent = "";

	PrintPretty(astTree, indent, 1, 1, filestream);
	
}


static void PrintPretty(Tree * node, std::string indent, int root, int last, std::ofstream& filestream) {
	std::cout << indent;
	filestream << indent;
	
	std::string newIndent = "";
	if (last) {
		if (!root) {
			//fprintf(filestream, "└─");
			std::cout << "'-" ;
			filestream << "'-";
			//newIndent = str_append(indent, "••");
			newIndent = indent + "**";
		}
		else {
			//newIndent = str_append(indent, "");
			newIndent = indent + "";
		}
	}
	else {
		//fprintf(filestream, "├─");
		std::cout << "|-" ;
		filestream << "|-";
		//newIndent = str_append(indent, "|•");
		newIndent = indent + "|*";
	}
	AstNode* astNode = node->value;


	std::cout << astNode->value << std::endl;
	filestream << astNode->value << std::endl;

	std::vector<Tree*> children = (node->children);

	size_t count = children.size();


	for (int i = 0; i < count; i++) {
		Tree * child = children[i];
		PrintPretty(child, newIndent, 0, i == count - 1, filestream);
	}

	

}
