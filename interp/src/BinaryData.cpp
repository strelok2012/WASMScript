
#include "Binary.h"
#include "Module.h"

#define PRINT_CONTENT 0

#if (PRINT_CONTENT)
#define BINARY_PRINTF(...) printf(VA_ARGS)
#else
#define BINARY_PRINTF
#endif

namespace wasm {

/* Elem section */
Result SourceReader::BeginElemSection(Offset size) { return Result::Ok; }
Result SourceReader::OnElemSegmentCount(Index count) {
	_targetModule->_elements.reserve(count);
	return Result::Ok;
}
Result SourceReader::BeginElemSegment(Index index, Index table_index) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, table_index);

	_currentIndex = table_index;
	return Result::Ok;
}
Result SourceReader::BeginElemSegmentInitExpr(Index index) { return Result::Ok; }
Result SourceReader::EndElemSegmentInitExpr(Index index) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, _initExprValue.value.i32);
	if (_initExprValue.type != Type::I32) {
		PushErrorStream([&] (StringStream &stream) { stream << "Invalid initializer type for element_segment"; });
		return Result::Error;
	}
	return Result::Ok;
}
Result SourceReader::OnElemSegmentFunctionIndexCount(Index index, Index count) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, count);
	if (index != _targetModule->_elements.size()) {
		PushErrorStream([&] (StringStream &stream) { stream << "Invalid elements block indext"; });
		return Result::Error;
	}
	_targetModule->_elements.emplace_back(_currentIndex, _initExprValue.value.i32, count);
	return Result::Ok;
}
Result SourceReader::OnElemSegmentFunctionIndex(Index segment_index, Index func_index) {
	BINARY_PRINTF("%s %u %u\n", __FUNCTION__, segment_index, func_index);
	_targetModule->_elements.at(segment_index).values.push_back(func_index);
	return Result::Ok;
}
Result SourceReader::EndElemSegment(Index index) { return Result::Ok; }
Result SourceReader::EndElemSection() { return Result::Ok; }


/* Data section */
Result SourceReader::BeginDataSection(Offset size) { return Result::Ok; }
Result SourceReader::OnDataSegmentCount(Index count) {
	_targetModule->_data.reserve(count);
	return Result::Ok;
}
Result SourceReader::BeginDataSegment(Index index, Index memory_index) {
	_currentIndex = memory_index;
	return Result::Ok;
}
Result SourceReader::BeginDataSegmentInitExpr(Index index) {
	return Result::Ok;
}
Result SourceReader::EndDataSegmentInitExpr(Index index) {
	if (_initExprValue.type != Type::I32) {
		PushErrorStream([&] (StringStream &stream) { stream << "Invalid initializer type for data_segment"; });
		return Result::Error;
	}
	return Result::Ok;
}
Result SourceReader::OnDataSegmentData(Index index, const void* data, Address size) {
	BINARY_PRINTF("%s %u %u %u %u\n", __FUNCTION__, index, _currentIndex, _initExprValue.value.i32, size);
	if (index != _targetModule->_data.size()) {
		PushErrorStream([&] (StringStream &stream) { stream << "Invalid elements block index"; });
		return Result::Error;
	}
	_targetModule->_data.emplace_back(_currentIndex, _initExprValue.value.i32, size, (const uint8_t *)data);
	return Result::Ok;
}
Result SourceReader::EndDataSegment(Index index) { return Result::Ok; }
Result SourceReader::EndDataSection() { return Result::Ok; }


/* InitExpr - used by elem, data and global sections; these functions are
 * only called between calls to Begin*InitExpr and End*InitExpr */
Result SourceReader::OnInitExprF32ConstExpr(Index index, uint32_t value) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, value);
	_initExprValue.type = Type::F32;
	_initExprValue.value.f32_bits = value;
	return Result::Ok;
}
Result SourceReader::OnInitExprF64ConstExpr(Index index, uint64_t value) {
	BINARY_PRINTF("%s %lu\n", __FUNCTION__, value);
	_initExprValue.type = Type::F64;
	_initExprValue.value.f64_bits = value;
	return Result::Ok;
}
Result SourceReader::OnInitExprGetGlobalExpr(Index index, Index global_index) {
	BINARY_PRINTF("%s\n", __FUNCTION__);
	/*  if (global_index >= num_global_imports_) {
	    PrintError("initializer expression can only reference an imported global");
	    return wabt::Result::Error;
	  }
	  Global* ref_global = GetGlobalByModuleIndex(global_index);
	  if (ref_global->mutable_) {
	    PrintError("initializer expression cannot reference a mutable global");
	    return wabt::Result::Error;
	  }
	  init_expr_value_ = ref_global->typed_value;
	  return wabt::Result::Ok;*/
	return Result::Ok;
}
Result SourceReader::OnInitExprI32ConstExpr(Index index, uint32_t value) {
	BINARY_PRINTF("%s %u\n", __FUNCTION__, value);
	_initExprValue.type = Type::I32;
	_initExprValue.value.i32 = value;
	return Result::Ok;
}
Result SourceReader::OnInitExprI64ConstExpr(Index index, uint64_t value) {
	BINARY_PRINTF("%s %lu\n", __FUNCTION__, value);
	_initExprValue.type = Type::I64;
	_initExprValue.value.i64 = value;
	return Result::Ok;
}

}
