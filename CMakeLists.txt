cmake_minimum_required(VERSION 3.20)
project(BinomialTree)

set(CMAKE_CXX_STANDARD 20)

# 1. Force Release Build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 2. HFT Optimization Flags (GCC/Clang)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
            -O3                 # Maximum optimization level
            -march=native       # Generate instructions specific to YOUR CPU (AVX2/AVX512)
            -ffast-math         # Allow reordering floating point math for speed
            -funroll-loops      # Unroll small loops to reduce branch prediction cost
            -fno-rtti           # Disable Run-Time Type Information (not needed here)
            -DNDEBUG            # Disable assertions (assert() costs time!)
    )
endif()

# 3. MSVC Optimization Flags (Windows)
if(MSVC)
    add_compile_options(
            /O2                 # Max speed
            /Oi                 # Intrinsic functions
            /Ot                 # Favor fast code
            /fp:fast            # Fast floating point model
    )
endif()


add_executable(BinomialTreeModel main.cpp Dividend.h binomial_tree.h
        cmake-build-debug-wsl/correct_binomial_tree.cpp)