#include <string>
#include <iostream>
#include <vector>
#include <util.hpp>
#include <tinge.hpp>


namespace graph {
	#define TOKENS \
		X(TOKEN_NONE) \
		X(TOKEN_EOF) \
		X(TOKEN_LPAREN) \
		X(TOKEN_RPAREN) \
		X(TOKEN_IDENTIFIER)

	#define X(x) #x,
		const char* to_str[] = { TOKENS };
	#undef X

	#define X(x) x,
		enum { TOKENS };
	#undef X

	#undef TOKENS


	inline util::Token next_token(const char*& ptr) {
		util::Token tok{{ptr, nullptr}, TOKEN_NONE};

		auto& [view, type] = tok;
		auto& [vbegin, vend] = view;

		if (*ptr == '\0') {
			type = TOKEN_EOF;
		}

		else if (*ptr == '(') { type = TOKEN_LPAREN; ++ptr; }
		else if (*ptr == ')') { type = TOKEN_RPAREN; ++ptr; }

		else if (not util::is_whitespace(*ptr)) {
			type = TOKEN_IDENTIFIER;

			do {
				++ptr;
			} while (not util::is_whitespace(*ptr) and *ptr != '(' and *ptr != ')');
		}

		else if (util::is_whitespace(*ptr)) {
			do {
				++ptr;
			} while (util::is_whitespace(*ptr));

			return next_token(ptr);
		}

		else {
			std::cerr << "encountered an unknown character.\n";
			std::exit(-1);
		}

		vend = ptr - vbegin;

		return tok;
	}
}


namespace graph {
	struct Identifer {
		util::Token tok;
	};

	struct List {
		util::Token op;
		std::vector<util::Node> children;
	};

	struct Empty {};

	using AST = util::AST<List, Identifer, Empty>;
	using Lexer = util::Lexer<graph::next_token>;
}


namespace graph {
	inline util::Node expr(graph::Lexer& lex, graph::AST& tree) {
		if (lex.advance() != TOKEN_LPAREN) {
			std::cerr << "expected opening parenthesis\n";
			std::exit(-1);
		}


		util::Token op = lex.advance();


		if (op == TOKEN_RPAREN) {
			return tree.add<Empty>();
		}

		else if (op != TOKEN_IDENTIFIER) {
			std::cerr << "expected Identifer";
			std::exit(-1);
		}

		std::vector<util::Node> children;

		while (lex.peek() != TOKEN_RPAREN and lex.peek() != TOKEN_EOF) {
			if (lex.peek() == TOKEN_LPAREN) {
				children.emplace_back(expr(lex, tree));
			}

			else if (lex.peek() == TOKEN_IDENTIFIER) {
				children.emplace_back(tree.add<Identifer>(lex.advance()));
			}
		}

		if (lex.advance() != TOKEN_RPAREN) {
			std::cerr << "expected closing parenthesis\n";
			std::exit(-1);
		}

		return tree.add<List>(op, children);
	}
}


namespace graph {
	template <typename T>
	void render_nodes(
		const T& variant,
		const graph::AST& tree,
		std::string& str,
		const int indent_size, int parent_id, int& node_counter
	) {
		util::visit(variant,
			[&] (const List& l) {
				int self_id = node_counter++;
				const auto& [op, children] = l;

				str += tinge::tab(indent_size) + tinge::strcat("n", self_id, " [label=\"", op, "\"];\n");

				if (self_id != parent_id) {
					str += tinge::tab(indent_size) + tinge::strcat("n", parent_id, " -> n", self_id, ";\n");
				}

				for (const auto& child: children) {
					render_nodes(tree[child], tree, str, indent_size, self_id, node_counter);
					node_counter++;
				}
			},

			[&] (const Identifer& x) {
				int self_id = node_counter++;
				str += tinge::tab(indent_size) + tinge::strcat("n", self_id, " [label=\"", x.tok, "\"];\n");

				if (self_id != parent_id) {
					str += tinge::tab(indent_size) + tinge::strcat("n", parent_id, " -> n", self_id, ";\n");
				}
			},

			[&] (const Empty&) {}
		);
	}


	template <typename T>
	void render_cluster(
		const T& variant,
		const graph::AST& tree,
		std::string& str,
		const std::string& title = "subgraph",
		const int indent_size = 0,
		int& node_counter = 0
	) {
		str += tinge::tab(indent_size) + title + " {\n";
			render_nodes(variant, tree, str, indent_size + 1, node_counter, node_counter);
			node_counter++;
		str += tinge::tab(indent_size) + "}\n";
	}


	std::string render(
		const std::vector<util::Node>& roots,
		const graph::AST& tree,
		const std::string& title = "digraph",
		const int indent_size = 0
	) {
		int node_counter = 0;
		std::string str;

		str += tinge::tab(indent_size) + title + " {\n";

		int i = 0;
		for (const util::Node& n: roots) {
			render_cluster(tree[n], tree, str, "subgraph cluster" + std::to_string(i), indent_size + 1, node_counter);
			i++;
		}

		str += tinge::tab(indent_size) + "}\n";

		return str;
	}
}


namespace graph {
	std::vector<util::Node> parse(graph::Lexer& lex, graph::AST& tree) {
		std::vector<util::Node> roots;

		while (lex.peek() != graph::TOKEN_EOF) {
			roots.emplace_back(graph::expr(lex, tree));
		}

		return roots;
	}
}


int main(int argc, const char* argv[]) {
	if (argc != 2) {
		std::cerr << "usage: wpp <file>\n";
		return -1;
	}

	auto expr = util::read_file(argv[1]);

	graph::AST tree;
	graph::Lexer lex{expr.c_str()};

	auto roots = graph::parse(lex, tree);
	std::cout << graph::render(roots, tree);

	return 0;
}
