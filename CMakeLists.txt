cmake_minimum_required(VERSION 3.20)
project(BinomialTreeModel)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(BinomialTreeModel main.cpp Dividend.h binomial_tree.h)