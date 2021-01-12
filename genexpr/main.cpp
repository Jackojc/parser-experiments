#include <string>
#include <iostream>

#include <cstdint>
#include <ctime>

#include <util.hpp>
#include <tinge.hpp>

namespace rng {
	namespace detail {
		constexpr uint64_t splitmix64(uint64_t seed) {
			seed = (seed ^ (seed >> 30)) * 0xBF58476D1CE4E5B9;
			seed = (seed ^ (seed >> 27)) * 0x94D049BB133111EB;
			return seed ^ (seed >> 31);
		}

		constexpr uint64_t rotl(uint64_t x, int k) {
			return (x << k) | (x >> (64 - k));
		}
	}

	struct Random {
		uint64_t state[4];
	};

	constexpr Random random_create(uint64_t seed) {
		Random state {};

		seed = detail::splitmix64(seed);
		state.state[0] = seed;

		seed = detail::splitmix64(seed);
		state.state[1] = seed;

		seed = detail::splitmix64(seed);
		state.state[2] = seed;

		seed = detail::splitmix64(seed);
		state.state[3] = seed;

		return state;
	}

	constexpr uint64_t random_next(Random& rng) {
		auto& s = rng.state;

		const uint64_t result = detail::rotl(s[0] + s[3], 23) + s[0];
		const uint64_t t = s[1] << 17;

		s[2] ^= s[0];
		s[3] ^= s[1];
		s[1] ^= s[2];
		s[0] ^= s[3];

		s[2] ^= t;
		s[3] = detail::rotl(s[3], 45);

		return result;
	}

	constexpr uint64_t random_range(Random& rng, uint64_t min, uint64_t max) {
		uint64_t range = max - min + 1;
		uint64_t x = 0, r = 0;

		do {
			x = random_next(rng);
			r = x;

			if (r >= range) {
				r -= range;

				if (r >= range)
					r %= range;
			}

		} while (x - r > -range);

		return r + min;
	}
}


namespace gexpr {
	inline void generate_literal(rng::Random&, std::string&);
	inline void generate_expr(rng::Random&, std::string&, const int = 100, int = 0);
	inline void generate_unary_expr(rng::Random&, std::string&, const int = 100, int = 0);
	inline void generate_binary_expr(rng::Random&, std::string&, const int = 100, int = 0);
	inline void generate_paren_expr(rng::Random&, std::string&, const int = 100, int = 0);


	inline void generate_expr(rng::Random& rng, std::string& str, const int max_depth, int depth) {
		if (str.size() >= 1'000'000'000) {
			std::cout << str;
			str.clear();
		}

		switch (rng::random_next(rng) % 4) {
			case 0: generate_unary_expr(rng, str, max_depth, depth + 1); break;
			case 1: generate_binary_expr(rng, str, max_depth, depth + 1); break;
			case 2: generate_expr(rng, str, max_depth, depth + 1); break;
			case 3: generate_paren_expr(rng, str, max_depth, depth + 1); break;
		}
	}

	inline void generate_literal(rng::Random& rng, std::string& str) {
		str += std::to_string(rng::random_range(rng, 1, 100));
	}

	inline void generate_binary_expr(rng::Random& rng, std::string& str, const int max_depth, int depth) {
		constexpr auto ops = std::array {
			" + ", " - ", " * ", " / ", " % ", " ** "
		};

		const auto op = ops[rng::random_next(rng) % ops.size()];

		if (depth + 1 >= max_depth) {
			generate_literal(rng, str);
			str += op;
			generate_literal(rng, str);
		}

		else {
			generate_expr(rng, str, max_depth, depth + 1);
			str += op;
			generate_expr(rng, str, max_depth, depth + 1);
		}
	}

	inline void generate_unary_expr(rng::Random& rng, std::string& str, const int max_depth, int depth) {
		constexpr auto ops = std::array {
			"+", "-"
		};

		const auto op = ops[rng::random_next(rng) % ops.size()];

		if (depth + 1 >= max_depth) {
			str += op;
			generate_literal(rng, str);
		}

		else {
			str += op;
			generate_expr(rng, str, max_depth, depth + 1);
		}
	}

	inline void generate_paren_expr(rng::Random& rng, std::string& str, const int max_depth, int depth) {
		str += "(";
		generate_binary_expr(rng, str, max_depth, depth + 1);
		str += ")";
	}
}

int main(int argc, const char* argv[]) {
	if (argc != 2) {
		std::cerr << "usage: genexpr <n>\n";
		return -1;
	}

	rng::Random rng = rng::random_create(time(nullptr));

	std::string out;
	out.reserve(1'000'000'000);
	gexpr::generate_expr(rng, out, std::stoi(argv[1]));

	if (out.size() > 0) {
		std::cout << out << '\n';
	}
	return 0;
}
