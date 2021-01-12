#pragma once

#ifndef CALC_UTIL_HPP
#define CALC_UTIL_HPP

#include <utility>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <vector>
#include <variant>

#include <tinge.hpp>


namespace util {
	inline std::string read_file(const std::string& fname) {
		auto filesize = std::filesystem::file_size(fname);
		std::ifstream is(fname, std::ios::binary);

		auto str = std::string(filesize + 1, '\0');
		is.read(str.data(), static_cast<std::streamsize>(filesize));

		return str;
	}
}


namespace util {
	struct View {
		const char *begin = nullptr, *end = nullptr;

		constexpr View() {}

		constexpr View(const char* const begin_, const char* const end_):
			begin(begin_), end(end_) {}
	};

	inline std::ostream& operator<<(std::ostream& os, const View& v) {
		const auto& [vbegin, vend] = v;
		os.write(vbegin, vend - vbegin);
		return os;
	}
}


namespace util {
	struct Token {
		View view{};
		uint8_t type = 0;

		constexpr Token() {}

		constexpr Token(View view_, uint8_t type_):
			view(view_), type(type_) {}
	};

	constexpr bool operator==(const Token& t, const uint8_t type) {
		return t.type == type;
	}

	constexpr bool operator!=(const Token& t, const uint8_t type) {
		return not(t == type);
	}

	inline std::ostream& operator<<(std::ostream& os, const Token& t) {
		const auto& [view, type] = t;
		return (os << view);
	}
}


namespace util {
	template <typename... Ts>
	constexpr bool in_group(char c, Ts&&... args) {
		return ((c == args) or ...);
	}

	constexpr bool in_range(char c, char lower, char upper) {
		return c >= lower and c <= upper;
	}


	constexpr bool is_digit(char c) {
		return in_range(c, '0', '9');
	}

	constexpr bool is_whitespace(char c) {
		return in_group(c, ' ', '\n', '\t', '\v', '\f');
	}
}


namespace util {
	// struct Node {
	// 	int64_t index = 0;

	// 	operator int64_t() {
	// 		return index;
	// 	}
	// };


	using Node = int64_t;
	constexpr Node NODE_EMPTY = -1;

	template <typename... Ts>
	class AST: public std::vector<std::variant<Ts...>> {
		using std::vector<std::variant<Ts...>>::vector;

		public:
			template <typename T, typename... Xs>
			Node add(Xs&&... args) {
				this->emplace_back(T{ std::forward<Xs>(args)... });
				// this->emplace_back(std::in_place_type<T>, std::forward<Xs>(args)...);
				return { static_cast<int64_t>(this->size() - 1) };
			}
	};
}


namespace util {
	template <Token next_token(const char*&), int lookahead = 1>
	class Lexer {
		private:
			const char* str = nullptr;
			std::array<Token, lookahead> tokens{};
			unsigned head = 0;


		public:
			Lexer(const char* const str_): str{str_} {
				for (int j = 0; j < lookahead; j++)
					advance();
			}


		public:
			const Token& peek(int n = 0) const {
				return tokens[(head + n) % lookahead];
			}

			Token advance() {
				Token tok = peek();
				tokens[head] = next_token(str);
				head = (head + 1) % lookahead;
				return tok;
			}
	};
}


namespace util {
	template <typename... Ts> struct overloaded: Ts... { using Ts::operator()...; };
	template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

	template <typename V, typename... Ts>
	constexpr decltype(auto) visit(V& variant, Ts&&... args) {
		return std::visit(util::overloaded{
			std::forward<Ts>(args)...
		}, variant);
	}
}


#endif
