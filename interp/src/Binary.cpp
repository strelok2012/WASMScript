/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdarg.h>
#include <stdio.h>
#include <alloca.h>
#include <string.h>
#include <iomanip>
#include "Binary.h"
#include "Module.h"

namespace wasm {

template <typename E>
constexpr typename std::underlying_type<E>::type toInt(const E &e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

template <typename Enum>
inline constexpr int GetEnumCount() {
	return toInt(Enum::Last) - toInt(Enum::First) + 1;
}

constexpr int kBinarySectionCount = GetEnumCount<BinarySection>();
constexpr int kExternalKindCount = GetEnumCount<ExternalKind>();

enum class LinkingEntryType {
	StackPointer = 1,
	SymbolInfo = 2,
	DataSize = 3,
	DataAlignment = 4,
	SegmentInfo = 5,
};

enum class SymbolBinding {
	Global = 0,
	Weak = 1,
	Local = 2,
};

enum class NameSectionSubsection {
	Function = 1,
	Local = 2,
};

#define WABT_FOREACH_BINARY_SECTION(V) \
	V(Custom, custom, 0)               \
	V(Type, type, 1)                   \
	V(Import, import, 2)               \
	V(Function, function, 3)           \
	V(Table, table, 4)                 \
	V(Memory, memory, 5)               \
	V(Global, global, 6)               \
	V(Export, export, 7)               \
	V(Start, start, 8)                 \
	V(Elem, elem, 9)                   \
	V(Code, code, 10)                  \
	V(Data, data, 11)

static const char* g_section_name[] = {
	"Custom",
	"Type",
	"Import",
	"Function",
	"Table",
	"Memory",
	"Global",
	"Export",
	"Start",
	"Elem",
	"Code",
	"Data"
};

static WABT_INLINE const char* GetSectionName(BinarySection sec) {
	assert(static_cast<int>(sec) < kBinarySectionCount);
	return g_section_name[static_cast<size_t>(sec)];
}

constexpr auto WABT_DEFAULT_SNPRINTF_ALLOCA_BUFSIZE = 256;

#define CHECK_RESULT(expr) do { if (expr == ::wasm::Result::Error) { return ::wasm::Result::Error; } } while (0)

#define ERROR_UNLESS(expr, V)  \
	do { if (!(expr)) {          \
		PushErrorStream([&] (StringStream &stream) { stream << V; }); \
		return Result::Error;    \
	} } while (0)

#define ERROR_UNLESS_OPCODE_ENABLED(opcode)           \
	do { if (!opcode.IsEnabled(_options->features)) { \
		return ReportUnexpectedOpcode(opcode);        \
	} } while (0)

#define CALLBACK0(member) ERROR_UNLESS(Succeeded(_delegate->member()), #member " callback failed")
#define CALLBACK(member, ...) ERROR_UNLESS(Succeeded(_delegate->member(__VA_ARGS__)), #member " callback failed")

class SourceReader::BinaryReader {
public:
	BinaryReader(SourceReader * delegate, const ReadOptions* options);

	Result ReadModule();

private:
	// void WABT_PRINTF_FORMAT(2, 3) PrintError(const char* format, ...);

	template <typename Callback>
	void PushErrorStream(const Callback &);

	Result ReadOpcode(Opcode* out_value, const char* desc) WABT_WARN_UNUSED;

	template <typename T>
	Result ReadT(T* out_value, const char* type_name, const char* desc) WABT_WARN_UNUSED;

	Result ReadU8(uint8_t* out_value, const char* desc) WABT_WARN_UNUSED;
	Result ReadU32(uint32_t* out_value, const char* desc) WABT_WARN_UNUSED;
	Result ReadF32(uint32_t* out_value, const char* desc) WABT_WARN_UNUSED;
	Result ReadF64(uint64_t* out_value, const char* desc) WABT_WARN_UNUSED;
	Result ReadU32Leb128(uint32_t* out_value, const char* desc) WABT_WARN_UNUSED;
	Result ReadS32Leb128(uint32_t* out_value, const char* desc) WABT_WARN_UNUSED;
	Result ReadS64Leb128(uint64_t* out_value, const char* desc) WABT_WARN_UNUSED;
	Result ReadType(Type* out_value, const char* desc) WABT_WARN_UNUSED;
	Result ReadStr(StringView* out_str, const char* desc) WABT_WARN_UNUSED;
	Result ReadBytes(const void** out_data, Address* out_data_size, const char* desc) WABT_WARN_UNUSED;
	Result ReadIndex(Index* index, const char* desc) WABT_WARN_UNUSED;
	Result ReadOffset(Offset* offset, const char* desc) WABT_WARN_UNUSED;

	Index NumTotalFuncs();
	Index NumTotalTables();
	Index NumTotalMemories();
	Index NumTotalGlobals();

	Result ReadI32InitExpr(Index index) WABT_WARN_UNUSED;
	Result ReadInitExpr(Index index, bool require_i32 = false) WABT_WARN_UNUSED;
	Result ReadTable(Type* out_elem_type, Limits* out_elem_limits) WABT_WARN_UNUSED;
	Result ReadMemory(Limits* out_page_limits) WABT_WARN_UNUSED;
	Result ReadGlobalHeader(Type* out_type, bool* out_mutable) WABT_WARN_UNUSED;
	Result ReadExceptionType(TypeVector& sig) WABT_WARN_UNUSED;
	Result ReadFunctionBody(Offset end_offset) WABT_WARN_UNUSED;
	Result ReadNamesSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadRelocSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadLinkingSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadCustomSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadTypeSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadImportSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadFunctionSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadTableSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadMemorySection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadGlobalSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadExportSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadStartSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadElemSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadCodeSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadDataSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadExceptionSection(Offset section_size) WABT_WARN_UNUSED;
	Result ReadSections() WABT_WARN_UNUSED;
	Result ReportUnexpectedOpcode(Opcode opcode, const char* message = nullptr);

	size_t _read_end = 0; // Either the section end or data_size.
	ReaderState *_state = nullptr;
	SourceReader* _delegate = nullptr;
	TypeVector _param_types;
	std::vector<Index> _target_depths;
	const ReadOptions* _options = nullptr;
	BinarySection _last_known_section = BinarySection::Invalid;

	Index _num_signatures = 0;
	Index _num_imports = 0;
	Index _num_func_imports = 0;
	Index _num_table_imports = 0;
	Index _num_memory_imports = 0;
	Index _num_global_imports = 0;
	Index _num_exception_imports = 0;
	Index _num_function_signatures = 0;
	Index _num_tables = 0;
	Index _num_memories = 0;
	Index _num_globals = 0;
	Index _num_exports = 0;
	Index _num_function_bodies = 0;
	Index _num_exceptions = 0;
};

SourceReader::BinaryReader::BinaryReader(SourceReader* delegate, const ReadOptions* options)
: _read_end(delegate->_state.size)
, _delegate(delegate)
, _options(options)
, _last_known_section(BinarySection::Invalid) {
	_state = &_delegate->_state;
}

template <typename Callback>
void SourceReader::BinaryReader::PushErrorStream(const Callback &cb) {
	StringStream stream;
	cb(stream);

	_delegate->OnError(stream);
}

Result SourceReader::BinaryReader::ReportUnexpectedOpcode(Opcode opcode,
		const char* message) {
	const char* maybe_space = " ";
	if (!message) {
		message = maybe_space = "";
	}
	PushErrorStream([&] (StringStream &stream) {
		stream << "unexpected opcode" << maybe_space << message << ": ";
		if (opcode.HasPrefix()) {
			stream << std::dec << opcode.GetPrefix() << " " << opcode.GetCode()
				<< std::setw(2) << std::setfill('0') << std::hex << " (0x" << opcode.GetPrefix() << " 0x" << opcode.GetCode() << ")";
		} else {
			stream << std::dec << opcode.GetCode()
				<< std::setw(2) << std::setfill('0') << std::hex << " (0x" << opcode.GetPrefix() << ")";
		}
	});
	return Result::Error;
}

Result SourceReader::BinaryReader::ReadOpcode(Opcode* out_value, const char* desc) {
	uint8_t value = 0;
	CHECK_RESULT(ReadU8(&value, desc));

	if (Opcode::IsPrefixByte(value)) {
		uint32_t code;
		CHECK_RESULT(ReadU32Leb128(&code, desc));
		*out_value = Opcode::FromCode(value, code);
	} else {
		*out_value = Opcode::FromCode(value);
	}
	return Result::Ok;
}

template<typename T>
Result SourceReader::BinaryReader::ReadT(T* out_value, const char* type_name, const char* desc) {
	if (_state->offset + sizeof(T) > _read_end) {
		PushErrorStream([&] (StringStream &stream) {
			stream << "unable to read " << type_name << ": " << desc;
		});
		return Result::Error;
	}
	memcpy(out_value, _state->data + _state->offset, sizeof(T));
	_state->offset += sizeof(T);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadU8(uint8_t* out_value, const char* desc) {
	return ReadT(out_value, "uint8_t", desc);
}

Result SourceReader::BinaryReader::ReadU32(uint32_t* out_value, const char* desc) {
	return ReadT(out_value, "uint32_t", desc);
}

Result SourceReader::BinaryReader::ReadF32(uint32_t* out_value, const char* desc) {
	return ReadT(out_value, "float", desc);
}

Result SourceReader::BinaryReader::ReadF64(uint64_t* out_value, const char* desc) {
	return ReadT(out_value, "double", desc);
}

Result SourceReader::BinaryReader::ReadU32Leb128(uint32_t* out_value, const char* desc) {
	const uint8_t* p = _state->data + _state->offset;
	const uint8_t* end = _state->data + _read_end;
	size_t bytes_read = wasm::ReadU32Leb128(p, end, out_value);
	ERROR_UNLESS(bytes_read > 0, "unable to read u32 leb128: " << desc);
	_state->offset += bytes_read;
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadS32Leb128(uint32_t* out_value, const char* desc) {
	const uint8_t* p = _state->data + _state->offset;
	const uint8_t* end = _state->data + _read_end;
	size_t bytes_read = wasm::ReadS32Leb128(p, end, out_value);
	ERROR_UNLESS(bytes_read > 0, "unable to read i32 leb128: " << desc);
	_state->offset += bytes_read;
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadS64Leb128(uint64_t* out_value, const char* desc) {
	const uint8_t* p = _state->data + _state->offset;
	const uint8_t* end = _state->data + _read_end;
	size_t bytes_read = wasm::ReadS64Leb128(p, end, out_value);
	ERROR_UNLESS(bytes_read > 0, "unable to read i64 leb128: " << desc);
	_state->offset += bytes_read;
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadType(Type* out_value, const char* desc) {
	uint32_t type = 0;
	CHECK_RESULT(ReadS32Leb128(&type, desc));
	// Must be in the vs7 range: [-128, 127).
	ERROR_UNLESS( (static_cast<int32_t>(type) >= -128 && static_cast<int32_t>(type) <= 127), "invalid type: " << type);
	*out_value = static_cast<Type>(type);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadStr(StringView* out_str, const char* desc) {
	uint32_t str_len = 0;
	CHECK_RESULT(ReadU32Leb128(&str_len, "string length"));

	ERROR_UNLESS(_state->offset + str_len <= _read_end, "unable to read string: " << desc);
	*out_str = StringView(reinterpret_cast<const char*>(_state->data) + _state->offset, str_len);
	_state->offset += str_len;
	ERROR_UNLESS(IsValidUtf8(out_str->data(), out_str->size()), "invalid utf-8 encoding: " << desc);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadBytes(const void** out_data, Address* out_data_size, const char* desc) {
	uint32_t data_size = 0;
	CHECK_RESULT(ReadU32Leb128(&data_size, "data size"));

	ERROR_UNLESS(_state->offset + data_size <= _read_end, "unable to read data: " << desc);

	*out_data = static_cast<const uint8_t*>(_state->data) + _state->offset;
	*out_data_size = data_size;
	_state->offset += data_size;
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadIndex(Index* index, const char* desc) {
	uint32_t value;
	CHECK_RESULT(ReadU32Leb128(&value, desc));
	*index = value;
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadOffset(Offset* offset, const char* desc) {
	uint32_t value;
	CHECK_RESULT(ReadU32Leb128(&value, desc));
	*offset = value;
	return Result::Ok;
}

static bool is_valid_external_kind(uint8_t kind) {
	return kind < kExternalKindCount;
}

static bool is_concrete_type(Type type) {
	switch (type) {
	case Type::I32:
	case Type::I64:
	case Type::F32:
	case Type::F64:
		return true;

	default:
		return false;
	}
}

static bool is_inline_sig_type(Type type) {
	return is_concrete_type(type) || type == Type::Void;
}

Index SourceReader::BinaryReader::NumTotalFuncs() {
	return _num_func_imports + _num_function_signatures;
}

Index SourceReader::BinaryReader::NumTotalTables() {
	return _num_table_imports + _num_tables;
}

Index SourceReader::BinaryReader::NumTotalMemories() {
	return _num_memory_imports + _num_memories;
}

Index SourceReader::BinaryReader::NumTotalGlobals() {
	return _num_global_imports + _num_globals;
}

Result SourceReader::BinaryReader::ReadI32InitExpr(Index index) {
	return ReadInitExpr(index, true);
}

Result SourceReader::BinaryReader::ReadInitExpr(Index index, bool require_i32) {
	Opcode opcode;
	CHECK_RESULT(ReadOpcode(&opcode, "opcode"));

	switch (opcode) {
	case Opcode::I32Const: {
		uint32_t value = 0;
		CHECK_RESULT(ReadS32Leb128(&value, "init_expr i32.const value"));
		CALLBACK(OnInitExprI32ConstExpr, index, value);
		break;
	}

	case Opcode::I64Const: {
		uint64_t value = 0;
		CHECK_RESULT(ReadS64Leb128(&value, "init_expr i64.const value"));
		CALLBACK(OnInitExprI64ConstExpr, index, value);
		break;
	}

	case Opcode::F32Const: {
		uint32_t value_bits = 0;
		CHECK_RESULT(ReadF32(&value_bits, "init_expr f32.const value"));
		CALLBACK(OnInitExprF32ConstExpr, index, value_bits);
		break;
	}

	case Opcode::F64Const: {
		uint64_t value_bits = 0;
		CHECK_RESULT(ReadF64(&value_bits, "init_expr f64.const value"));
		CALLBACK(OnInitExprF64ConstExpr, index, value_bits);
		break;
	}

	case Opcode::GetGlobal: {
		Index global_index;
		CHECK_RESULT(ReadIndex(&global_index, "init_expr get_global index"));
		CALLBACK(OnInitExprGetGlobalExpr, index, global_index);
		break;
	}

	case Opcode::End:
		return Result::Ok;

	default:
		return ReportUnexpectedOpcode(opcode, "in initializer expression");
	}

	if (require_i32 && opcode != Opcode::I32Const
			&& opcode != Opcode::GetGlobal) {
		PushErrorStream([&] (StringStream &stream) { stream << "expected i32 init_expr"; });
		return Result::Error;
	}

	CHECK_RESULT(ReadOpcode(&opcode, "opcode"));
	ERROR_UNLESS(opcode == Opcode::End, "expected END opcode after initializer expression");
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadTable(Type* out_elem_type, Limits* out_elem_limits) {
	CHECK_RESULT(ReadType(out_elem_type, "table elem type"));
	ERROR_UNLESS(*out_elem_type == Type::Anyfunc, "table elem type must by anyfunc");

	uint32_t flags;
	uint32_t initial;
	uint32_t max = 0;
	CHECK_RESULT(ReadU32Leb128(&flags, "table flags"));
	CHECK_RESULT(ReadU32Leb128(&initial, "table initial elem count"));
	bool has_max = flags & WABT_BINARY_LIMITS_HAS_MAX_FLAG;
	bool is_shared = flags & WABT_BINARY_LIMITS_IS_SHARED_FLAG;
	ERROR_UNLESS(!is_shared, "tables may not be shared");
	if (has_max) {
		CHECK_RESULT(ReadU32Leb128(&max, "table max elem count"));
		ERROR_UNLESS(initial <= max, "table initial elem count must be <= max elem count");
	}

	out_elem_limits->has_max = has_max;
	out_elem_limits->initial = initial;
	out_elem_limits->max = max;
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadMemory(Limits* out_page_limits) {
	uint32_t flags;
	uint32_t initial;
	uint32_t max = 0;
	CHECK_RESULT(ReadU32Leb128(&flags, "memory flags"));
	CHECK_RESULT(ReadU32Leb128(&initial, "memory initial page count"));
	ERROR_UNLESS(initial <= WABT_MAX_PAGES, "invalid memory initial size");
	bool has_max = flags & WABT_BINARY_LIMITS_HAS_MAX_FLAG;
	bool is_shared = flags & WABT_BINARY_LIMITS_IS_SHARED_FLAG;
	ERROR_UNLESS(!is_shared || has_max, "shared memory must have a max size");
	if (has_max) {
		CHECK_RESULT(ReadU32Leb128(&max, "memory max page count"));
		ERROR_UNLESS(max <= WABT_MAX_PAGES, "invalid memory max size");
		ERROR_UNLESS(initial <= max, "memory initial size must be <= max size");
	}

	out_page_limits->has_max = has_max;
	out_page_limits->is_shared = is_shared;
	out_page_limits->initial = initial;
	out_page_limits->max = max;
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadGlobalHeader(Type* out_type, bool* out_mutable) {
	Type global_type = Type::Void;
	uint8_t mutable_ = 0;
	CHECK_RESULT(ReadType(&global_type, "global type"));
	ERROR_UNLESS(is_concrete_type(global_type), "invalid global type: " << std::hex << static_cast<int>(global_type));

	CHECK_RESULT(ReadU8(&mutable_, "global mutability"));
	ERROR_UNLESS(mutable_ <= 1, "global mutability must be 0 or 1");

	*out_type = global_type;
	*out_mutable = mutable_;
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadFunctionBody(Offset end_offset) {
	bool seen_end_opcode = false;
	while (_state->offset < end_offset) {
		Opcode opcode;
		CHECK_RESULT(ReadOpcode(&opcode, "opcode"));
		CALLBACK(OnOpcode, opcode);
		switch (opcode) {
		case Opcode::Unreachable:
			CALLBACK0(OnUnreachableExpr);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::Block: {
			Type sig_type;
			CHECK_RESULT(ReadType(&sig_type, "block signature type"));
			ERROR_UNLESS(is_inline_sig_type(sig_type),
					"expected valid block signature type");
			Index num_types = sig_type == Type::Void ? 0 : 1;
			CALLBACK(OnBlockExpr, num_types, &sig_type);
			CALLBACK(OnOpcodeBlockSig, num_types, &sig_type);
			break;
		}

		case Opcode::Loop: {
			Type sig_type;
			CHECK_RESULT(ReadType(&sig_type, "loop signature type"));
			ERROR_UNLESS(is_inline_sig_type(sig_type),
					"expected valid block signature type");
			Index num_types = sig_type == Type::Void ? 0 : 1;
			CALLBACK(OnLoopExpr, num_types, &sig_type);
			CALLBACK(OnOpcodeBlockSig, num_types, &sig_type);
			break;
		}

		case Opcode::If: {
			Type sig_type;
			CHECK_RESULT(ReadType(&sig_type, "if signature type"));
			ERROR_UNLESS(is_inline_sig_type(sig_type),
					"expected valid block signature type");
			Index num_types = sig_type == Type::Void ? 0 : 1;
			CALLBACK(OnIfExpr, num_types, &sig_type);
			CALLBACK(OnOpcodeBlockSig, num_types, &sig_type);
			break;
		}

		case Opcode::Else:
			CALLBACK0(OnElseExpr);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::Select:
			CALLBACK0(OnSelectExpr);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::Br: {
			Index depth;
			CHECK_RESULT(ReadIndex(&depth, "br depth"));
			CALLBACK(OnBrExpr, depth);
			CALLBACK(OnOpcodeIndex, depth);
			break;
		}

		case Opcode::BrIf: {
			Index depth;
			CHECK_RESULT(ReadIndex(&depth, "br_if depth"));
			CALLBACK(OnBrIfExpr, depth);
			CALLBACK(OnOpcodeIndex, depth);
			break;
		}

		case Opcode::BrTable: {
			Index num_targets;
			CHECK_RESULT(ReadIndex(&num_targets, "br_table target count"));
			_target_depths.resize(num_targets);

			for (Index i = 0; i < num_targets; ++i) {
				Index target_depth;
				CHECK_RESULT(ReadIndex(&target_depth, "br_table target depth"));
				_target_depths[i] = target_depth;
			}

			Index default_target_depth;
			CHECK_RESULT(
					ReadIndex(&default_target_depth,
							"br_table default target depth"));

			Index* target_depths =
					num_targets ? _target_depths.data() : nullptr;

			CALLBACK(OnBrTableExpr, num_targets, target_depths,
					default_target_depth);
			break;
		}

		case Opcode::Return:
			CALLBACK0(OnReturnExpr);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::Nop:
			CALLBACK0(OnNopExpr);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::Drop:
			CALLBACK0(OnDropExpr);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::End:
			if (_state->offset == end_offset) {
				seen_end_opcode = true;
				CALLBACK0(OnEndFunc);
			} else {
				CALLBACK0(OnEndExpr);
			}
			break;

		case Opcode::I32Const: {
			uint32_t value;
			CHECK_RESULT(ReadS32Leb128(&value, "i32.const value"));
			CALLBACK(OnI32ConstExpr, value);
			CALLBACK(OnOpcodeUint32, value);
			break;
		}

		case Opcode::I64Const: {
			uint64_t value;
			CHECK_RESULT(ReadS64Leb128(&value, "i64.const value"));
			CALLBACK(OnI64ConstExpr, value);
			CALLBACK(OnOpcodeUint64, value);
			break;
		}

		case Opcode::F32Const: {
			uint32_t value_bits = 0;
			CHECK_RESULT(ReadF32(&value_bits, "f32.const value"));
			CALLBACK(OnF32ConstExpr, value_bits);
			CALLBACK(OnOpcodeF32, value_bits);
			break;
		}

		case Opcode::F64Const: {
			uint64_t value_bits = 0;
			CHECK_RESULT(ReadF64(&value_bits, "f64.const value"));
			CALLBACK(OnF64ConstExpr, value_bits);
			CALLBACK(OnOpcodeF64, value_bits);
			break;
		}

		case Opcode::GetGlobal: {
			Index global_index;
			CHECK_RESULT(ReadIndex(&global_index, "get_global global index"));
			CALLBACK(OnGetGlobalExpr, global_index);
			CALLBACK(OnOpcodeIndex, global_index);
			break;
		}

		case Opcode::GetLocal: {
			Index local_index;
			CHECK_RESULT(ReadIndex(&local_index, "get_local local index"));
			CALLBACK(OnGetLocalExpr, local_index);
			CALLBACK(OnOpcodeIndex, local_index);
			break;
		}

		case Opcode::SetGlobal: {
			Index global_index;
			CHECK_RESULT(ReadIndex(&global_index, "set_global global index"));
			CALLBACK(OnSetGlobalExpr, global_index);
			CALLBACK(OnOpcodeIndex, global_index);
			break;
		}

		case Opcode::SetLocal: {
			Index local_index;
			CHECK_RESULT(ReadIndex(&local_index, "set_local local index"));
			CALLBACK(OnSetLocalExpr, local_index);
			CALLBACK(OnOpcodeIndex, local_index);
			break;
		}

		case Opcode::Call: {
			Index func_index;
			CHECK_RESULT(ReadIndex(&func_index, "call function index"));
			ERROR_UNLESS(func_index < NumTotalFuncs(), "invalid call function index: " << func_index);
			CALLBACK(OnCallExpr, func_index);
			CALLBACK(OnOpcodeIndex, func_index);
			break;
		}

		case Opcode::CallIndirect: {
			Index sig_index;
			CHECK_RESULT(ReadIndex(&sig_index, "call_indirect signature index"));
			ERROR_UNLESS(sig_index < _num_signatures, "invalid call_indirect signature index");
			uint32_t reserved;
			CHECK_RESULT(ReadU32Leb128(&reserved, "call_indirect reserved"));
			ERROR_UNLESS(reserved == 0, "call_indirect reserved value must be 0");
			CALLBACK(OnCallIndirectExpr, sig_index);
			CALLBACK(OnOpcodeUint32Uint32, sig_index, reserved);
			break;
		}

		case Opcode::TeeLocal: {
			Index local_index;
			CHECK_RESULT(ReadIndex(&local_index, "tee_local local index"));
			CALLBACK(OnTeeLocalExpr, local_index);
			CALLBACK(OnOpcodeIndex, local_index);
			break;
		}

		case Opcode::I32Load8S:
		case Opcode::I32Load8U:
		case Opcode::I32Load16S:
		case Opcode::I32Load16U:
		case Opcode::I64Load8S:
		case Opcode::I64Load8U:
		case Opcode::I64Load16S:
		case Opcode::I64Load16U:
		case Opcode::I64Load32S:
		case Opcode::I64Load32U:
		case Opcode::I32Load:
		case Opcode::I64Load:
		case Opcode::F32Load:
		case Opcode::F64Load: {
			uint32_t alignment_log2;
			CHECK_RESULT(ReadU32Leb128(&alignment_log2, "load alignment"));
			Address offset;
			CHECK_RESULT(ReadU32Leb128(&offset, "load offset"));

			CALLBACK(OnLoadExpr, opcode, alignment_log2, offset);
			CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
			break;
		}

		case Opcode::I32Store8:
		case Opcode::I32Store16:
		case Opcode::I64Store8:
		case Opcode::I64Store16:
		case Opcode::I64Store32:
		case Opcode::I32Store:
		case Opcode::I64Store:
		case Opcode::F32Store:
		case Opcode::F64Store: {
			uint32_t alignment_log2;
			CHECK_RESULT(ReadU32Leb128(&alignment_log2, "store alignment"));
			Address offset;
			CHECK_RESULT(ReadU32Leb128(&offset, "store offset"));

			CALLBACK(OnStoreExpr, opcode, alignment_log2, offset);
			CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
			break;
		}

		case Opcode::CurrentMemory: {
			uint32_t reserved;
			CHECK_RESULT(ReadU32Leb128(&reserved, "current_memory reserved"));
			ERROR_UNLESS(reserved == 0,
					"current_memory reserved value must be 0");
			CALLBACK0(OnCurrentMemoryExpr);
			CALLBACK(OnOpcodeUint32, reserved);
			break;
		}

		case Opcode::GrowMemory: {
			uint32_t reserved;
			CHECK_RESULT(ReadU32Leb128(&reserved, "grow_memory reserved"));
			ERROR_UNLESS(reserved == 0, "grow_memory reserved value must be 0");
			CALLBACK0(OnGrowMemoryExpr);
			CALLBACK(OnOpcodeUint32, reserved);
			break;
		}

		case Opcode::I32Add:
		case Opcode::I32Sub:
		case Opcode::I32Mul:
		case Opcode::I32DivS:
		case Opcode::I32DivU:
		case Opcode::I32RemS:
		case Opcode::I32RemU:
		case Opcode::I32And:
		case Opcode::I32Or:
		case Opcode::I32Xor:
		case Opcode::I32Shl:
		case Opcode::I32ShrU:
		case Opcode::I32ShrS:
		case Opcode::I32Rotr:
		case Opcode::I32Rotl:
		case Opcode::I64Add:
		case Opcode::I64Sub:
		case Opcode::I64Mul:
		case Opcode::I64DivS:
		case Opcode::I64DivU:
		case Opcode::I64RemS:
		case Opcode::I64RemU:
		case Opcode::I64And:
		case Opcode::I64Or:
		case Opcode::I64Xor:
		case Opcode::I64Shl:
		case Opcode::I64ShrU:
		case Opcode::I64ShrS:
		case Opcode::I64Rotr:
		case Opcode::I64Rotl:
		case Opcode::F32Add:
		case Opcode::F32Sub:
		case Opcode::F32Mul:
		case Opcode::F32Div:
		case Opcode::F32Min:
		case Opcode::F32Max:
		case Opcode::F32Copysign:
		case Opcode::F64Add:
		case Opcode::F64Sub:
		case Opcode::F64Mul:
		case Opcode::F64Div:
		case Opcode::F64Min:
		case Opcode::F64Max:
		case Opcode::F64Copysign:
			CALLBACK(OnBinaryExpr, opcode);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::I32Eq:
		case Opcode::I32Ne:
		case Opcode::I32LtS:
		case Opcode::I32LeS:
		case Opcode::I32LtU:
		case Opcode::I32LeU:
		case Opcode::I32GtS:
		case Opcode::I32GeS:
		case Opcode::I32GtU:
		case Opcode::I32GeU:
		case Opcode::I64Eq:
		case Opcode::I64Ne:
		case Opcode::I64LtS:
		case Opcode::I64LeS:
		case Opcode::I64LtU:
		case Opcode::I64LeU:
		case Opcode::I64GtS:
		case Opcode::I64GeS:
		case Opcode::I64GtU:
		case Opcode::I64GeU:
		case Opcode::F32Eq:
		case Opcode::F32Ne:
		case Opcode::F32Lt:
		case Opcode::F32Le:
		case Opcode::F32Gt:
		case Opcode::F32Ge:
		case Opcode::F64Eq:
		case Opcode::F64Ne:
		case Opcode::F64Lt:
		case Opcode::F64Le:
		case Opcode::F64Gt:
		case Opcode::F64Ge:
			CALLBACK(OnCompareExpr, opcode);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::I32Clz:
		case Opcode::I32Ctz:
		case Opcode::I32Popcnt:
		case Opcode::I64Clz:
		case Opcode::I64Ctz:
		case Opcode::I64Popcnt:
		case Opcode::F32Abs:
		case Opcode::F32Neg:
		case Opcode::F32Ceil:
		case Opcode::F32Floor:
		case Opcode::F32Trunc:
		case Opcode::F32Nearest:
		case Opcode::F32Sqrt:
		case Opcode::F64Abs:
		case Opcode::F64Neg:
		case Opcode::F64Ceil:
		case Opcode::F64Floor:
		case Opcode::F64Trunc:
		case Opcode::F64Nearest:
		case Opcode::F64Sqrt:
			CALLBACK(OnUnaryExpr, opcode);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::I32TruncSF32:
		case Opcode::I32TruncSF64:
		case Opcode::I32TruncUF32:
		case Opcode::I32TruncUF64:
		case Opcode::I32WrapI64:
		case Opcode::I64TruncSF32:
		case Opcode::I64TruncSF64:
		case Opcode::I64TruncUF32:
		case Opcode::I64TruncUF64:
		case Opcode::I64ExtendSI32:
		case Opcode::I64ExtendUI32:
		case Opcode::F32ConvertSI32:
		case Opcode::F32ConvertUI32:
		case Opcode::F32ConvertSI64:
		case Opcode::F32ConvertUI64:
		case Opcode::F32DemoteF64:
		case Opcode::F32ReinterpretI32:
		case Opcode::F64ConvertSI32:
		case Opcode::F64ConvertUI32:
		case Opcode::F64ConvertSI64:
		case Opcode::F64ConvertUI64:
		case Opcode::F64PromoteF32:
		case Opcode::F64ReinterpretI64:
		case Opcode::I32ReinterpretF32:
		case Opcode::I64ReinterpretF64:
		case Opcode::I32Eqz:
		case Opcode::I64Eqz:
			CALLBACK(OnConvertExpr, opcode);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::Try: {
			ERROR_UNLESS_OPCODE_ENABLED(opcode);
			Type sig_type;
			CHECK_RESULT(ReadType(&sig_type, "try signature type"));
			ERROR_UNLESS(is_inline_sig_type(sig_type),
					"expected valid block signature type");
			Index num_types = sig_type == Type::Void ? 0 : 1;
			CALLBACK(OnTryExpr, num_types, &sig_type);
			CALLBACK(OnOpcodeBlockSig, num_types, &sig_type);
			break;
		}

		case Opcode::Catch: {
			ERROR_UNLESS_OPCODE_ENABLED(opcode);
			Index index;
			CHECK_RESULT(ReadIndex(&index, "exception index"));
			CALLBACK(OnCatchExpr, index);
			CALLBACK(OnOpcodeIndex, index);
			break;
		}

		case Opcode::CatchAll: {
			ERROR_UNLESS_OPCODE_ENABLED(opcode);
			CALLBACK(OnCatchAllExpr);
			CALLBACK0(OnOpcodeBare);
			break;
		}

		case Opcode::Rethrow: {
			ERROR_UNLESS_OPCODE_ENABLED(opcode);
			Index depth;
			CHECK_RESULT(ReadIndex(&depth, "catch depth"));
			CALLBACK(OnRethrowExpr, depth);
			CALLBACK(OnOpcodeIndex, depth);
			break;
		}

		case Opcode::Throw: {
			ERROR_UNLESS_OPCODE_ENABLED(opcode);
			Index index;
			CHECK_RESULT(ReadIndex(&index, "exception index"));
			CALLBACK(OnThrowExpr, index);
			CALLBACK(OnOpcodeIndex, index);
			break;
		}

		case Opcode::I32Extend8S:
		case Opcode::I32Extend16S:
		case Opcode::I64Extend8S:
		case Opcode::I64Extend16S:
		case Opcode::I64Extend32S:
			ERROR_UNLESS_OPCODE_ENABLED(opcode);
			CALLBACK(OnUnaryExpr, opcode);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::I32TruncSSatF32:
		case Opcode::I32TruncUSatF32:
		case Opcode::I32TruncSSatF64:
		case Opcode::I32TruncUSatF64:
		case Opcode::I64TruncSSatF32:
		case Opcode::I64TruncUSatF32:
		case Opcode::I64TruncSSatF64:
		case Opcode::I64TruncUSatF64:
			ERROR_UNLESS_OPCODE_ENABLED(opcode);
			CALLBACK(OnConvertExpr, opcode);
			CALLBACK0(OnOpcodeBare);
			break;

		case Opcode::AtomicWake: {
			uint32_t alignment_log2;
			CHECK_RESULT(ReadU32Leb128(&alignment_log2, "load alignment"));
			Address offset;
			CHECK_RESULT(ReadU32Leb128(&offset, "load offset"));

			CALLBACK(OnAtomicWakeExpr, opcode, alignment_log2, offset);
			CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
			break;
		}

		case Opcode::I32AtomicWait:
		case Opcode::I64AtomicWait: {
			uint32_t alignment_log2;
			CHECK_RESULT(ReadU32Leb128(&alignment_log2, "load alignment"));
			Address offset;
			CHECK_RESULT(ReadU32Leb128(&offset, "load offset"));

			CALLBACK(OnAtomicWaitExpr, opcode, alignment_log2, offset);
			CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
			break;
		}

		case Opcode::I32AtomicLoad8U:
		case Opcode::I32AtomicLoad16U:
		case Opcode::I64AtomicLoad8U:
		case Opcode::I64AtomicLoad16U:
		case Opcode::I64AtomicLoad32U:
		case Opcode::I32AtomicLoad:
		case Opcode::I64AtomicLoad: {
			uint32_t alignment_log2;
			CHECK_RESULT(ReadU32Leb128(&alignment_log2, "load alignment"));
			Address offset;
			CHECK_RESULT(ReadU32Leb128(&offset, "load offset"));

			CALLBACK(OnAtomicLoadExpr, opcode, alignment_log2, offset);
			CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
			break;
		}

		case Opcode::I32AtomicStore8:
		case Opcode::I32AtomicStore16:
		case Opcode::I64AtomicStore8:
		case Opcode::I64AtomicStore16:
		case Opcode::I64AtomicStore32:
		case Opcode::I32AtomicStore:
		case Opcode::I64AtomicStore: {
			uint32_t alignment_log2;
			CHECK_RESULT(ReadU32Leb128(&alignment_log2, "store alignment"));
			Address offset;
			CHECK_RESULT(ReadU32Leb128(&offset, "store offset"));

			CALLBACK(OnAtomicStoreExpr, opcode, alignment_log2, offset);
			CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
			break;
		}

		case Opcode::I32AtomicRmwAdd:
		case Opcode::I64AtomicRmwAdd:
		case Opcode::I32AtomicRmw8UAdd:
		case Opcode::I32AtomicRmw16UAdd:
		case Opcode::I64AtomicRmw8UAdd:
		case Opcode::I64AtomicRmw16UAdd:
		case Opcode::I64AtomicRmw32UAdd:
		case Opcode::I32AtomicRmwSub:
		case Opcode::I64AtomicRmwSub:
		case Opcode::I32AtomicRmw8USub:
		case Opcode::I32AtomicRmw16USub:
		case Opcode::I64AtomicRmw8USub:
		case Opcode::I64AtomicRmw16USub:
		case Opcode::I64AtomicRmw32USub:
		case Opcode::I32AtomicRmwAnd:
		case Opcode::I64AtomicRmwAnd:
		case Opcode::I32AtomicRmw8UAnd:
		case Opcode::I32AtomicRmw16UAnd:
		case Opcode::I64AtomicRmw8UAnd:
		case Opcode::I64AtomicRmw16UAnd:
		case Opcode::I64AtomicRmw32UAnd:
		case Opcode::I32AtomicRmwOr:
		case Opcode::I64AtomicRmwOr:
		case Opcode::I32AtomicRmw8UOr:
		case Opcode::I32AtomicRmw16UOr:
		case Opcode::I64AtomicRmw8UOr:
		case Opcode::I64AtomicRmw16UOr:
		case Opcode::I64AtomicRmw32UOr:
		case Opcode::I32AtomicRmwXor:
		case Opcode::I64AtomicRmwXor:
		case Opcode::I32AtomicRmw8UXor:
		case Opcode::I32AtomicRmw16UXor:
		case Opcode::I64AtomicRmw8UXor:
		case Opcode::I64AtomicRmw16UXor:
		case Opcode::I64AtomicRmw32UXor:
		case Opcode::I32AtomicRmwXchg:
		case Opcode::I64AtomicRmwXchg:
		case Opcode::I32AtomicRmw8UXchg:
		case Opcode::I32AtomicRmw16UXchg:
		case Opcode::I64AtomicRmw8UXchg:
		case Opcode::I64AtomicRmw16UXchg:
		case Opcode::I64AtomicRmw32UXchg: {
			uint32_t alignment_log2;
			CHECK_RESULT(ReadU32Leb128(&alignment_log2, "memory alignment"));
			Address offset;
			CHECK_RESULT(ReadU32Leb128(&offset, "memory offset"));

			CALLBACK(OnAtomicRmwExpr, opcode, alignment_log2, offset);
			CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
			break;
		}

		case Opcode::I32AtomicRmwCmpxchg:
		case Opcode::I64AtomicRmwCmpxchg:
		case Opcode::I32AtomicRmw8UCmpxchg:
		case Opcode::I32AtomicRmw16UCmpxchg:
		case Opcode::I64AtomicRmw8UCmpxchg:
		case Opcode::I64AtomicRmw16UCmpxchg:
		case Opcode::I64AtomicRmw32UCmpxchg: {
			uint32_t alignment_log2;
			CHECK_RESULT(ReadU32Leb128(&alignment_log2, "memory alignment"));
			Address offset;
			CHECK_RESULT(ReadU32Leb128(&offset, "memory offset"));

			CALLBACK(OnAtomicRmwCmpxchgExpr, opcode, alignment_log2, offset);
			CALLBACK(OnOpcodeUint32Uint32, alignment_log2, offset);
			break;
		}

		default:
			return ReportUnexpectedOpcode(opcode);
		}
	}
	ERROR_UNLESS(_state->offset == end_offset, "function body longer than given size");
	ERROR_UNLESS(seen_end_opcode, "function body must end with END opcode");
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadNamesSection(Offset section_size) {
	CALLBACK(BeginNamesSection, section_size);
	Index i = 0;
	Offset previous_read_end = _read_end;
	uint32_t previous_subsection_type = 0;
	while (_state->offset < _read_end) {
		uint32_t name_type;
		Offset subsection_size;
		CHECK_RESULT(ReadU32Leb128(&name_type, "name type"));
		if (i != 0) {
			ERROR_UNLESS(name_type != previous_subsection_type, "duplicate sub-section");
			ERROR_UNLESS(name_type >= previous_subsection_type, "out-of-order sub-section");
		}
		previous_subsection_type = name_type;
		CHECK_RESULT(ReadOffset(&subsection_size, "subsection size"));
		size_t subsection_end = _state->offset + subsection_size;
		ERROR_UNLESS(subsection_end <= _read_end, "invalid sub-section size: extends past end");
		_read_end = subsection_end;

		switch (static_cast<NameSectionSubsection>(name_type)) {
		case NameSectionSubsection::Function:
			CALLBACK(OnFunctionNameSubsection, i, name_type, subsection_size);
			if (subsection_size) {
				Index num_names;
				CHECK_RESULT(ReadIndex(&num_names, "name count"));
				CALLBACK(OnFunctionNamesCount, num_names);
				Index last_function_index = kInvalidIndex;

				for (Index j = 0; j < num_names; ++j) {
					Index function_index;
					StringView function_name;

					CHECK_RESULT(ReadIndex(&function_index, "function index"));
					ERROR_UNLESS(function_index != last_function_index, "duplicate function name: " << function_index);
					ERROR_UNLESS(last_function_index == kInvalidIndex || function_index > last_function_index,
							"function index out of order: " << function_index);
					last_function_index = function_index;
					ERROR_UNLESS(function_index < NumTotalFuncs(), "invalid function index: " << function_index);
					CHECK_RESULT(ReadStr(&function_name, "function name"));
					CALLBACK(OnFunctionName, function_index, function_name);
				}
			}
			break;
		case NameSectionSubsection::Local:
			CALLBACK(OnLocalNameSubsection, i, name_type, subsection_size);
			if (subsection_size) {
				Index num_funcs;
				CHECK_RESULT(ReadIndex(&num_funcs, "function count"));
				CALLBACK(OnLocalNameFunctionCount, num_funcs);
				Index last_function_index = kInvalidIndex;
				for (Index j = 0; j < num_funcs; ++j) {
					Index function_index;
					CHECK_RESULT(ReadIndex(&function_index, "function index"));
					ERROR_UNLESS(function_index < NumTotalFuncs(), "invalid function index: " << function_index);
					ERROR_UNLESS(last_function_index == kInvalidIndex || function_index > last_function_index,
							"locals function index out of order: " << function_index);
					last_function_index = function_index;
					Index num_locals;
					CHECK_RESULT(ReadIndex(&num_locals, "local count"));
					CALLBACK(OnLocalNameLocalCount, function_index, num_locals);
					Index last_local_index = kInvalidIndex;
					for (Index k = 0; k < num_locals; ++k) {
						Index local_index;
						StringView local_name;

						CHECK_RESULT(ReadIndex(&local_index, "named index"));
						ERROR_UNLESS(local_index != last_local_index, "duplicate local index: " << local_index);
						ERROR_UNLESS(last_local_index == kInvalidIndex || local_index > last_local_index,
								"local index out of order: " << local_index);
						last_local_index = local_index;
						CHECK_RESULT(ReadStr(&local_name, "name"));
						CALLBACK(OnLocalName, function_index, local_index, local_name);
					}
				}
			}
			break;
		default:
			// Unknown subsection, skip it.
			_state->offset = subsection_end;
			break;
		}
		++i;
		ERROR_UNLESS(_state->offset == subsection_end, "unfinished sub-section (expected end: 0x" << std::hex << subsection_end << ")");
		_read_end = previous_read_end;
	}
	CALLBACK0(EndNamesSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadRelocSection(Offset section_size) {
	CALLBACK(BeginRelocSection, section_size);
	uint32_t section;
	CHECK_RESULT(ReadU32Leb128(&section, "section"));
	StringView section_name;
	if (static_cast<BinarySection>(section) == BinarySection::Custom) {
		CHECK_RESULT(ReadStr(&section_name, "section name"));
	}
	Index num_relocs;
	CHECK_RESULT(ReadIndex(&num_relocs, "relocation count"));
	CALLBACK(OnRelocCount, num_relocs, static_cast<BinarySection>(section), section_name);
	for (Index i = 0; i < num_relocs; ++i) {
		Offset offset;
		Index index;
		uint32_t reloc_type, addend = 0;
		CHECK_RESULT(ReadU32Leb128(&reloc_type, "relocation type"));
		CHECK_RESULT(ReadOffset(&offset, "offset"));
		CHECK_RESULT(ReadIndex(&index, "index"));
		RelocType type = static_cast<RelocType>(reloc_type);
		switch (type) {
		case RelocType::MemoryAddressLEB:
		case RelocType::MemoryAddressSLEB:
		case RelocType::MemoryAddressI32:
			CHECK_RESULT(ReadS32Leb128(&addend, "addend"));
			break;
		default:
			break;
		}
		CALLBACK(OnReloc, type, offset, index, addend);
	}
	CALLBACK0(EndRelocSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadLinkingSection(Offset section_size) {
	CALLBACK(BeginLinkingSection, section_size);
	Offset previous_read_end = _read_end;
	while (_state->offset < _read_end) {
		uint32_t linking_type;
		Offset subsection_size;
		CHECK_RESULT(ReadU32Leb128(&linking_type, "type"));
		CHECK_RESULT(ReadOffset(&subsection_size, "subsection size"));
		size_t subsection_end = _state->offset + subsection_size;
		ERROR_UNLESS(subsection_end <= _read_end,
				"invalid sub-section size: extends past end");
		_read_end = subsection_end;

		switch (static_cast<LinkingEntryType>(linking_type)) {
		case LinkingEntryType::StackPointer: {
			uint32_t stack_ptr;
			CHECK_RESULT(ReadU32Leb128(&stack_ptr, "stack pointer index"));
			CALLBACK(OnStackGlobal, stack_ptr);
			break;
		}
		case LinkingEntryType::SymbolInfo: {
			uint32_t info_count;
			CHECK_RESULT(ReadU32Leb128(&info_count, "info count"));
			CALLBACK(OnSymbolInfoCount, info_count);
			while (info_count--) {
				StringView name;
				uint32_t info;
				CHECK_RESULT(ReadStr(&name, "symbol name"));
				CHECK_RESULT(ReadU32Leb128(&info, "sym flags"));
				CALLBACK(OnSymbolInfo, name, info);
			}
			break;
		}
		case LinkingEntryType::DataSize: {
			uint32_t data_size;
			CHECK_RESULT(ReadU32Leb128(&data_size, "data size"));
			CALLBACK(OnDataSize, data_size);
			break;
		}
		case LinkingEntryType::DataAlignment: {
			uint32_t data_alignment;
			CHECK_RESULT(ReadU32Leb128(&data_alignment, "data alignment"));
			CALLBACK(OnDataAlignment, data_alignment);
			break;
		}
		case LinkingEntryType::SegmentInfo: {
			uint32_t info_count;
			CHECK_RESULT(ReadU32Leb128(&info_count, "info count"));
			CALLBACK(OnSegmentInfoCount, info_count);
			for (Index i = 0; i < info_count; i++) {
				StringView name;
				uint32_t alignment;
				uint32_t flags;
				CHECK_RESULT(ReadStr(&name, "segment name"));
				CHECK_RESULT(ReadU32Leb128(&alignment, "segment alignment"));
				CHECK_RESULT(ReadU32Leb128(&flags, "segment flags"));
				CALLBACK(OnSegmentInfo, i, name, alignment, flags);
			}
			break;
		}
		default:
			// Unknown subsection, skip it.
			_state->offset = subsection_end;
			break;
		}
		ERROR_UNLESS(_state->offset == subsection_end, "unfinished sub-section (expected end: 0x" << std::hex << subsection_end << ")");
		_read_end = previous_read_end;
	}
	CALLBACK0(EndLinkingSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadExceptionType(TypeVector& sig) {
	Index num_values;
	CHECK_RESULT(ReadIndex(&num_values, "exception type count"));
	sig.resize(num_values);
	for (Index j = 0; j < num_values; ++j) {
		Type value_type;
		CHECK_RESULT(ReadType(&value_type, "exception value type"));
		ERROR_UNLESS(is_concrete_type(value_type), "excepted valid exception value type (got " << static_cast<int>(value_type) << ")");
		sig[j] = value_type;
	}
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadExceptionSection(Offset section_size) {
	CALLBACK(BeginExceptionSection, section_size);
	CHECK_RESULT(ReadIndex(&_num_exceptions, "exception count"));
	CALLBACK(OnExceptionCount, _num_exceptions);

	for (Index i = 0; i < _num_exceptions; ++i) {
		TypeVector sig;
		CHECK_RESULT(ReadExceptionType(sig));
		CALLBACK(OnExceptionType, i, sig);
	}

	CALLBACK(EndExceptionSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadCustomSection(Offset section_size) {
	StringView section_name;
	CHECK_RESULT(ReadStr(&section_name, "section name"));
	CALLBACK(BeginCustomSection, section_size, section_name);

	bool name_section_ok = _last_known_section >= BinarySection::Import;
	if (_options->read_debug_names && name_section_ok
			&& section_name == WABT_BINARY_SECTION_NAME) {
		CHECK_RESULT(ReadNamesSection(section_size));
	} else if (section_name.rfind(WABT_BINARY_SECTION_RELOC, 0) == 0) {
		// Reloc sections always begin with "reloc."
		CHECK_RESULT(ReadRelocSection(section_size));
	} else if (section_name == WABT_BINARY_SECTION_LINKING) {
		CHECK_RESULT(ReadLinkingSection(section_size));
	} else if (_options->features.isExceptionsEnabled()
			&& section_name == WABT_BINARY_SECTION_EXCEPTION) {
		CHECK_RESULT(ReadExceptionSection(section_size));
	} else {
		// This is an unknown custom section, skip it.
		_state->offset = _read_end;
	}
	CALLBACK0(EndCustomSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadTypeSection(Offset section_size) {
	CALLBACK(BeginTypeSection, section_size);
	CHECK_RESULT(ReadIndex(&_num_signatures, "type count"));
	CALLBACK(OnTypeCount, _num_signatures);

	for (Index i = 0; i < _num_signatures; ++i) {
		Type form;
		CHECK_RESULT(ReadType(&form, "type form"));
		ERROR_UNLESS(form == Type::Func, "unexpected type form: " << static_cast<int>(form));

		Index num_params;
		CHECK_RESULT(ReadIndex(&num_params, "function param count"));

		_param_types.resize(num_params);

		for (Index j = 0; j < num_params; ++j) {
			Type param_type;
			CHECK_RESULT(ReadType(&param_type, "function param type"));
			ERROR_UNLESS(is_concrete_type(param_type), "expected valid param type (got " << static_cast<int>(param_type) << ")");
			_param_types[j] = param_type;
		}

		Index num_results;
		CHECK_RESULT(ReadIndex(&num_results, "function result count"));
		ERROR_UNLESS(num_results <= 1, "result count must be 0 or 1");

		Type result_type = Type::Void;
		if (num_results) {
			CHECK_RESULT(ReadType(&result_type, "function result type"));
			ERROR_UNLESS(is_concrete_type(result_type), "expected valid result type: " << static_cast<int>(result_type));
		}

		Type* param_types = num_params ? _param_types.data() : nullptr;

		CALLBACK(OnType, i, num_params, param_types, num_results, &result_type);
	}
	CALLBACK0(EndTypeSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadImportSection(Offset section_size) {
	CALLBACK(BeginImportSection, section_size);
	CHECK_RESULT(ReadIndex(&_num_imports, "import count"));
	CALLBACK(OnImportCount, _num_imports);
	for (Index i = 0; i < _num_imports; ++i) {
		StringView module_name;
		CHECK_RESULT(ReadStr(&module_name, "import module name"));
		StringView field_name;
		CHECK_RESULT(ReadStr(&field_name, "import field name"));

		uint32_t kind;
		CHECK_RESULT(ReadU32Leb128(&kind, "import kind"));
		switch (static_cast<ExternalKind>(kind)) {
		case ExternalKind::Func: {
			Index sig_index;
			CHECK_RESULT(ReadIndex(&sig_index, "import signature index"));
			ERROR_UNLESS(sig_index < _num_signatures, "invalid import signature index");
			CALLBACK(OnImport, i, module_name, field_name);
			CALLBACK(OnImportFunc, i, module_name, field_name, _num_func_imports, sig_index);
			_num_func_imports++;
			break;
		}

		case ExternalKind::Table: {
			Type elem_type;
			Limits elem_limits;
			CHECK_RESULT(ReadTable(&elem_type, &elem_limits));
			CALLBACK(OnImport, i, module_name, field_name);
			CALLBACK(OnImportTable, i, module_name, field_name, _num_table_imports, elem_type, &elem_limits);
			_num_table_imports++;
			break;
		}

		case ExternalKind::Memory: {
			Limits page_limits;
			CHECK_RESULT(ReadMemory(&page_limits));
			CALLBACK(OnImport, i, module_name, field_name);
			CALLBACK(OnImportMemory, i, module_name, field_name, _num_memory_imports, &page_limits);
			_num_memory_imports++;
			break;
		}

		case ExternalKind::Global: {
			Type type;
			bool mutable_;
			CHECK_RESULT(ReadGlobalHeader(&type, &mutable_));
			CALLBACK(OnImport, i, module_name, field_name);
			CALLBACK(OnImportGlobal, i, module_name, field_name, _num_global_imports, type, mutable_);
			_num_global_imports++;
			break;
		}

		case ExternalKind::Except: {
			ERROR_UNLESS(_options->features.isExceptionsEnabled(),
					"invalid import exception kind: exceptions not allowed");
			TypeVector sig;
			CHECK_RESULT(ReadExceptionType(sig));
			CALLBACK(OnImport, i, module_name, field_name);
			CALLBACK(OnImportException, i, module_name, field_name, _num_exception_imports, sig);
			_num_exception_imports++;
			break;
		}
		}
	}
	CALLBACK0(EndImportSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadFunctionSection(Offset section_size) {
	CALLBACK(BeginFunctionSection, section_size);
	CHECK_RESULT(ReadIndex(&_num_function_signatures, "function signature count"));
	CALLBACK(OnFunctionCount, _num_function_signatures);
	for (Index i = 0; i < _num_function_signatures; ++i) {
		Index func_index = _num_func_imports + i;
		Index sig_index;
		CHECK_RESULT(ReadIndex(&sig_index, "function signature index"));
		ERROR_UNLESS(sig_index < _num_signatures, "invalid function signature index: " << sig_index);
		CALLBACK(OnFunction, func_index, sig_index);
	}
	CALLBACK0(EndFunctionSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadTableSection(Offset section_size) {
	CALLBACK(BeginTableSection, section_size);
	CHECK_RESULT(ReadIndex(&_num_tables, "table count"));
	ERROR_UNLESS(_num_tables <= 1, "table count (" << _num_tables << ") must be 0 or 1");
	CALLBACK(OnTableCount, _num_tables);
	for (Index i = 0; i < _num_tables; ++i) {
		Index table_index = _num_table_imports + i;
		Type elem_type;
		Limits elem_limits;
		CHECK_RESULT(ReadTable(&elem_type, &elem_limits));
		CALLBACK(OnTable, table_index, elem_type, &elem_limits);
	}
	CALLBACK0(EndTableSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadMemorySection(Offset section_size) {
	CALLBACK(BeginMemorySection, section_size);
	CHECK_RESULT(ReadIndex(&_num_memories, "memory count"));
	ERROR_UNLESS(_num_memories <= 1, "memory count must be 0 or 1");
	CALLBACK(OnMemoryCount, _num_memories);
	for (Index i = 0; i < _num_memories; ++i) {
		Index memory_index = _num_memory_imports + i;
		Limits page_limits;
		CHECK_RESULT(ReadMemory(&page_limits));
		CALLBACK(OnMemory, memory_index, &page_limits);
	}
	CALLBACK0(EndMemorySection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadGlobalSection(Offset section_size) {
	CALLBACK(BeginGlobalSection, section_size);
	CHECK_RESULT(ReadIndex(&_num_globals, "global count"));
	CALLBACK(OnGlobalCount, _num_globals);
	for (Index i = 0; i < _num_globals; ++i) {
		Index global_index = i;
		Type global_type;
		bool mutable_;
		CHECK_RESULT(ReadGlobalHeader(&global_type, &mutable_));
		CALLBACK(BeginGlobal, global_index, global_type, mutable_);
		CALLBACK(BeginGlobalInitExpr, global_index);
		CHECK_RESULT(ReadInitExpr(global_index));
		CALLBACK(EndGlobalInitExpr, global_index);
		CALLBACK(EndGlobal, global_index);
	}
	CALLBACK0(EndGlobalSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadExportSection(Offset section_size) {
	CALLBACK(BeginExportSection, section_size);
	CHECK_RESULT(ReadIndex(&_num_exports, "export count"));
	CALLBACK(OnExportCount, _num_exports);
	for (Index i = 0; i < _num_exports; ++i) {
		StringView name;
		CHECK_RESULT(ReadStr(&name, "export item name"));

		uint8_t external_kind = 0;
		CHECK_RESULT(ReadU8(&external_kind, "export external kind"));
		ERROR_UNLESS(is_valid_external_kind(external_kind), "invalid export external kind: " << external_kind);

		Index item_index;
		CHECK_RESULT(ReadIndex(&item_index, "export item index"));
		switch (static_cast<ExternalKind>(external_kind)) {
		case ExternalKind::Func:
			ERROR_UNLESS(item_index < NumTotalFuncs(), "invalid export func index: " << item_index);
			break;
		case ExternalKind::Table:
			ERROR_UNLESS(item_index < NumTotalTables(), "invalid export table index: " << item_index);
			break;
		case ExternalKind::Memory:
			ERROR_UNLESS(item_index < NumTotalMemories(), "invalid export memory index: " << item_index);
			break;
		case ExternalKind::Global:
			ERROR_UNLESS(item_index < NumTotalGlobals(), "invalid export global index: " << item_index);
			break;
		case ExternalKind::Except:
			// Note: Can't check if index valid, exceptions section comes later.
			ERROR_UNLESS(_options->features.isExceptionsEnabled(), "invalid export exception kind: exceptions not allowed");
			break;
		}

		CALLBACK(OnExport, i, static_cast<ExternalKind>(external_kind), item_index, name);
	}
	CALLBACK0(EndExportSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadStartSection(Offset section_size) {
	CALLBACK(BeginStartSection, section_size);
	Index func_index;
	CHECK_RESULT(ReadIndex(&func_index, "start function index"));
	ERROR_UNLESS(func_index < NumTotalFuncs(), "invalid start function index: " << func_index);
	CALLBACK(OnStartFunction, func_index);
	CALLBACK0(EndStartSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadElemSection(Offset section_size) {
	CALLBACK(BeginElemSection, section_size);
	Index num_elem_segments;
	CHECK_RESULT(ReadIndex(&num_elem_segments, "elem segment count"));
	CALLBACK(OnElemSegmentCount, num_elem_segments);
	ERROR_UNLESS(num_elem_segments == 0 || NumTotalTables() > 0, "elem section without table section");
	for (Index i = 0; i < num_elem_segments; ++i) {
		Index table_index;
		CHECK_RESULT(ReadIndex(&table_index, "elem segment table index"));
		CALLBACK(BeginElemSegment, i, table_index);
		CALLBACK(BeginElemSegmentInitExpr, i);
		CHECK_RESULT(ReadI32InitExpr(i));
		CALLBACK(EndElemSegmentInitExpr, i);

		Index num_function_indexes;
		CHECK_RESULT( ReadIndex(&num_function_indexes, "elem segment function index count"));
		CALLBACK(OnElemSegmentFunctionIndexCount, i, num_function_indexes);
		for (Index j = 0; j < num_function_indexes; ++j) {
			Index func_index;
			CHECK_RESULT(ReadIndex(&func_index, "elem segment function index"));
			CALLBACK(OnElemSegmentFunctionIndex, i, func_index);
		}
		CALLBACK(EndElemSegment, i);
	}
	CALLBACK0(EndElemSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadCodeSection(Offset section_size) {
	CALLBACK(BeginCodeSection, section_size);
	CHECK_RESULT(ReadIndex(&_num_function_bodies, "function body count"));
	ERROR_UNLESS(_num_function_signatures == _num_function_bodies, "function signature count != function body count");
	CALLBACK(OnFunctionBodyCount, _num_function_bodies);
	for (Index i = 0; i < _num_function_bodies; ++i) {
		Index func_index = i;
		Offset func_offset = _state->offset;
		_state->offset = func_offset;
		CALLBACK(BeginFunctionBody, func_index);
		uint32_t body_size;
		CHECK_RESULT(ReadU32Leb128(&body_size, "function body size"));
		Offset body_start_offset = _state->offset;
		Offset end_offset = body_start_offset + body_size;

		Index num_local_decls;
		CHECK_RESULT(ReadIndex(&num_local_decls, "local declaration count"));
		CALLBACK(OnLocalDeclCount, num_local_decls);
		for (Index k = 0; k < num_local_decls; ++k) {
			Index num_local_types;
			CHECK_RESULT(ReadIndex(&num_local_types, "local type count"));
			Type local_type;
			CHECK_RESULT(ReadType(&local_type, "local type"));
			ERROR_UNLESS(is_concrete_type(local_type), "expected valid local type");
			CALLBACK(OnLocalDecl, k, num_local_types, local_type);
		}

		CHECK_RESULT(ReadFunctionBody(end_offset));
		CALLBACK(EndFunctionBody, func_index);
	}
	CALLBACK0(EndCodeSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadDataSection(Offset section_size) {
	CALLBACK(BeginDataSection, section_size);
	Index num_data_segments;
	CHECK_RESULT(ReadIndex(&num_data_segments, "data segment count"));
	CALLBACK(OnDataSegmentCount, num_data_segments);
	ERROR_UNLESS(num_data_segments == 0 || NumTotalMemories() > 0, "data section without memory section");
	for (Index i = 0; i < num_data_segments; ++i) {
		Index memory_index;
		CHECK_RESULT(ReadIndex(&memory_index, "data segment memory index"));
		CALLBACK(BeginDataSegment, i, memory_index);
		CALLBACK(BeginDataSegmentInitExpr, i);
		CHECK_RESULT(ReadI32InitExpr(i));
		CALLBACK(EndDataSegmentInitExpr, i);

		Address data_size;
		const void* data;
		CHECK_RESULT(ReadBytes(&data, &data_size, "data segment data"));
		CALLBACK(OnDataSegmentData, i, data, data_size);
		CALLBACK(EndDataSegment, i);
	}
	CALLBACK0(EndDataSection);
	return Result::Ok;
}

Result SourceReader::BinaryReader::ReadSections() {
	Result result = Result::Ok;

	while (_state->offset < _state->size) {
		uint32_t section_code;
		Offset section_size;
		// Temporarily reset read_end_ to the full data size so the next section
		// can be read.
		_read_end = _state->size;
		CHECK_RESULT(ReadU32Leb128(&section_code, "section code"));
		CHECK_RESULT(ReadOffset(&section_size, "section size"));
		_read_end = _state->offset + section_size;
		if (section_code >= kBinarySectionCount) {
			PushErrorStream([&] (StringStream &stream) {
				stream << "invalid section code: " << section_code << "; max is " << kBinarySectionCount - 1;
			});
			return Result::Error;
		}

		BinarySection section = static_cast<BinarySection>(section_code);

		ERROR_UNLESS(_read_end <= _state->size,
				"invalid section size: extends past end");

		ERROR_UNLESS(_last_known_section == BinarySection::Invalid
			|| section == BinarySection::Custom || section > _last_known_section,
			"section " << GetSectionName(section) << " out of order");

#define V(Name, name, code)                             \
  case BinarySection::Name:                             \
    section_result = Read##Name##Section(section_size); \
    result = result | section_result;                   \
    break;

		Result section_result = Result::Error;

		switch (section) {
		WABT_FOREACH_BINARY_SECTION (V)
	case BinarySection::Invalid:
		WABT_UNREACHABLE;
		}

#undef V

		if (Failed(section_result)) {
			if (_options->stop_on_first_error) {
				return Result::Error;
			}

			// If we're continuing after failing to read this section, move the
			// offset to the expected section end. This way we may be able to read
			// further sections.
			_state->offset = _read_end;
		}

		ERROR_UNLESS(_state->offset == _read_end, "unfinished section (expected end: 0x" << std::hex << _read_end << ")");

		if (section != BinarySection::Custom) {
			_last_known_section = section;
		}
	}

	return result;
}

Result SourceReader::BinaryReader::ReadModule() {
	uint32_t magic = 0;
	CHECK_RESULT(ReadU32(&magic, "magic"));
	ERROR_UNLESS(magic == WABT_BINARY_MAGIC, "bad magic value");
	uint32_t version = 0;
	CHECK_RESULT(ReadU32(&version, "version"));
	ERROR_UNLESS(version == WABT_BINARY_VERSION, "bad wasm file version: " << std::hex << version << " (expected " << WABT_BINARY_VERSION << ")");
	CALLBACK(BeginModule, version);
	CHECK_RESULT(ReadSections());
	CALLBACK0(EndModule);
	return Result::Ok;
}

bool SourceReader::init(Module *module, const uint8_t *data, size_t size, const ReadOptions &opts) {
	_state = ReaderState(data, size);
	_targetModule = module;

	_opcodes.reserve(256);
	_labels.reserve(32);
	_labelStack.reserve(32);
	_jumpTable.reserve(32);

	if (_targetModule) {
		BinaryReader reader(this, &opts);
		return reader.ReadModule() != Result::Error;
	}
	return false;
}

}
