cmake_minimum_required(VERSION 3.22)
add_executable(SeniorProject main.cpp common.h lexer.cpp lexer.h expr.h parser.cpp parser.h codegen.h
        expr.cpp
        token.cpp
        PostFixExprVisualizer.h
        type_checker.h)
llvm_map_components_to_libnames(llvm_libs all ${LLVM_TARGETS_TO_BUILD} support core irreader codegen mc mca mcparser option irprinter passes)
target_link_libraries(SeniorProject ${llvm_libs})
