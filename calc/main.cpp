#include <iostream>
#include <variant>
#include <cstdint>
#include <cmath>
#include <random>

#include <tinge.hpp>
#include <util.hpp>

// #define BENCH

#ifdef BENCH
	#define ANKERL_NANOBENCH_IMPLEMENT
	#include <nanobench.h>
#endif


namespace calc {
	#define TOKENS \
		X(TOKEN_NONE) \
		X(TOKEN_EOF) \
		X(TOKEN_LITERAL) \
		\
		X(TOKEN_ADD) \
		X(TOKEN_SUB) \
		X(TOKEN_MUL) \
		X(TOKEN_DIV) \
		X(TOKEN_MOD) \
		X(TOKEN_POW) \
		\
		X(TOKEN_LSHIFT) \
		X(TOKEN_RSHIFT) \
		\
		X(TOKEN_AND) \
		X(TOKEN_OR) \
		X(TOKEN_NOT) \
		X(TOKEN_XOR) \
		\
		X(TOKEN_LPAREN) \
		X(TOKEN_RPAREN)


	#define X(x) #x,
		const char* to_str[] = { TOKENS };
	#undef X

	#define X(x) x,
		enum { TOKENS };
	#undef X

	#undef TOKENS


	inline util::Token next_token(const char*& ptr) {
		util::Token tok{{ptr, 1}, TOKEN_NONE};

		auto& [view, type] = tok;
		auto& [vbegin, vend] = view;

		if (*ptr == '\0') {
			type = TOKEN_EOF;
		}

		else if (util::is_digit(*ptr)) {
			type = TOKEN_LITERAL;

			do {
				++ptr;
			} while (util::is_digit(*ptr));
		}

		else if (util::is_whitespace(*ptr)) {
			do {
				++ptr;
			} while (util::is_whitespace(*ptr));

			return next_token(ptr);
		}

		else if (*ptr == '+') { type = TOKEN_ADD; ++ptr; }
		else if (*ptr == '-') { type = TOKEN_SUB; ++ptr; }
		else if (*ptr == '/') { type = TOKEN_DIV; ++ptr; }
		else if (*ptr == '%') { type = TOKEN_MOD; ++ptr; }

		else if (*ptr == '*') {
			type = TOKEN_MUL;
			++ptr;

			if (*ptr == '*') {
				type = TOKEN_POW;
				++ptr;
			}
		}

		// else if (*ptr == '&') { type = TOKEN_AND; ++ptr; }
		// else if (*ptr == '|') { type = TOKEN_OR; ++ptr; }
		// else if (*ptr == '~') { type = TOKEN_NOT; ++ptr; }
		// else if (*ptr == '^') { type = TOKEN_XOR; ++ptr; }

		// else if (*ptr == '<' and *(ptr + 1) == '<') {
		// 	type = TOKEN_LSHIFT;
		// 	ptr += 2;
		// }

		// else if (*ptr == '>' and *(ptr + 1) == '>') {
		// 	type = TOKEN_RSHIFT;
		// 	ptr += 2;
		// }

		else if (*ptr == '(') { type = TOKEN_LPAREN; ++ptr; }
		else if (*ptr == ')') { type = TOKEN_RPAREN; ++ptr; }

		else {
			std::cerr << "encountered an unknown character.\n";
			std::exit(-1);
		}

		vend = ptr - vbegin;

		return tok;
	}
}


namespace calc {
	struct BinaryOp {
		util::Token op;
		util::Node lhs, rhs;

		BinaryOp(const util::Token& op_, util::Node lhs_, util::Node rhs_):
			op(op_), lhs(lhs_), rhs(rhs_) {}
	};

	struct UnaryOp {
		util::Token op;
		util::Node node;

		UnaryOp(const util::Token& op_, util::Node node_):
			op(op_), node(node_) {}
	};

	// using Literal = double;

	struct Literal {
		util::View view;
	};

	using AST = util::AST<BinaryOp, UnaryOp, Literal>;
	using Lexer = util::Lexer<calc::next_token>;
}





// Parser.
/*
	expr = expr '+' expr
		 | expr '-' expr
		 | expr '*' expr
		 | expr '/' expr
		 | expr '%' expr
		 | expr '^' expr
		 | expr '&' expr
		 | expr '|' expr
		 | expr '<<' expr
		 | expr '>>' expr
		 | '+' expr
		 | '-' expr
		 | '~' expr
*/
namespace calc {
	// Precedence. (Make sure none of these entries are 0)
	enum {
		PREC_OR  = 1,
		PREC_XOR = 2,
		PREC_AND = 3,

		PREC_LSHIFT = 4,
		PREC_RSHIFT = 4,

		PREC_ADD = 5,
		PREC_SUB = 5,

		PREC_MUL = 6,
		PREC_DIV = 6,
		PREC_MOD = 6,

		PREC_NEG = 7,
		PREC_POS = 7,
		PREC_NOT = 7,

		PREC_POW = 8,
	};

	// Associativity.
	enum {
		ASSOC_LEFT,
		ASSOC_RIGHT,
	};

	struct Entry {
		int prec;
		int assoc;

		int get() const { return prec + assoc; }
	};

	constexpr auto infix_bp = [] () {
		std::array<Entry, 256> arr{};

		for (auto& x: arr)
			x = { 0, 0 };

		arr[TOKEN_ADD]    = { PREC_ADD,    ASSOC_LEFT };
		arr[TOKEN_SUB]    = { PREC_SUB,    ASSOC_LEFT };
		arr[TOKEN_MUL]    = { PREC_MUL,    ASSOC_LEFT };
		arr[TOKEN_DIV]    = { PREC_DIV,    ASSOC_LEFT };
		arr[TOKEN_MOD]    = { PREC_MOD,    ASSOC_LEFT };
		arr[TOKEN_POW]    = { PREC_POW,    ASSOC_RIGHT };

		// arr[TOKEN_XOR]    = { PREC_XOR,    ASSOC_LEFT };
		// arr[TOKEN_AND]    = { PREC_AND,    ASSOC_LEFT };
		// arr[TOKEN_OR]     = { PREC_OR,     ASSOC_LEFT };

		// arr[TOKEN_LSHIFT] = { PREC_LSHIFT, ASSOC_LEFT };
		// arr[TOKEN_RSHIFT] = { PREC_RSHIFT, ASSOC_LEFT };

		return arr;
	} ();

	constexpr auto prefix_bp = [] () {
		std::array<Entry, 256> arr{};

		for (auto& x: arr)
			x = { 0, 0 };

		arr[TOKEN_ADD] = { PREC_ADD, ASSOC_RIGHT };
		arr[TOKEN_SUB] = { PREC_SUB, ASSOC_RIGHT };
		// arr[TOKEN_NOT] = { PREC_NOT, ASSOC_RIGHT };

		return arr;
	} ();

	// constexpr auto postfix_bp = [] () {
	// 	std::array<Entry, 256> arr{};

	// 	for (auto& x: arr)
	// 		x = { 0, 0 };

	// 	arr[TOKEN_FACT] = { PREC_FACT, ASSOC_LEFT };

	// 	return arr;
	// } ();


	// Pratt parser.
	inline util::Node expr(calc::Lexer& lex, calc::AST& tree, int bp = 0) {
		util::Node lhs{0};
		util::Token tok = lex.advance();

		switch (tok.type) {
			case TOKEN_ADD:
			case TOKEN_SUB:
			case TOKEN_NOT:
				lhs = tree.add<UnaryOp>(tok, expr(lex, tree, prefix_bp[tok.type].get()));
				break;

			case TOKEN_LITERAL: {
				// Literal num = 0;

				// for (auto [it, end] = tok.view; it != tok.view.begin + end; it++)
				// 	num = (num * 10.f) + (*it - '0');

				lhs = tree.add<Literal>(tok.view);

				break;
			}

			case TOKEN_LPAREN: {
				lhs = expr(lex, tree);

				if (lex.advance() != TOKEN_RPAREN) {
					std::cerr << "expected closing parenthesis\n";
					std::exit(-1);
				}

				break;
			}

			default: break;
		}

		while (true) {
			tok = lex.peek();

			auto [prec, assoc] = infix_bp[tok.type];

			if (prec == 0)
				break;

			if (prec <= bp)
				break;

			lex.advance();

			util::Node e = expr(lex, tree, prec + assoc);
			lhs = tree.add<BinaryOp>(tok, lhs, e);
		}

		return lhs;
	}
}


namespace calc {
	template <typename T>
	double eval(const T& variant, const calc::AST& tree) {
		return util::visit(variant,
			[&] (const BinaryOp& bop) {
				const auto& [op, lhs_node, rhs_node] = bop;

				auto lhs = eval(tree[lhs_node], tree);
				auto rhs = eval(tree[rhs_node], tree);

				switch (op.type) {
					case TOKEN_ADD:    return lhs + rhs;
					case TOKEN_SUB:    return lhs - rhs;
					case TOKEN_MUL:    return lhs * rhs;
					case TOKEN_DIV:    return lhs / rhs;
					case TOKEN_MOD:    return std::fmod(lhs, rhs);
					case TOKEN_POW:    return std::pow(lhs, rhs);
					// case TOKEN_LSHIFT: return lhs << rhs;
					// case TOKEN_RSHIFT: return lhs >> rhs;
					// case TOKEN_XOR:    return lhs ^ rhs;
					// case TOKEN_AND:    return lhs & rhs;
					// case TOKEN_OR:     return lhs | rhs;
					default: break;
				}

				return 0;
			},

			[&] (const UnaryOp& uop) {
				const auto& [op, node] = uop;

				switch (op.type) {
					case TOKEN_ADD: return +eval(tree[node], tree);
					case TOKEN_SUB: return -eval(tree[node], tree);
					// case TOKEN_NOT: return ~eval(tree[node], tree);
					default: break;
				}

				return 0;
			},

			[&] (const Literal& x) { return std::stod(x.view.str()); }
		);
	};
}


namespace calc {
	template <typename T>
	void print(const T& variant, const calc::AST& tree, std::string& str) {
		return util::visit(variant,
			[&] (const BinaryOp& bop) {
				const auto& [op, lhs_node, rhs_node] = bop;

				str += "( " + op.str() + " ";
					print(tree[lhs_node], tree, str);
				str += " ";
					print(tree[rhs_node], tree, str);
				str += " )";

				// return tinge::strcat("( ", op, " ", lhs, " ", rhs, " )");
			},

			[&] (const UnaryOp& uop) {
				const auto& [op, node] = uop;

				str += "( " + op.str() + " ";
					print(tree[node], tree, str);
				str += " )";

				// return tinge::strcat("( ", op, " ", print(tree[node], tree), " )");
			},

			[&] (const Literal& x) { str += x.view.str(); }
		);
	}


	std::string print(util::Node id, const calc::AST& tree) {
		std::string str;
		print(tree[id], tree, str);
		return str;
	}
}


namespace calc {
	std::vector<util::Node> parse(calc::Lexer& lex, calc::AST& tree) {
		std::vector<util::Node> roots;

		while (lex.peek() != calc::TOKEN_EOF)
			roots.emplace_back(calc::expr(lex, tree));

		return roots;
	}
}


int main(int argc, const char* argv[]) {
	if (argc != 2) {
		std::cerr << "usage: wpp <file>\n";
		return -1;
	}

	#ifdef BENCH
		// {
		// 	tinge::noticeln("generating data...");
		// 	std::vector<std::string> data;

		// 	for (int i = 0; i < 50'000; i++) {
		// 		data.emplace_back(calc::generate_expr());
		// 	}

		// 	tinge::noticeln(tinge::tab(), "done.");


		// 	using namespace std::chrono_literals;
		// 	auto bench = ankerl::nanobench::Bench().minEpochIterations(100'000).timeUnit(1us, "μs");

		// 	int i = 0;
		// 	bench.run("eval", [&] {
		// 		calc::AST tree;
		// 		calc::Lexer lex{data[i % data.size()].c_str()};

		// 		for (util::Node root: calc::parse(lex, tree)) {
		// 			ankerl::nanobench::doNotOptimizeAway(calc::eval(tree[root], tree));
		// 		}

		// 		i++;
		// 	});

		// 	tinge::noticeln(tinge::tab(), "done.");
		// }

	#else
		auto expr = util::read_file(argv[1]);

		calc::AST tree;
		calc::Lexer lex{expr.c_str()};

		auto roots = calc::parse(lex, tree);

		for (util::Node root: roots) {
			std::cout << calc::print(root, tree) << '\n';
			// tinge::successln(calc::eval(tree[root], tree));
		}

	#endif

	return 0;
}
