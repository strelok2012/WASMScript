
#include "Module.h"
#include "Binary.h"

namespace wasm {

Func::Local::Local(Type t, Index count) : type(t), count(count) { }


Func::Func(Index sig) : sig(sig), resultsCount(0) { }


Module::Signature::Signature(Index param_count, Type* param_types, Index result_count, Type* result_types) {
	params.reserve(param_count);
	for (Index i = 0; i < param_count; ++ i) {
		params.emplace_back(param_types[i]);
	}

	results.reserve(result_count);
	for (Index i = 0; i < result_count; ++ i) {
		results.emplace_back(result_types[i]);
	}
}


Module::Import::Import(ExternalKind kind) : kind(kind) { }

Module::Import::Import(ExternalKind kind, StringView module, StringView field)
: kind(kind), module(module.data(), module.size()), field(field.data(), field.size()) { }


Module::FuncImport::FuncImport() : Import(ExternalKind::Func) { }

Module::FuncImport::FuncImport(StringView module, StringView field)
: Import(ExternalKind::Func, module, field) { }

Module::FuncImport::FuncImport(StringView module, StringView field, Index ind)
: Import(ExternalKind::Func, module, field), sig(ind) { }


Module::TableImport::TableImport() : Import(ExternalKind::Table) { }

Module::TableImport::TableImport(StringView module, StringView field)
: Import(ExternalKind::Table, module, field) { }

Module::TableImport::TableImport(StringView module, StringView field, Type t, const Limits &limits)
: Import(ExternalKind::Table, module, field), type(t), limits(limits) { }


Module::MemoryImport::MemoryImport() : Import(ExternalKind::Table) { }

Module::MemoryImport::MemoryImport(StringView module, StringView field)
: Import(ExternalKind::Table, module, field) { }

Module::MemoryImport::MemoryImport(StringView module, StringView field, const Limits &limits)
: Import(ExternalKind::Table, module, field), limits(limits) { }


Module::GlobalImport::GlobalImport() : Import(ExternalKind::Global) { }

Module::GlobalImport::GlobalImport(StringView module, StringView field)
: Import(ExternalKind::Global, module, field) { }

Module::GlobalImport::GlobalImport(StringView module, StringView field, Type t, bool m)
: Import(ExternalKind::Global, module, field), type(t), mutable_(m) { }


Module::ExceptImport::ExceptImport() : Import(ExternalKind::Except) { }

Module::ExceptImport::ExceptImport(StringView module, StringView field)
: Import(ExternalKind::Except, module, field) { }

Module::ExceptImport::ExceptImport(StringView module, StringView field, TypeVector &&vec)
: Import(ExternalKind::Except, module, field), sig(move(vec)) { }


Module::Table::Table(Type t, const Limits& limits) : type(t), limits(limits) { }

Module::Memory::Memory(const Limits& limits) : limits(limits) { }


Module::Global::Global(const TypedValue& value, bool mut)
: value(value), mut(mut) { }

Module::Global::Global(Type type, bool mut)
: value(type), mut(mut)  { }


Module::IndexObject::IndexObject(Index idx, bool import)
: index(idx), import(import) { }


Module::Export::Export(ExternalKind kind, IndexObject index, StringView name)
: kind(kind), index(index), name(name.data(), name.size()) { }


Module::Elements::Elements(Index t, Index off, Index capacity)
: table(t), offset(off) {
	values.reserve(capacity);
}


Module::Data::Data(Index m, Address offset, Address size, const uint8_t *bytes)
: memory(m), offset(offset) {
	data.assign(bytes, bytes + size);
}

bool Module::init(const uint8_t *data, size_t size, const ReadOptions &opts) {
	wasm::SourceReader reader;
	return reader.init(this, data, size, opts);
}

bool Module::hasMemory() const {
	return !_memoryIndex.empty();
}

bool Module::hasTable() const {
	return !_tableIndex.empty();
}

Module::Signature *Module::getSignature(Index idx) {
	if (idx < _types.size()) {
		return &_types[idx];
	}
	return nullptr;
}

Func * Module::getFunc(Index idx) {
	if (idx < _funcs.size()) {
		return &_funcs[idx];
	}
	return nullptr;
}
Module::Table * Module::getTable(Index idx) {
	if (idx < _tables.size()) {
		return &_tables[idx];
	}
	return nullptr;
}
Module::Memory * Module::getMemory(Index idx) {
	if (idx < _memory.size()) {
		return &_memory[idx];
	}
	return nullptr;
}
Module::Global * Module::getGlobal(Index idx) {
	if (idx < _globals.size()) {
		return &_globals[idx];
	}
	return nullptr;
}

const Module::FuncImport * Module::getImportFunc(Index idx) const {
	if (idx < _funcImports.size()) {
		return &_funcImports[idx];
	}
	return nullptr;
}
const Module::TableImport * Module::getImportTable(Index idx) const {
	if (idx < _tableImports.size()) {
		return &_tableImports[idx];
	}
	return nullptr;
}
const Module::MemoryImport * Module::getImportMemory(Index idx) const {
	if (idx < _memoryImports.size()) {
		return &_memoryImports[idx];
	}
	return nullptr;
}
const Module::GlobalImport * Module::getImportGlobal(Index idx) const {
	if (idx < _globalImports.size()) {
		return &_globalImports[idx];
	}
	return nullptr;
}

const Module::IndexObject *Module::getFunctionIndex(Index idx) const {
	if (idx < _funcIndex.size()) {
		return &_funcIndex[idx];
	}
	return nullptr;
}
const Module::IndexObject *Module::getGlobalIndex(Index idx) const {
	if (idx < _globalIndex.size()) {
		return &_globalIndex[idx];
	}
	return nullptr;
}
const Module::IndexObject *Module::getMemoryIndex(Index idx) const {
	if (idx < _memoryIndex.size()) {
		return &_memoryIndex[idx];
	}
	return nullptr;
}
const Module::IndexObject *Module::getTableIndex(Index idx) const {
	if (idx < _tableIndex.size()) {
		return &_tableIndex[idx];
	}
	return nullptr;
}

const Vector<Module::IndexObject> & Module::getFuncIndexVec() const {
	return _funcIndex;
}
const Vector<Module::IndexObject> & Module::getGlobalIndexVec() const {
	return _globalIndex;
}
const Vector<Module::IndexObject> & Module::getMemoryIndexVec() const {
	return _memoryIndex;
}
const Vector<Module::IndexObject> & Module::getTableIndexVec() const {
	return _tableIndex;
}

std::pair<Module::Signature *, bool> Module::getFuncSignature(Index idx) {
	if (auto obj = getFunctionIndex(idx)) {
		if (obj->import) {
			if (auto func = getImportFunc(obj->index)) {
				return std::pair<Module::Signature *, bool>(getSignature(func->sig), true);
			}
		} else {
			if (auto func = getFunc(obj->index)) {
				return std::pair<Module::Signature *, bool>(getSignature(func->sig), false);
			}
		}
	}
	return std::pair<Module::Signature *, bool>(nullptr, false);
}

std::pair<Type, bool> Module::getGlobalType(Index idx) {
	if (auto obj = getGlobalIndex(idx)) {
		if (obj->import) {
			if (auto g = getImportGlobal(obj->index)) {
				return std::pair<Type, bool>(g->type, g->mutable_);
			}
		} else {
			if (auto g = getGlobal(obj->index)) {
				return std::pair<Type, bool>(g->value.type, g->mut);
			}
		}
	}
	return std::pair<Type, bool>(Type::Void, false);
}

}
