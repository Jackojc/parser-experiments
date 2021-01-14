#include <string>
#include <iostream>
#include <vector>
#include <util.hpp>
#include <tinge.hpp>




struct Opt {
	char val;

	auto get() const {
		return val;
	}
};


struct LongOpt {
	std::string val;

	auto get() const {
		return val;
	}
};


struct Positional {
	std::string val;

	auto get() const {
		return val;
	}
};



struct Context {
	int& i;
	const char*& ptr;
	const char* const* argv;

	std::string get_next() {
		return std::string{argv[i++ + 1]};
	}
};



using Value = std::variant<LongOpt, Opt, Positional>;




template <typename F>
void consume(int argc, const char* argv[], const F& func) {
	int i = 1;
	const char* ptr = argv[i];

	Context ctx{i, ptr, argv};

	for (; i < argc; i++, ptr = argv[i]) {
		int dashes = 0;
		while (*ptr == '-') {
			dashes++;
			ptr++;
		}


		switch (dashes) {
			case 0: {

				std::string str;

				while (*ptr) {
					str += *ptr++;
				}

				func(Positional{str}, ctx);

			} break;

			case 1: {

				while (*ptr) {
					func(Opt{*ptr++}, ctx);
				}

			} break;

			case 2: {

				std::string str;

				while (*ptr) {
					str += *ptr++;
				}

				func(LongOpt{str}, ctx);

			} break;

		}
	}
}


/*


	// called for each option
	template <typename T>
	void handle_arguments(const T& variant, Context& ctx) {
		util::visit(variant,
			// -a -b -cd
			[&] (Short variant, Context ctx) {
				switch (variant.val) {
					case 'a': break;
					case 'b': break;
					case 'c': break;
				}
			},

			// --name jack --hey
			[&] (Long variant, Context ctx) {
				if (variant.val == "name") {
					std::string name = ctx.get_next();
					return;
				}
			},

			// file.txt
			[&] (Pos variant, Context ctx) {

			}
		);
	}


	consume(argc, argv, handle_arguments);





	consume(argc, argv,
		[&] (Short variant, Context ctx) {
			switch (variant.val) {
				case 'a': break;
				case 'b': break;
				case 'c': break;
			}
		},

		// --name jack --hey
		[&] (Long variant, Context ctx) {
			if (variant.val == "name") {
				std::string name = ctx.get_next();
				return;
			}
		},

		// file.txt
		[&] (Pos variant, Context ctx) {

		}
	);


*/


int main(int argc, const char* argv[]) {
	consume(argc, argv, [&] (const Value& variant, Context& ctx) {
		util::visit(variant,
			[&] (const Opt& x) {
				std::cout << "short: " << x.get() << '\n';
			},

			[&] (const LongOpt& x) {
				std::cout << "long: " << x.get() << " = " << ctx.get_next() << '\n';
			},

			[&] (const Positional& x) {
				std::cout << "pos: " << x.get() << '\n';
			}
		);
	});

	return 0;
}
