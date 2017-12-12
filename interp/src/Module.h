
#ifndef SRC_MODULE_H_
#define SRC_MODULE_H_

#include "Utils.h"
#include "Opcode.h"

namespace wasm {

struct Func {
	struct Local {
		Local(Type, Index);

		Type type;
		Index count;
	};

	struct OpcodeRec {
		Opcode::Enum opcode;
		union {
			struct {
				uint32_t v1;
				uint32_t v2;
			} value32;
			uint64_t value64;
		};

		OpcodeRec(Opcode::Enum op, uint32_t v1, uint32_t v2) noexcept
		: opcode(op) {
			value32.v1 = v1;
			value32.v2 = v2;
		}

		OpcodeRec(Opcode::Enum op, uint64_t v1) noexcept
		: opcode(op) {
			value64 = v1;
		}

		OpcodeRec(const OpcodeRec &) = default;
	};

	struct Label {
		Label(Index results, Index stack)
		: results(results), stack(stack) { }

		Label(Index results, Index stack, Index offset)
		: results(results), stack(stack), offset(offset) { }

		Label(const Label &) = default;

		Index results = 0;
		Index stack = 0;
		Index offset = 0;
	};

	Func(Index sig);

	Index sig;
	Index resultsCount;
	Vector<Type> types;
	Vector<OpcodeRec> opcodes;
	Vector<Label> labels;
	Vector<Index> jumpTable;
};

// Module should store only headers and constant data, not runtime data
class Module {
public:
	struct Signature {
		Signature() = default;
		Signature(Index param_count, Type* param_types, Index result_count, Type* result_types);

		Vector<Type> params;
		Vector<Type> results;
	};

	struct Import {
		explicit Import(ExternalKind kind);
		Import(ExternalKind kind, StringView module, StringView field);

		ExternalKind kind;
		String module;
		String field;
	};

	struct FuncImport : Import {
		FuncImport();
		FuncImport(StringView module, StringView field);
		FuncImport(StringView module, StringView field, Index ind);

		Index sig = kInvalidIndex;
	};

	struct TableImport : Import {
		TableImport();
		TableImport(StringView module, StringView field);
		TableImport(StringView module, StringView field, Type, const Limits &);

		Type type = Type::Anyfunc;
		Limits limits;
	};

	struct MemoryImport : Import {
		MemoryImport();
		MemoryImport(StringView module, StringView field);
		MemoryImport(StringView module, StringView field, const Limits &);

		Limits limits;
	};

	struct GlobalImport: Import {
		GlobalImport();
		GlobalImport(StringView module, StringView field);
		GlobalImport(StringView module, StringView field, Type, bool);

		Type type = Type::Void;
		bool mutable_ = false;
	};

	struct ExceptImport : Import {
		ExceptImport();
		ExceptImport(StringView module, StringView field);
		ExceptImport(StringView module, StringView field, TypeVector &&);

		TypeVector sig;
	};

	struct Table {
		explicit Table(Type, const Limits& limits);

		Type type;
		Limits limits;
	};

	struct Memory {
		explicit Memory(const Limits& limits);

		Limits limits;
	};

	struct Global {
		Global() = default;
		Global(const TypedValue& value, bool mut);
		Global(Type value, bool mut);

		TypedValue value;
		bool mut = false;
		Index import = kInvalidIndex; /* or INVALID_INDEX if not imported */
	};

	struct IndexObject {
		IndexObject() = default;
		IndexObject(Index, bool);

		Index index = kInvalidIndex;
		bool import = false;
	};

	struct Export {
		Export(ExternalKind, IndexObject, StringView);

		ExternalKind kind = ExternalKind::Func;
		IndexObject index;
		String name;
	};

	struct Elements {
		Elements(Index, Index, Index capacity);

		Index table;
		Index offset;
		Vector<Index> values;
	};

	struct Data {
		Data(Index, Address offset, Address size, const uint8_t *data);

		Index memory;
		Address offset;
		Vector<uint8_t> data;
	};

	bool init(const uint8_t *, size_t, const ReadOptions & = ReadOptions());

	bool hasMemory() const;
	bool hasTable() const;

	Signature *getSignature(Index);

	Func * getFunc(Index);
	Table * getTable(Index);
	Memory * getMemory(Index);
	Global * getGlobal(Index);

	const FuncImport * getImportFunc(Index) const;
	const TableImport * getImportTable(Index) const;
	const MemoryImport * getImportMemory(Index) const;
	const GlobalImport * getImportGlobal(Index) const;

	const IndexObject *getFunctionIndex(Index) const;
	const IndexObject *getGlobalIndex(Index) const;
	const IndexObject *getMemoryIndex(Index) const;
	const IndexObject *getTableIndex(Index) const;

	const Vector<IndexObject> & getFuncIndexVec() const;
	const Vector<IndexObject> & getGlobalIndexVec() const;
	const Vector<IndexObject> & getMemoryIndexVec() const;
	const Vector<IndexObject> & getTableIndexVec() const;

	std::pair<Signature *, bool> getFuncSignature(Index);
	std::pair<Type, bool> getGlobalType(Index); // Type, mutable

protected:
	friend class SourceReader;

	size_t _typeCount;
	Vector<Signature> _types;
	Vector<FuncImport> _funcImports;
	Vector<TableImport> _tableImports;
	Vector<MemoryImport> _memoryImports;
	Vector<GlobalImport> _globalImports;
	Vector<ExceptImport> _exceptImports;

	Vector<Func> _funcs;
	Vector<Table> _tables;
	Vector<Memory> _memory;
	Vector<Global> _globals;

	Vector<IndexObject> _funcIndex;
	Vector<IndexObject> _globalIndex;
	Vector<IndexObject> _memoryIndex;
	Vector<IndexObject> _tableIndex;

	Vector<Export> _exports;
	Vector<Elements> _elements;
	Vector<Data> _data;

	IndexObject _startFunction;
};

}

#endif /* SRC_MODULE_H_ */
