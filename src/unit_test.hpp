/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef UNIT_TEST_HPP_INCLUDED
#define UNIT_TEST_HPP_INCLUDED

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace test {

struct failure_exception {
};

typedef std::function<void ()> UnitTest;
typedef std::function<void (int)> BenchmarkTest;
typedef std::function<void (int, const std::string&)> CommandLineBenchmarkTest;
typedef std::function<void (const std::vector<std::string>&)> UtilityProgram;

int register_test(const std::string& name, UnitTest test);
int register_benchmark(const std::string& name, BenchmarkTest test);
int register_benchmark_cl(const std::string& name, CommandLineBenchmarkTest test);
int register_utility(const std::string& name, UtilityProgram utility, bool needs_video);
bool utility_needs_video(const std::string& name);
bool run_tests(const std::vector<std::string>* tests=NULL);
void run_benchmarks(const std::vector<std::string>* benchmarks=NULL);
void run_command_line_benchmark(const std::string& benchmark_name, const std::string& arg);
void run_utility(const std::string& utility_name, const std::vector<std::string>& arg);

std::string run_benchmark(const std::string& name, BenchmarkTest fn);

}

#define CHECK(cond, msg) if(!(cond)) { std::cerr << __FILE__ << ":" << __LINE__ << ": TEST CHECK FAILED: " << #cond << ": " << msg << "\n"; throw test::failure_exception(); }

#define CHECK_CMP(a, b, cmp) CHECK((a) cmp (b), #a << ": " << (a) << "; " << #b << ": " << (b))

#define CHECK_EQ(a, b) CHECK_CMP(a, b, ==)
#define CHECK_NE(a, b) CHECK_CMP(a, b, !=)
#define CHECK_LE(a, b) CHECK_CMP(a, b, <=)
#define CHECK_GE(a, b) CHECK_CMP(a, b, >=)
#define CHECK_LT(a, b) CHECK_CMP(a, b, <)
#define CHECK_GT(a, b) CHECK_CMP(a, b, >)

//on mobile phones we don't do unit tests or benchmarks.
#if TARGET_OS_IPHONE || defined(TARGET_OS_HARMATTAN)

#define UNIT_TEST(name) \
	void TEST_##name()

#define BENCHMARK(name) \
	void BENCHMARK_##name(int benchmark_iterations)

#define BENCHMARK_LOOP

#define BENCHMARK_ARG(name, arg) \
	void BENCHMARK_ARG_##name(int benchmark_iterations, arg)

#define BENCHMARK_ARG_CALL(name, id, arg)

#define BENCHMARK_ARG_CALL_COMMAND_LINE(name)

#define UTILITY(name) void UTILITY_##name(const std::vector<std::string>& args)

#define COMMAND_LINE_UTILITY(name) \
	void UTILITY_##name(const std::vector<std::string>& args)

#else

#define UNIT_TEST(name) \
	void TEST_##name(); \
	static int TEST_VAR_##name = test::register_test(#name, TEST_##name); \
	void TEST_##name()

#define BENCHMARK(name) \
	void BENCHMARK_##name(int benchmark_iterations); \
	static int BENCHMARK_VAR_##name = test::register_benchmark(#name, BENCHMARK_##name); \
	void BENCHMARK_##name(int benchmark_iterations)

#define BENCHMARK_LOOP while(benchmark_iterations--)

#define BENCHMARK_ARG(name, arg) \
	void BENCHMARK_ARG_##name(int benchmark_iterations, arg)

#define BENCHMARK_ARG_CALL(name, id, arg) \
	void BENCHMARK_ARG_CALL_##name_##id(int benchmark_iterations) { \
		BENCHMARK_ARG_##name(benchmark_iterations, arg); \
	} \
	static int BENCHMARK_ARG_VAR_##name_##id = test::register_benchmark(#name " " #id, BENCHMARK_ARG_CALL_##name_##id);

#define BENCHMARK_ARG_CALL_COMMAND_LINE(name) \
	void BENCHMARK_ARG_CALL_##name(int benchmark_iterations, const std::string& arg) { \
		BENCHMARK_ARG_##name(benchmark_iterations, arg); \
	} \
	static int BENCHMARK_ARG_VAR_##name = test::register_benchmark_cl(#name, BENCHMARK_ARG_CALL_##name);

#define UTILITY(name) \
    void UTILITY_##name(const std::vector<std::string>& args); \
	static int UTILITY_VAR_##name = test::register_utility(#name, UTILITY_##name, true); \
	void UTILITY_##name(const std::vector<std::string>& args)

#define COMMAND_LINE_UTILITY(name) \
    void UTILITY_##name(const std::vector<std::string>& args); \
	static int UTILITY_VAR_##name = test::register_utility(#name, UTILITY_##name, false); \
	void UTILITY_##name(const std::vector<std::string>& args)

#endif

#endif
