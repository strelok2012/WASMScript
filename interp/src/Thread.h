
#ifndef SRC_THREAD_H_
#define SRC_THREAD_H_

#include "Utils.h"

#define WABT_WARN_UNUSED __attribute__ ((warn_unused_result))

namespace wasm {

#define FOREACH_INTERP_RESULT(V)                                              \
	V(Ok, "ok")                                                               \
	/* returned from the top-most function */                                 \
	V(Returned, "returned")                                                   \
	/* memory access is out of bounds */                                      \
	V(TrapMemoryAccessOutOfBounds, "out of bounds memory access")             \
	/* atomic memory access is unaligned  */                                  \
	V(TrapAtomicMemoryAccessUnaligned, "atomic memory access is unaligned")   \
	/* converting from float -> int would overflow int */                     \
	V(TrapIntegerOverflow, "integer overflow")                                \
	/* dividend is zero in integer divide */                                  \
	V(TrapIntegerDivideByZero, "integer divide by zero")                      \
	/* converting from float -> int where float is nan */                     \
	V(TrapInvalidConversionToInteger, "invalid conversion to integer")        \
	/* function table index is out of bounds */                               \
	V(TrapUndefinedTableIndex, "undefined table index")                       \
	/* function table element is uninitialized */                             \
	V(TrapUninitializedTableElement, "uninitialized table element")           \
	/* unreachable instruction executed */                                    \
	V(TrapUnreachable, "unreachable executed")                                \
	/* call indirect signature doesn't match function table signature */      \
	V(TrapIndirectCallSignatureMismatch, "indirect call signature mismatch")  \
	/* ran out of call stack frames (probably infinite recursion) */          \
	V(TrapCallStackExhausted, "call stack exhausted")                         \
	/* ran out of value stack space */                                        \
	V(TrapValueStackExhausted, "value stack exhausted")                       \
	/* we called a host function, but the return value didn't match the */    \
	/* expected type */                                                       \
	V(TrapHostResultTypeMismatch, "host result type mismatch")                \
	/* we called an import function, but it didn't complete succesfully */    \
	V(TrapHostTrapped, "host function trapped")                               \
	/* we attempted to call a function with the an argument list that doesn't \
	* match the function signature */                                         \
	V(ArgumentTypeMismatch, "argument type mismatch")                         \
	/* we tried to get an export by name that doesn't exist */                \
	V(UnknownExport, "unknown export")                                        \
	/* the expected export kind doesn't match. */                             \
	V(ExportKindMismatch, "export kind mismatch")

class Thread {
public:
	enum class Result {
	#define V(Name, str) Name,
		FOREACH_INTERP_RESULT(V)
	#undef V
	};

	struct Options {
		static const uint32_t kDefaultValueStackSize = 512 * 1024
				/ sizeof(Value);
		static const uint32_t kDefaultCallStackSize = 64 * 1024;

		explicit Options(uint32_t value_stack_size = kDefaultValueStackSize,
				uint32_t call_stack_size = kDefaultCallStackSize);

		uint32_t value_stack_size;
		uint32_t call_stack_size;
	};

	explicit Thread(const Options& = Options());

	void Reset();
	Index NumValues() const {
		return value_stack_top_;
	}
	Result Push(Value) WABT_WARN_UNUSED;
	Value Pop();
	Value ValueAt(Index at) const;

	Result Run(int num_instructions = 1);

private:
	template<typename MemType>
	Result GetAccessAddress(const uint8_t** pc, void** out_address);

	template<typename MemType>
	Result GetAtomicAccessAddress(const uint8_t** pc, void** out_address);

	Value& Top();
	Value& Pick(Index depth);

	// Push/Pop values with conversions, e.g. Push<float> will convert to the
	// ValueTypeRep (uint32_t) and push that. Similarly, Pop<float> will pop the
	// value and convert to float.
	template<typename T>
	Result Push(T) WABT_WARN_UNUSED;
	template<typename T>
	T Pop();

	// Push/Pop values without conversions, e.g. Push<float> will take a uint32_t
	// argument which is the integer representation of that float value.
	// Similarly, PopRep<float> will not convert the value to a float.
	template<typename T>
	Result PushRep(ValueTypeRep<T>) WABT_WARN_UNUSED;
	template<typename T>
	ValueTypeRep<T> PopRep();

	void DropKeep(uint32_t drop_count, uint8_t keep_count);

	Result PushCall(const uint8_t* pc) WABT_WARN_UNUSED;
	Offset PopCall();

	template<typename R, typename T> using UnopFunc = R(T);
	template<typename R, typename T> using UnopTrapFunc = Result(T, R*);
	template<typename R, typename T> using BinopFunc = R(T, T);
	template<typename R, typename T> using BinopTrapFunc = Result(T, T, R*);

	template<typename MemType, typename ResultType = MemType>
	Result Load(const uint8_t** pc) WABT_WARN_UNUSED;
	template<typename MemType, typename ResultType = MemType>
	Result Store(const uint8_t** pc) WABT_WARN_UNUSED;
	template<typename MemType, typename ResultType = MemType>
	Result AtomicLoad(const uint8_t** pc) WABT_WARN_UNUSED;
	template<typename MemType, typename ResultType = MemType>
	Result AtomicStore(const uint8_t** pc) WABT_WARN_UNUSED;
	template<typename MemType, typename ResultType = MemType>
	Result AtomicRmw(BinopFunc<ResultType, ResultType>, const uint8_t** pc)
			WABT_WARN_UNUSED;
	template<typename MemType, typename ResultType = MemType>
	Result AtomicRmwCmpxchg(const uint8_t** pc) WABT_WARN_UNUSED;

	template<typename R, typename T = R>
	Result Unop(UnopFunc<R, T> func) WABT_WARN_UNUSED;
	template<typename R, typename T = R>
	Result UnopTrap(UnopTrapFunc<R, T> func) WABT_WARN_UNUSED;

	template<typename R, typename T = R>
	Result Binop(BinopFunc<R, T> func) WABT_WARN_UNUSED;
	template<typename R, typename T = R>
	Result BinopTrap(BinopTrapFunc<R, T> func) WABT_WARN_UNUSED;

	std::vector<Value> value_stack_;
	uint32_t value_stack_top_ = 0;
	uint32_t call_stack_top_ = 0;
};

}

#endif /* SRC_THREAD_H_ */
