
#ifndef SRC_RUNTIMEENVIRONMENT_H_
#define SRC_RUNTIMEENVIRONMENT_H_

#include "Environment.h"
#include "Thread.h"

namespace wasm {

struct LinkingThreadOptions : LinkingPolicy {
	uint32_t valueStackSize = Thread::kDefaultValueStackSize;
	uint32_t callStackSize = Thread::kDefaultCallStackSize;
};

class ThreadedRuntime : public Runtime {
public:
	virtual ~ThreadedRuntime() { }
	ThreadedRuntime();

	bool init(const Environment *, const LinkingThreadOptions & = LinkingThreadOptions());

	const Func *getExportFunc(const StringView &module, const StringView &name) const;
	const Func *getExportFunc(const RuntimeModule &module, const StringView &name) const;

	const RuntimeGlobal *getGlobal(const StringView &module, const StringView &name) const;
	const RuntimeGlobal *getGlobal(const RuntimeModule &module, const StringView &name) const;

	bool setGlobal(const StringView &module, const StringView &name, const Value &);
	bool setGlobal(const RuntimeModule &module, const StringView &name, const Value &);

	bool call(const RuntimeModule &module, const Func &, Vector<Value> &paramsInOut);
	bool call(const RuntimeModule &module, const Func &, Value *paramsInOut);

	bool call(const Func &, Vector<Value> &paramsInOut);
	bool call(const Func &, Value *paramsInOut);

	Thread::Result callSafe(const RuntimeModule &module, const Func &, Vector<Value> &paramsInOut);
	Thread::Result callSafe(const RuntimeModule &module, const Func &, Value *paramsInOut);

	Thread::Result callSafe(const Func &, Vector<Value> &paramsInOut);
	Thread::Result callSafe(const Func &, Value *paramsInOut);

	virtual void onError(StringStream &) const;
	virtual void onThreadError(const Thread &) const;

protected:
	bool _silent = false;
	Thread _mainThread;
};

}

#endif /* SRC_RUNTIMEENVIRONMENT_H_ */
