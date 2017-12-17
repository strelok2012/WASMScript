
#ifndef EXEC_IMPORTDELEGATE_H_
#define EXEC_IMPORTDELEGATE_H_

#include "src/RuntimeEnvironment.h"
#include "SExpr.h"

namespace wasm {
namespace test {

class TestEnvironment : public Environment {
public:
	struct Test {
		String name;
		String data;
		Vector<sexpr::Token> list;
	};

	static TestEnvironment *getInstance();

	TestEnvironment();

	bool run();
	bool loadAsserts(const StringView &, const uint8_t *, size_t);

protected:
	bool runTest(wasm::ThreadedRuntime &, const Test &);

	HostModule *_testModule = nullptr;
	Vector<Test> _tests;
};

}
}

#endif /* EXEC_IMPORTDELEGATE_H_ */
