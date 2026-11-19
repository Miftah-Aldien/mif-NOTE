cmake_minimum_required(VERSION 3.20)
project(mif-NOTE-CPP CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FIX MSVCP140.dll not found: pakai runtime STATIC (/MT), bukan dynamic (/MD)
# Supaya Mif-NOTE.exe bisa jalan di EeePC Windows 7 tanpa install VC Redist
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(Mif-NOTE WIN32 src/main.cpp)
target_link_libraries(Mif-NOTE PRIVATE comctl32 comdlg32 user32 gdi32)
