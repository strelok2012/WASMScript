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

#include "ImportDelegate.h"
#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include <limits.h>

#include "src/Module.h"

/*
using namespace wabt;
using namespace wabt::interp;

static int s_verbose;
static const char* s_infile;
static Thread::Options s_thread_options;
static Stream* s_trace_stream;
static Features s_features;
std::string callExport;

std::unique_ptr<FileStream> s_stdout_stream;
static std::unique_ptr<FileStream> s_log_stream;

enum class RunVerbosity {
	Quiet = 0,
	Verbose = 1,
};


static void ParseOptions(int argc, char** argv) {
  OptionParser parser("wasm-interp", "");

  parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
    s_verbose++;
    s_log_stream = FileStream::CreateStdout();
  });
  parser.AddHelpOption();
  s_features.AddOptions(&parser);
  parser.AddOption('E', "run-my-export", "SIZE",
                     "Run export",
                     [](const std::string& argument) {
	  	  	  	  	  	  callExport = argument;
                     });
  parser.AddOption('C', "call-stack-size", "SIZE",
                   "Size in elements of the call stack",
                   [](const std::string& argument) {
                     // TODO(binji): validate.
                     s_thread_options.call_stack_size = atoi(argument.c_str());
                   });
  parser.AddOption('t', "trace", "Trace execution",
                   []() { s_trace_stream = s_stdout_stream.get(); });

  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) { s_infile = argument; });
  parser.Parse(argc, argv);
}


static void RunExport(std::string exportName, interp::Module* module, Executor* executor, RunVerbosity verbose) {
	for (const interp::Export& export_ : module->exports) {
		if (export_.name == exportName) {
			TypedValues args;
			TypedValues results;
			Value myValue;
			myValue.i32 = 41;
			TypedValue myArg = TypedValue(Type::I32, myValue);
			args.emplace_back(myArg);
			ExecResult exec_result = executor->RunExport(&export_, args);
			for (TypedValue& value : exec_result.values) {
				switch (value.type) {
				case Type::I32:
					std::cout << "export result i32 " << value.value.i32
							<< "\r\n";
					break;
				default:
					break;
				}

			}
			if (verbose == RunVerbosity::Verbose) {
				WriteCall(s_stdout_stream.get(), string_view(), export_.name,
						args, exec_result.values, exec_result.result);
			}
		}
	}
}

static wabt::Result ReadModule(const char* module_filename, Environment* env, ErrorHandler* error_handler, DefinedModule** out_module) {
	wabt::Result result;
	std::vector<uint8_t> file_data;

	*out_module = nullptr;

	result = ReadFile(module_filename, &file_data);
	if (Succeeded(result)) {
		const bool kReadDebugNames = true;
		const bool kStopOnFirstError = true;
		ReadBinaryOptions options(s_features, s_log_stream.get(),
				kReadDebugNames, kStopOnFirstError);
		result = ReadBinaryInterp(env, DataOrNull(file_data), file_data.size(),
				&options, error_handler, out_module);

		if (Succeeded(result)) {
			if (s_verbose)
				env->DisassembleModule(s_stdout_stream.get(), *out_module);
		}
	}
	return result;
}

static void InitEnvironment(Environment* env) {
	HostModule* host_module = env->AppendHostModule("env");
	host_module->import_delegate.reset(new ImportDelegate());
}

static wabt::Result ReadAndRunModule(const char* module_filename) {
	wabt::Result result;
	Environment env;
	InitEnvironment(&env);

	ErrorHandlerFile error_handler(Location::Type::Binary);
	DefinedModule* module = nullptr;
	result = ReadModule(module_filename, &env, &error_handler, &module);
	if (Succeeded(result)) {
		Executor executor(&env, s_trace_stream, s_thread_options);
		ExecResult exec_result = executor.RunStartFunction(module);
		if (exec_result.result == interp::Result::Ok) {
			RunExport(callExport, module, &executor, RunVerbosity::Verbose);
		} else {
			WriteResult(s_stdout_stream.get(), "error running start function",
					exec_result.result);
		}
	}
	return result;
}

int ProgramMain(int argc, char** argv) {
	InitStdio();
	ParseOptions(argc, argv);
	s_stdout_stream = FileStream::CreateStdout();

	wabt::Result result = ReadAndRunModule(s_infile);
	return result != wabt::Result::Ok;
}*/


void process_file_data(const char *filename, const uint8_t *data, size_t size) {
	wasm::Module mod;

	mod.init(data, size);
}

void read_file(const char *filename) {
	char buf[PATH_MAX + 1] = { 0 };

	FILE *fp;
	fp = fopen(filename, "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		long int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		uint8_t buf[size];
		if (fread(buf, size, 1, fp) == 1) {
			process_file_data(filename, buf, size);
		}

		fclose(fp);
	}
}

int main(int argc, char** argv) {
	char buf[PATH_MAX + 1] = { 0 };

	char *cwd = nullptr;

	if (argc > 1) {
		cwd = realpath(argv[1], buf);
	}

	if (cwd) {
		read_file(cwd);
	}

	return 0;
}
