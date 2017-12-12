
#include "Binary.h"
#include "Module.h"

namespace wasm {


/* Names section */
Result SourceReader::BeginNamesSection(Offset size) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnFunctionNameSubsection(Index index, uint32_t name_type, Offset subsection_size) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnFunctionNamesCount(Index num_functions) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnFunctionName(Index function_index, StringView function_name) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnLocalNameSubsection(Index index, uint32_t name_type, Offset subsection_size) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnLocalNameFunctionCount(Index num_functions) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnLocalNameLocalCount(Index function_index, Index num_locals) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnLocalName(Index function_index, Index local_index, StringView local_name) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::EndNamesSection() {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}

/* Reloc section */
Result SourceReader::BeginRelocSection(Offset size) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnRelocCount(Index count, BinarySection section_code, StringView section_name) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnReloc(RelocType type, Offset offset, Index index, uint32_t addend) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::EndRelocSection() {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}

/* Linking section */
Result SourceReader::BeginLinkingSection(Offset size) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnStackGlobal(Index stack_global) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnSymbolInfoCount(Index count) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnSymbolInfo(StringView name, uint32_t flags) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnDataSize(uint32_t data_size) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnDataAlignment(uint32_t data_alignment) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnSegmentInfoCount(Index count) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnSegmentInfo(Index index, StringView name, uint32_t alignment, uint32_t flags) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::EndLinkingSection() {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}

/* Exception section */
Result SourceReader::BeginExceptionSection(Offset size) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnExceptionCount(Index count) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::OnExceptionType(Index index, TypeVector& sig) {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}
Result SourceReader::EndExceptionSection() {
	printf("%s\n", __FUNCTION__);
	return Result::Ok;
}


}
