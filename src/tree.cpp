#include "pch.h"
#include "tree.h"

void Tree::print(std::ostream& file, const std::string& indent, bool root, int last) {
	if (root) file << "\n";
	file << indent;
	std::string new_indent = "";
	if (last) {
		if (!root) {
			file << "`-";
			new_indent = indent + "**";
		}
		else {
			new_indent = indent;
		}
	}
	else {
		file << "|-";
		new_indent = indent + "|*";
	}

	file << this->node_->value_ + "\n";
	size_t count = this->children_.size();
	for (int i = 0; i < count; ++i) {
		this->children_.at(i)->print(file, new_indent, false, i == count - 1);
	}
}