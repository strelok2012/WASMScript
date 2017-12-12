
#include "Binary.h"
#include "Module.h"

#define PRINT_CONTENT 0

#if (PRINT_CONTENT)
#define BINARY_PRINTF(...) printf(__VA_ARGS__)
#else
#define BINARY_PRINTF
#endif

namespace wasm {

static void printType(StringStream &stream, Type t) {
	switch (t) {
	case Type::I32: stream << "i32"; break;
	case Type::I64: stream << "i64"; break;
	case Type::F32: stream << "f32"; break;
	case Type::F64: stream << "f64"; break;
	case Type::Anyfunc: stream << "anyfunc"; break;
	case Type::Func: stream << "func"; break;
	case Type::Void: stream << "void"; break;
	default: stream << "unknown"; break;
	}
}


/* Type section */
Result SourceReader::BeginTypeSection(Offset size) { return Result::Ok; }
Result SourceReader::OnTypeCount(Index count) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, count);
	_targetModule->_types.reserve(count);

	return Result::Ok;
}
Result SourceReader::OnType(Index index, Index nparam, Type* param, Index nresult, Type* result) {
	if (index == _targetModule->_types.size()) {
#if (PRINT_CONTENT)
		StringStream stream; stream << __FUNCTION__ << " [" << index << "]";

		for (Index i = 0; i < nparam; ++ i) {
			stream << " (param ";
			printType(stream, param[i]);
			stream << ")";
		}

		for (Index i = 0; i < nresult; ++ i) {
			stream << " (return ";
			printType(stream, result[i]);
			stream << ")";
		}

		stream << "\n";

		BINARY_PRINTF("%s", stream.str().data());
#endif

		_targetModule->_types.emplace_back(nparam, param, nresult, result);
		return Result::Ok;
	}
	return Result::Error;
}
Result SourceReader::EndTypeSection() { return Result::Ok; }


/* Import section */
Result SourceReader::BeginImportSection(Offset size) { return Result::Ok; }
Result SourceReader::OnImportCount(Index count) { return Result::Ok; }
Result SourceReader::OnImport(Index index, StringView module, StringView field) { return Result::Ok; }
Result SourceReader::OnImportFunc(Index import, StringView module, StringView field, Index func, Index sig) {
	BINARY_PRINTF("%s %u %.*s %.*s %u %u\n", __FUNCTION__, import, (int)module.size(), module.data(), (int)field.size(), field.data(), func, sig);

	_targetModule->_funcImports.emplace_back(module, field, sig);
	_targetModule->_funcIndex.emplace_back(_targetModule->_funcImports.size() - 1, true);
	return Result::Ok;
}
Result SourceReader::OnImportTable(Index import, StringView module, StringView field, Index table, Type elem, const Limits* elemLimits) {
	BINARY_PRINTF("%s %u %.*s %.*s %u %d (%lu %lu)\n", __FUNCTION__, import, (int)module.size(), module.data(), (int)field.size(), field.data(), table, elem
			, elemLimits->initial, elemLimits->max);

	_targetModule->_tableImports.emplace_back(module, field, elem, *elemLimits);
	_targetModule->_tableIndex.emplace_back(_targetModule->_tableImports.size() - 1, true);
	return Result::Ok;
}
Result SourceReader::OnImportMemory(Index import, StringView module, StringView field, Index memory, const Limits* pageLimits) {
	BINARY_PRINTF("%s %u %.*s %.*s %u (%lu %lu)\n", __FUNCTION__, import, (int)module.size(), module.data(), (int)field.size(), field.data(), memory
				, pageLimits->initial, pageLimits->max);

	_targetModule->_memoryImports.emplace_back(module, field, *pageLimits);
	_targetModule->_memoryIndex.emplace_back(_targetModule->_memoryImports.size() - 1, true);
	return Result::Ok;
}
Result SourceReader::OnImportGlobal(Index import, StringView module, StringView field, Index global, Type type, bool mut) {
	BINARY_PRINTF("%s %u %.*s %.*s %u %d (%d)\n", __FUNCTION__, import, (int)module.size(), module.data(), (int)field.size(), field.data(), global, type, mut);

	_targetModule->_globalImports.emplace_back(module, field, type, mut);
	_targetModule->_globalIndex.emplace_back(_targetModule->_globalImports.size() - 1, true);
	return Result::Ok;
}
Result SourceReader::OnImportException(Index import, StringView module, StringView field, Index except, TypeVector& sig) {
	BINARY_PRINTF("%s %u %.*s %.*s %u\n", __FUNCTION__, import, (int)module.size(), module.data(), (int)field.size(), field.data(), except);

	_targetModule->_exceptImports.emplace_back(module, field, move(sig));
	return Result::Ok;
}
Result SourceReader::EndImportSection() { return Result::Ok; }


/* Function section */
Result SourceReader::BeginFunctionSection(Offset size) { return Result::Ok; }
Result SourceReader::OnFunctionCount(Index count) {
	_targetModule->_funcs.reserve(count);
	return Result::Ok;
}
Result SourceReader::OnFunction(Index index, Index sig) {
	BINARY_PRINTF("%s %u %u\n", __FUNCTION__, index, sig);
	_targetModule->_funcs.emplace_back(sig);
	_targetModule->_funcIndex.emplace_back(_targetModule->_funcs.size() - 1, false);
	return Result::Ok;
}
Result SourceReader::EndFunctionSection() { return Result::Ok; }


/* Table section */
Result SourceReader::BeginTableSection(Offset size) { return Result::Ok; }
Result SourceReader::OnTableCount(Index count) {
	_targetModule->_tables.reserve(count);
	return Result::Ok;
}
Result SourceReader::OnTable(Index index, Type type, const Limits* limits) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	_targetModule->_tables.emplace_back(type, *limits);
	_targetModule->_tableIndex.emplace_back(_targetModule->_tables.size() - 1, false);
	return Result::Ok;
}
Result SourceReader::EndTableSection() { return Result::Ok; }


/* Memory section */
Result SourceReader::BeginMemorySection(Offset size) { return Result::Ok; }
Result SourceReader::OnMemoryCount(Index count) {
	_targetModule->_memory.reserve(count);
	return Result::Ok;
}
Result SourceReader::OnMemory(Index index, const Limits* limits) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, index);
	_targetModule->_memory.emplace_back(*limits);
	_targetModule->_memoryIndex.emplace_back(_targetModule->_memory.size() - 1, false);
	return Result::Ok;
}
Result SourceReader::EndMemorySection() { return Result::Ok; }


/* Global section */
Result SourceReader::BeginGlobalSection(Offset size) { return Result::Ok; }
Result SourceReader::OnGlobalCount(Index count) {
	_targetModule->_globals.reserve(count);
	return Result::Ok;
}
Result SourceReader::BeginGlobal(Index index, Type type, bool mut) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	_targetModule->_globals.emplace_back(type, mut);
	return Result::Ok;
}
Result SourceReader::BeginGlobalInitExpr(Index index) {
	_initExprValue.type = Type::Void;
	return Result::Ok;
}
Result SourceReader::EndGlobalInitExpr(Index index) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	Module::Global* global = _targetModule->getGlobal(index);
	if (_initExprValue.type != global->value.type) {
		StringStream stream;
		stream << "type mismatch in global, expected ";
		printType(stream, global->value.type);
		stream << " but got ";
		printType(stream, _initExprValue.type);
		stream << ".";
		OnError(stream);
		//return Result::Error;
	}
	global->value = _initExprValue;
	return Result::Ok;
}
Result SourceReader::EndGlobal(Index index) { return Result::Ok; }
Result SourceReader::EndGlobalSection() { return Result::Ok; }


/* Exports section */
Result SourceReader::BeginExportSection(Offset size) { return Result::Ok; }
Result SourceReader::OnExportCount(Index count) {
	_targetModule->_exports.reserve(count);
	return Result::Ok;
}
Result SourceReader::OnExport(Index index, ExternalKind kind, Index item_index, StringView name) {
	switch (kind) {
	case ExternalKind::Func:
		BINARY_PRINTF("%s func %u %u %.*s\n", __FUNCTION__, index, item_index, (int)name.size(), name.data());
		if (item_index < _targetModule->_funcIndex.size()) {
			_targetModule->_exports.emplace_back(kind, _targetModule->_funcIndex[item_index], name);
		} else {
			PushErrorStream([&] (StringStream &stream) {
				stream << "No func object for export found";
			});
			return Result::Error;
		}
		break;

	case ExternalKind::Table:
		BINARY_PRINTF("%s table %u %u %.*s\n", __FUNCTION__, index, item_index, (int)name.size(), name.data());
		if (item_index < _targetModule->_tableIndex.size()) {
			_targetModule->_exports.emplace_back(kind, _targetModule->_tableIndex[item_index], name);
		} else {
			PushErrorStream([&] (StringStream &stream) {
				stream << "No table object for export found";
			});
			return Result::Error;
		}
		break;

	case ExternalKind::Memory:
		BINARY_PRINTF("%s memory %u %u %.*s\n", __FUNCTION__, index, item_index, (int)name.size(), name.data());
		if (item_index < _targetModule->_memoryIndex.size()) {
			_targetModule->_exports.emplace_back(kind, _targetModule->_memoryIndex[item_index], name);
		} else {
			PushErrorStream([&] (StringStream &stream) {
				stream << "No memory object for export found";
			});
			return Result::Error;
		}
		break;

	case ExternalKind::Global:
		BINARY_PRINTF("%s global %u %u %.*s\n", __FUNCTION__, index, item_index, (int)name.size(), name.data());
		if (item_index < _targetModule->_globalIndex.size()) {
			_targetModule->_exports.emplace_back(kind, _targetModule->_globalIndex[item_index], name);
		} else {
			PushErrorStream([&] (StringStream &stream) {
				stream << "No global object for export found";
			});
			return Result::Error;
		}
		break;

	case ExternalKind::Except:
		// TODO(karlschimpf) Define
		PushErrorStream([&] (StringStream &stream) {
			stream << "BinaryReaderInterp::OnExport(except) not implemented";
		});
		return Result::Error;
		break;

	default:
		break;
	}
	return Result::Ok;
}
Result SourceReader::EndExportSection() { return Result::Ok; }


/* Start section */
Result SourceReader::BeginStartSection(Offset size) { return Result::Ok; }
Result SourceReader::OnStartFunction(Index func_index) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, func_index);
	if (func_index < _targetModule->_funcIndex.size()) {
		_targetModule->_startFunction = _targetModule->_funcIndex[func_index];
	} else {
		PushErrorStream([&] (StringStream &stream) { stream << "No func object for start function found"; });
		return Result::Error;
	}
	return Result::Ok;
}
Result SourceReader::EndStartSection() { return Result::Ok; }


}
