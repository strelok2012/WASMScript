
#include <iostream>
/*
#include "ImportDelegate.h"

using namespace wabt;
using namespace wabt::interp;

#define PRIimport "\"" PRIstringview "." PRIstringview "\""
#define PRINTF_IMPORT_ARG(x) WABT_PRINTF_STRING_VIEW_ARG((x).module_name) , WABT_PRINTF_STRING_VIEW_ARG((x).field_name)

wabt::Result ImportDelegate::ImportFunc(FuncImport* import, Func* func, FuncSignature* func_sig, const ErrorCallback& callback) {
	if (import->field_name == "print") {
		cast<HostFunc>(func)->callback = PrintCallback;
		return wabt::Result::Ok;
	} else if (import->field_name == "import_function") {
		cast<HostFunc>(func)->callback = MyImportCallback;
		return wabt::Result::Ok;
	} else if (import->field_name == "rust_begin_unwind") {
		cast<HostFunc>(func)->callback = RustUnwindCallback;
		return wabt::Result::Ok;
	} else {
		PrintError(callback, "unknown host function import " PRIimport,
				PRINTF_IMPORT_ARG(*import));
		return wabt::Result::Error;
	}
}

wabt::Result ImportDelegate::ImportTable(TableImport* import, Table* table, const ErrorCallback& callback) {
	return wabt::Result::Error;
}

wabt::Result ImportDelegate::ImportMemory(MemoryImport* import, Memory* memory, const ErrorCallback& callback) {
	return wabt::Result::Error;
}

wabt::Result ImportDelegate::ImportGlobal(GlobalImport* import, Global* global, const ErrorCallback& callback) {
	return wabt::Result::Error;
}


ImportDelegate::InterpResult ImportDelegate::PrintCallback(const HostFunc* func, const FuncSignature* sig, Index num_args, TypedValue* args,
			Index num_results, TypedValue* out_results, void* user_data) {
	memset(out_results, 0, sizeof(TypedValue) * num_results);
	for (Index i = 0; i < num_results; ++i)
		out_results[i].type = sig->result_types[i];

	TypedValues vec_args(args, args + num_args);
	TypedValues vec_results(out_results, out_results + num_results);

	printf("called host ");
	WriteCall(s_stdout_stream.get(), func->module_name, func->field_name,
			vec_args, vec_results, InterpResult::Ok);
	return InterpResult::Ok;
}

ImportDelegate::InterpResult ImportDelegate::MyImportCallback(const HostFunc* func, const FuncSignature* sig, Index num_args, TypedValue* args,
			Index num_results, TypedValue* out_results, void* user_data) {
	memset(out_results, 0, sizeof(TypedValue) * num_results);
	for (Index i = 0; i < num_results; ++i)
		out_results[i].type = sig->result_types[i];

	TypedValues vec_args(args, args + num_args);
	TypedValues vec_results(out_results, out_results + num_results);

	std::cout << "Called my import function with args: \r\n";
	for (auto &res : vec_args) {
		switch (res.type) {
		case Type::I32:
			std::cout << "import argument i32 " << res.value.i32 << "\r\n";
			break;
		default:
			break;
		}
	}

	WriteCall(s_stdout_stream.get(), func->module_name, func->field_name,
			vec_args, vec_results, interp::Result::Ok);
	return InterpResult::Ok;
}

ImportDelegate::InterpResult ImportDelegate::RustUnwindCallback(const HostFunc* func, const FuncSignature* sig, Index num_args, TypedValue* args,
			Index num_results, TypedValue* out_results, void* user_data) {
	return InterpResult::Ok;
}

void ImportDelegate::PrintError(const ErrorCallback& callback, const char* format, ...) {
	WABT_SNPRINTF_ALLOCA(buffer, length, format);
	callback(buffer);
}
*/
