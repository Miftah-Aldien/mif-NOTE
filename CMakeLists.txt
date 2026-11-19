cmake_minimum_required(VERSION 3.20)
project(mif-NOTE-CPP CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Folder Scintilla dikirim dari workflow lewat -DSCINTILLA_ROOT=...
if(NOT DEFINED SCINTILLA_ROOT)
  if(EXISTS "${CMAKE_SOURCE_DIR}/scintilla-src/scintilla-5.4.1/include/Scintilla.h")
    set(SCINTILLA_ROOT "${CMAKE_SOURCE_DIR}/scintilla-src/scintilla-5.4.1")
  elseif(EXISTS "${CMAKE_SOURCE_DIR}/scintilla/include/Scintilla.h")
    set(SCINTILLA_ROOT "${CMAKE_SOURCE_DIR}/scintilla")
  endif()
endif()

add_executable(Mif-NOTE WIN32 src/main.cpp src/mif-note.rc)
target_link_libraries(Mif-NOTE PRIVATE comctl32 comdlg32 user32 gdi32)

if(DEFINED SCINTILLA_ROOT)
  message(STATUS "Scintilla root: ${SCINTILLA_ROOT}")
  target_include_directories(Mif-NOTE PRIVATE "${SCINTILLA_ROOT}/include")
else()
  message(WARNING "SCINTILLA_ROOT tidak ketemu, build akan gagal di Scintilla.h")
endif()
