
#ifndef EXEC_IMPORTDELEGATE_H_
#define EXEC_IMPORTDELEGATE_H_

#include "src/binary-reader-interp.h"
#include "src/binary-reader.h"
#include "src/cast.h"
#include "src/error-handler.h"
#include "src/interp.h"
#include "src/option-parser.h"

extern std::unique_ptr<wabt::FileStream> s_stdout_stream;

class ImportDelegate: public wabt::interp::HostImportDelegate {
public:
	using Result = wabt::Result;
	using InterpResult = wabt::interp::Result;

	using FuncImport = wabt::interp::FuncImport;
	using Func = wabt::interp::Func;
	using FuncSignature = wabt::interp::FuncSignature;

	using TableImport = wabt::interp::TableImport;
	using Table = wabt::interp::Table;

	using MemoryImport = wabt::interp::MemoryImport;
	using Memory = wabt::interp::Memory;

	using GlobalImport = wabt::interp::GlobalImport;
	using Global = wabt::interp::Global;

	using HostFunc = wabt::interp::HostFunc;
	using TypedValue = wabt::interp::TypedValue;
	using Index = wabt::Index;

	wabt::Result ImportFunc(FuncImport* import, Func* func, FuncSignature* func_sig, const ErrorCallback& callback) override;
	wabt::Result ImportTable(TableImport* import, Table* table, const ErrorCallback& callback) override;
	wabt::Result ImportMemory(MemoryImport* import, Memory* memory, const ErrorCallback& callback) override;
	wabt::Result ImportGlobal(GlobalImport* import, Global* global, const ErrorCallback& callback) override;

private:
	static InterpResult PrintCallback(const HostFunc* func, const FuncSignature* sig, Index num_args, TypedValue* args,
			Index num_results, TypedValue* out_results, void* user_data);

	static InterpResult MyImportCallback(const HostFunc* func, const FuncSignature* sig, Index num_args, TypedValue* args,
			Index num_results, TypedValue* out_results, void* user_data);

	static InterpResult RustUnwindCallback(const HostFunc* func, const FuncSignature* sig, Index num_args, TypedValue* args,
			Index num_results, TypedValue* out_results, void* user_data);

	void PrintError(const ErrorCallback& callback, const char* format, ...);
};

#endif /* EXEC_IMPORTDELEGATE_H_ */
