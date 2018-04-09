CXX := g++
CXXFLAGS := -fno-rtti -O0 -g
PLUGIN_CXXFLAGS := -fpic

LLVM_CXXFLAGS := `llvm-config --cxxflags`
LLVM_LDFLAGS := `llvm-config --ldflags`

LLVM_LDFLAGS_NOLIBS := `llvm-config --ldflags`
PLUGIN_LDFLAGS := -shared

LLVM_INCLUDES := -I/home/parallels/llvm-4.0.0.obj/include
LLVM_LIB := -L/home/parallels/llvm-4.0.0.obj/lib


CLANG_LIBS := \
	-Wl,--start-group \
	-lclangAST \
	-lclangASTMatchers \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangEdit \
	-lclangRewrite \
	-lclangRewriteFrontend \
	-lclangStaticAnalyzerFrontend \
	-lclangStaticAnalyzerCheckers \
	-lclangStaticAnalyzerCore \
	-lclangSerialization \
	-lclangToolingCore \
	-lclangTooling \
	-lclangFormat \
	-Wl,--end-group

all: control_flow_analysis.so

control_flow_analysis.so: control_flow_analysis.o
	$(CXX) $(PLUGIN_LDFLAGS) -o $@ $^ $(LLVM_LIB) $(LLVM_LDFLAGS)

control_flow_analysis.o: control_flow_analysis.cpp
	$(CXX) -c $^ $(LLVM_INCLUDES) $(LLVM_CXXFLAGS)


