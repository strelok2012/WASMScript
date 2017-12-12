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

#include "Binary.h"
#include "Module.h"

#include <stdarg.h>

namespace wasm {

#define PRINT_CONTENT 0

#if (PRINT_CONTENT)
#define BINARY_PRINTF(...) printf(__VA_ARGS__)
#else
#define BINARY_PRINTF
#endif

#define CHECK_RESULT(expr) do { if (expr == ::wasm::Result::Error) { return ::wasm::Result::Error; } } while (0)

static Result CheckHasMemory(SourceReader *reader, Module *module, Opcode opcode) {
	if (!module->hasMemory()) {
		reader->PushErrorStream([&] (StringStream &stream) {
			stream << opcode.GetName() << " requires an imported or defined memory.";
		});
		return Result::Error;
	}
	return Result::Ok;
}

static Result CheckAlign(SourceReader *reader, uint32_t alignment_log2, Address natural_alignment) {
	if (alignment_log2 >= 32 || (1U << alignment_log2) > natural_alignment) {
		reader->PushErrorStream([&] (StringStream &stream) {
			stream << "Alignment must not be larger than natural alignment (" << natural_alignment << ")";
		});
		return Result::Error;
	}
	return Result::Ok;
}

static Result CheckAtomicAlign(SourceReader *reader, uint32_t alignment_log2, Address natural_alignment) {
	if (alignment_log2 >= 32 || (1U << alignment_log2) != natural_alignment) {
		reader->PushErrorStream([&] (StringStream &stream) {
			stream << "Alignment must be equal to natural alignment (" << natural_alignment << ")";
		});
		return Result::Error;
	}
	return Result::Ok;
}

static Result CheckLocal(SourceReader *reader, Func *func, Index local_index) {
	Index locals_count = func->types.size();
	if (local_index >= locals_count) {
		reader->PushErrorStream([&] (StringStream &stream) {
			stream << "invalid local_index: " << local_index << " (max " << locals_count << ")";
		});
		return Result::Error;
	}
	return Result::Ok;
}

static Result CheckGlobal(SourceReader *reader, Module *module, Index global_index) {
	auto & idx = module->getGlobalIndexVec();
	if (global_index >= idx.size()) {
		reader->PushErrorStream([&] (StringStream &stream) {
			stream << "invalid global_index: " << global_index << " (max " << idx.size() << ")";
		});
		return Result::Error;
	}
	return Result::Ok;
}


bool SourceReader::OnError(StringStream & message) {
	BINARY_PRINTF("%s\n", message.str().data());
	return true;
}

/* Module */
Result SourceReader::BeginModule(uint32_t version) { return Result::Ok; }
Result SourceReader::EndModule() { return Result::Ok; }

/* Custom section */
Result SourceReader::BeginCustomSection(Offset size, const StringView &section_name) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::EndCustomSection() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	return Result::Ok;
}

/* Code section */
Result SourceReader::BeginCodeSection(Offset size) { return Result::Ok; }
Result SourceReader::OnFunctionBodyCount(Index count) { return Result::Ok; }
Result SourceReader::BeginFunctionBody(Index index) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, index);
	if (index >= _targetModule->_funcs.size()) {
		PushErrorStream([&] (StringStream &stream) { stream << "Invalid function code index: " << index; });
		return Result::Error;
	}
	_currentFunc = &_targetModule->_funcs[index];
	Module::Signature* sig = _targetModule->getSignature(_currentFunc->sig);

	_currentFunc->resultsCount = sig->results.size();
	for (auto &it : sig->params) {
		_currentFunc->types.emplace_back(it);
	}

	CHECK_RESULT(_typechecker.BeginFunction(&sig->results));
	PushLabel(sig->results.size(), 0, kInvalidIndex);

	return Result::Ok;
}
Result SourceReader::EndFunctionBody(Index index) {
	CHECK_RESULT(_typechecker.EndFunction());
	PopLabel(_opcodes.size());
	EmitOpcodeValue(Opcode::Return, _labels.front().results);

	_currentFunc->opcodes = _opcodes;
	_currentFunc->labels = _labels;
	_currentFunc->jumpTable = _jumpTable;

	_opcodes.clear();
	_labels.clear();
	_jumpTable.clear();

	BINARY_PRINTF("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::EndCodeSection() { return Result::Ok; }

Result SourceReader::OnLocalDeclCount(Index count) { return Result::Ok; }
Result SourceReader::OnLocalDecl(Index decl_index, Index count, Type type) {
	BINARY_PRINTF("%s %u %u\n", __FUNCTION__, decl_index, count);

	for (Index i = 0; i < count; ++i) {
		_currentFunc->types.emplace_back(type);
	}

  //if (decl_index == current_func_->local_decl_count - 1) {
    /* last local declaration, allocate space for all locals. */
    //CHECK_RESULT(EmitOpcode(Opcode::InterpAlloca));
  //  CHECK_RESULT(EmitI32(current_func_->local_count));
 // }
	return Result::Ok;
}


/* Function expressions; called between BeginFunctionBody and
 EndFunctionBody */

Result SourceReader::OnAtomicLoadExpr(Opcode opcode, uint32_t alignment_log2, Address offset) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, opcode));
	CHECK_RESULT(CheckAtomicAlign(this, alignment_log2, opcode.GetMemorySize()));
	CHECK_RESULT(_typechecker.OnAtomicLoad(opcode));
	EmitOpcodeValue(opcode, 0, offset); // memory_index offset
	return Result::Ok;
}
Result SourceReader::OnAtomicStoreExpr(Opcode opcode, uint32_t alignment_log2, Address offset) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, opcode));
	CHECK_RESULT(CheckAtomicAlign(this, alignment_log2, opcode.GetMemorySize()));
	CHECK_RESULT(_typechecker.OnAtomicStore(opcode));
	EmitOpcodeValue(opcode, 0, offset); // memory_index offset
	return Result::Ok;
}
Result SourceReader::OnAtomicRmwExpr(Opcode opcode, uint32_t alignment_log2, Address offset) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, opcode));
	CHECK_RESULT(CheckAtomicAlign(this, alignment_log2, opcode.GetMemorySize()));
	CHECK_RESULT(_typechecker.OnAtomicRmw(opcode));
	EmitOpcodeValue(opcode, 0, offset); // memory_index offset
	return Result::Ok;
}
Result SourceReader::OnAtomicRmwCmpxchgExpr(Opcode opcode, uint32_t alignment_log2, Address offset) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, opcode));
	CHECK_RESULT(CheckAtomicAlign(this, alignment_log2, opcode.GetMemorySize()));
	CHECK_RESULT(_typechecker.OnAtomicRmwCmpxchg(opcode));
	EmitOpcodeValue(opcode, 0, offset);
	return Result::Ok;
}
Result SourceReader::OnAtomicWaitExpr(Opcode opcode, uint32_t alignment_log2, Address offset) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, opcode));
	CHECK_RESULT(CheckAtomicAlign(this, alignment_log2, opcode.GetMemorySize()));
	CHECK_RESULT(_typechecker.OnAtomicWait(opcode));
	EmitOpcodeValue(opcode, 0, offset);
	return Result::Error;
}
Result SourceReader::OnAtomicWakeExpr(Opcode opcode, uint32_t alignment_log2, Address offset) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, opcode));
	CHECK_RESULT(CheckAtomicAlign(this, alignment_log2, opcode.GetMemorySize()));
	CHECK_RESULT(_typechecker.OnAtomicWake(opcode));
	EmitOpcodeValue(opcode, 0, offset);
	return Result::Error;
}

Result SourceReader::OnUnaryExpr(Opcode opcode) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnUnary(opcode));
	EmitOpcodeValue(opcode, 0, 0);
	return Result::Ok;
}
Result SourceReader::OnBinaryExpr(Opcode opcode) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnBinary(opcode));
	EmitOpcodeValue(opcode, 0, 0);
	return Result::Ok;
}

Result SourceReader::OnBlockExpr(Index num_types, Type* sig_types) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, num_types);
	TypeVector sig(sig_types, sig_types + num_types);
	CHECK_RESULT(_typechecker.OnBlock(&sig));
	PushLabel(num_types, _typechecker.type_stack_size(), kInvalidIndex);
	return Result::Ok;
}
Result SourceReader::OnBrExpr(Index depth) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, depth);
	CHECK_RESULT(_typechecker.OnBr(depth));
	EmitOpcodeValue(Opcode::Br, _labelStack.at(_labelStack.size() - 1 - depth), 0);
	return Result::Ok;
}
Result SourceReader::OnBrIfExpr(Index depth) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, depth);
	CHECK_RESULT(_typechecker.OnBrIf(depth));
	EmitOpcodeValue(Opcode::Br, _labelStack.at(_labelStack.size() - 1 - depth), 0);
	return Result::Ok;
}
Result SourceReader::OnBrTableExpr(Index num_targets, Index* target_depths, Index default_target_depth) {
	BINARY_PRINTF("%s\n", __FUNCTION__);

	auto size = _labelStack.size();
	auto target = _jumpTable.size();
	_jumpTable.push_back(num_targets);

	for (Index i = 0; i <= num_targets; ++i) {
		Index depth = i != num_targets ? target_depths[i] : default_target_depth;
		CHECK_RESULT(_typechecker.OnBrTableTarget(depth));
	}

	CHECK_RESULT(_typechecker.BeginBrTable());
	for (Index i = 0; i < num_targets; ++i) {
		_jumpTable.push_back(_labelStack[size - 1 - target_depths[i]]);
		CHECK_RESULT(_typechecker.OnBrTableTarget(target_depths[i]));
	}
	_jumpTable.push_back(size - 1 - default_target_depth);
	CHECK_RESULT(_typechecker.OnBrTableTarget(default_target_depth));
	CHECK_RESULT(_typechecker.EndBrTable());

	EmitOpcodeValue(Opcode::BrTable, target, 0);
	return Result::Ok;
}
Result SourceReader::OnLoopExpr(Index num_types, Type* sig_types) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	TypeVector sig(sig_types, sig_types + num_types);
	CHECK_RESULT(_typechecker.OnLoop(&sig));
	PushLabel(num_types, _typechecker.type_stack_size(), _opcodes.size());
	return Result::Ok;
}
Result SourceReader::OnIfExpr(Index num_types, Type* sig_types) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	TypeVector sig(sig_types, sig_types + num_types);
	CHECK_RESULT(_typechecker.OnIf(&sig));
	PushLabel(num_types, _typechecker.type_stack_size(), kInvalidIndex);
	EmitOpcodeValue(Opcode::If, _labelStack.back(), 0);
	return Result::Ok;
}
Result SourceReader::OnElseExpr() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnElse());
	auto backLabel = _labels.at(_labelStack.back());
	EmitOpcodeValue(Opcode::Else, _labels.size(), 0);
	PopLabel(_opcodes.size());
	PushLabel(backLabel.results, backLabel.stack, kInvalidIndex);
	return Result::Ok;
}
Result SourceReader::OnEndExpr() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnEnd());
	PopLabel(_opcodes.size());
	return Result::Ok;
}
Result SourceReader::OnDropExpr() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnDrop());
	EmitOpcodeValue(Opcode::Drop, 0, 0);
	return Result::Ok;
}

Result SourceReader::OnCallExpr(Index func_index) {
	BINARY_PRINTF("%s\n", __FUNCTION__);

	auto sig = _targetModule->getFuncSignature(func_index);
	if (!sig.first) {
		return Result::Error;
	}

	CHECK_RESULT(_typechecker.OnCall(&sig.first->params, &sig.first->results));
	EmitOpcodeValue(Opcode::Call, func_index);
	return Result::Ok;
}
Result SourceReader::OnCallIndirectExpr(Index sig_index) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	if (!_targetModule->hasTable()) {
		PushErrorStream([&] (StringStream &stream) { stream << "found call_indirect operator, but no table"; });
		return Result::Error;
	}

	Module::Signature* sig = _targetModule->getSignature(sig_index);
	CHECK_RESULT(_typechecker.OnCallIndirect(&sig->params, &sig->results));
	EmitOpcodeValue(Opcode::Call, sig_index, 0); // sig index, table index
	return Result::Ok;
}

Result SourceReader::OnCompareExpr(Opcode opcode) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	return OnBinaryExpr(opcode);
}
Result SourceReader::OnConvertExpr(Opcode opcode) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	return OnUnaryExpr(opcode);
}

Result SourceReader::OnCurrentMemoryExpr() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, Opcode::CurrentMemory));
	CHECK_RESULT(_typechecker.OnCurrentMemory());
	EmitOpcodeValue(Opcode::CurrentMemory, 0, 0); // memory_index
	return Result::Ok;
}

Result SourceReader::OnI32ConstExpr(uint32_t value) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnConst(Type::I32));
	EmitOpcodeValue(Opcode::I32Const, value);
	return Result::Ok;
}
Result SourceReader::OnI64ConstExpr(uint64_t value) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnConst(Type::I64));
	EmitOpcodeValue(Opcode::I64Const, value);
	return Result::Ok;
}
Result SourceReader::OnF32ConstExpr(uint32_t value_bits) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnConst(Type::F32));
	EmitOpcodeValue(Opcode::F32Const, value_bits);
	return Result::Ok;
}
Result SourceReader::OnF64ConstExpr(uint64_t value_bits) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnConst(Type::F64));
	EmitOpcodeValue(Opcode::F64Const, value_bits);
	return Result::Ok;
}

Result SourceReader::OnGetGlobalExpr(Index global_index) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckGlobal(this, _targetModule, global_index));
	auto type = _targetModule->getGlobalType(global_index);
	if (type.first == Type::Void) {
		return Result::Error;
	}

	CHECK_RESULT(_typechecker.OnGetGlobal(type.first));
	EmitOpcodeValue(Opcode::GetGlobal, global_index);
	return Result::Ok;
}
Result SourceReader::OnSetGlobalExpr(Index global_index) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckGlobal(this, _targetModule, global_index));
	auto type = _targetModule->getGlobalType(global_index);
	if (type.first == Type::Void) {
		return Result::Error;
	}

	if (!type.second) {
		PushErrorStream([&] (StringStream &stream) {
			stream << "can't set_global on immutable global at index " << global_index;
		});
		return Result::Error;
	}
	CHECK_RESULT(_typechecker.OnSetGlobal(type.first));
	EmitOpcodeValue(Opcode::SetGlobal, global_index);
	return Result::Ok;
}

Result SourceReader::OnGetLocalExpr(Index local_index) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckLocal(this, _currentFunc, local_index));
	Type type = _currentFunc->types[local_index];
	// Get the translated index before calling _typechecker.OnGetLocal because it
	// will update the type stack size. We need the index to be relative to the
	// old stack size.
	//Index translated_local_index = TranslateLocalIndex(local_index);
	CHECK_RESULT(_typechecker.OnGetLocal(type));
	EmitOpcodeValue(Opcode::GetLocal, local_index);
	return Result::Ok;
}
Result SourceReader::OnSetLocalExpr(Index local_index) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckLocal(this, _currentFunc, local_index));
	Type type = _currentFunc->types[local_index];
	CHECK_RESULT(_typechecker.OnSetLocal(type));
	EmitOpcodeValue(Opcode::SetLocal, local_index);
	return Result::Ok;
}

Result SourceReader::OnGrowMemoryExpr() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, Opcode::GrowMemory));
	CHECK_RESULT(_typechecker.OnGrowMemory());
	EmitOpcodeValue(Opcode::GrowMemory, 0, 0); // memory index
	return Result::Ok;
}

Result SourceReader::OnLoadExpr(Opcode opcode, uint32_t alignment_log2, Address offset) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, opcode));
	CHECK_RESULT(CheckAlign(this, alignment_log2, opcode.GetMemorySize()));
	CHECK_RESULT(_typechecker.OnLoad(opcode));
	EmitOpcodeValue(opcode, offset);
	return Result::Ok;
}
Result SourceReader::OnStoreExpr(Opcode opcode, uint32_t alignment_log2, Address offset) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckHasMemory(this, _targetModule, opcode));
	CHECK_RESULT(CheckAlign(this, alignment_log2, opcode.GetMemorySize()));
	CHECK_RESULT(_typechecker.OnStore(opcode));
	EmitOpcodeValue(opcode, offset);
	return Result::Ok;
}


Result SourceReader::OnTeeLocalExpr(Index local_index) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(CheckLocal(this, _currentFunc, local_index));
	Type type = _currentFunc->types[local_index];
	CHECK_RESULT(_typechecker.OnTeeLocal(type));
	EmitOpcodeValue(Opcode::TeeLocal, local_index);
	return Result::Ok;
}

Result SourceReader::OnReturnExpr() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	Index drop_count, keep_count;
	CHECK_RESULT(_typechecker.OnReturn());
	EmitOpcodeValue(Opcode::Return, _currentFunc->resultsCount); // return count
	return Result::Ok;
}

Result SourceReader::OnSelectExpr() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnSelect());
	EmitOpcodeValue(Opcode::Select, 0, 0);
	return Result::Ok;
}

Result SourceReader::OnUnreachableExpr() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	CHECK_RESULT(_typechecker.OnUnreachable());
	EmitOpcodeValue(Opcode::Unreachable, 0, 0);
	return Result::Ok;
}


Result SourceReader::OnOpcode(Opcode Opcode) { return Result::Ok; }
Result SourceReader::OnOpcodeBare() { return Result::Ok; }
Result SourceReader::OnOpcodeUint32(uint32_t value) { return Result::Ok; }
Result SourceReader::OnOpcodeIndex(Index value) { return Result::Ok; }
Result SourceReader::OnOpcodeUint32Uint32(uint32_t value, uint32_t value2) { return Result::Ok; }
Result SourceReader::OnOpcodeUint64(uint64_t value) { return Result::Ok; }
Result SourceReader::OnOpcodeF32(uint32_t value) { return Result::Ok; }
Result SourceReader::OnOpcodeF64(uint64_t value) { return Result::Ok; }
Result SourceReader::OnOpcodeBlockSig(Index num_types, Type* sig_types) { return Result::Ok; }
Result SourceReader::OnCatchExpr(Index except_index) { return Result::Ok; }
Result SourceReader::OnCatchAllExpr() { return Result::Ok; }
Result SourceReader::OnRethrowExpr(Index depth) { return Result::Ok; }
Result SourceReader::OnThrowExpr(Index except_index) { return Result::Ok; }
Result SourceReader::OnTryExpr(Index num_types, Type* sig_types) { return Result::Ok; }

Result SourceReader::OnNopExpr() { return Result::Ok; }
Result SourceReader::OnEndFunc() {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	return Result::Ok;
}

void SourceReader::EmitOpcodeValue(Opcode opcode, uint32_t value1, uint32_t value2) {
	_opcodes.emplace_back(opcode, value1, value2);
}
void SourceReader::EmitOpcodeValue(Opcode opcode, uint64_t value) {
	_opcodes.emplace_back(opcode, value);
}


void SourceReader::PushLabel(Index results, Index stack, Index position) {
	_labels.emplace_back(Func::Label{results, stack, position});
	_labelStack.push_back(_labels.size() - 1);
}

void SourceReader::PopLabel(Index position) {
	if (_labels.back().offset == kInvalidOffset) {
		_labels.back().offset = position;
	}
	_labelStack.pop_back();
}

}
