
#ifndef EXEC_SEXPR_H_
#define EXEC_SEXPR_H_

#include "src/Utils.h"

namespace sexpr {

using StringView = wasm::StringView;

template <typename T>
using Vector = wasm::Vector<T>;

struct Token {
	enum Kind {
		Word,
		List
	};

	Token() = default;

	explicit Token(StringView t) : kind(Word), token(t) { }
	explicit Token(Kind k) : kind(k) { }

	Kind kind = Word;
	StringView token;
	Vector<Token> vec;
};

Vector<Token> parse(StringView);
void print(std::ostream &, const Token &);
void print(std::ostream &, const Vector<Token> &);

}

#endif /* EXEC_SEXPR_H_ */
