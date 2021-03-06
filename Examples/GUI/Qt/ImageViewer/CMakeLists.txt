cmake_minimum_required(VERSION 2.8.8)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
if(POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW) # CMake 3.0
endif()
project(QtImageViewer)

find_package(VTK COMPONENTS
  vtkGUISupportQt
  vtkIOImage
  vtkInteractionImage
  vtkRenderingOpenGL
  vtkTestingCore
)
include(${VTK_USE_FILE})

if("${VTK_QT_VERSION}" STREQUAL "")
  message(FATAL_ERROR "VTK was not built with Qt")
endif()

set( Srcs main.cxx )

if(VTK_QT_VERSION VERSION_GREATER "4")
  find_package(Qt5Core REQUIRED QUIET)

  add_executable(qtimageviewer ${Srcs})
  qt5_use_modules(qtimageviewer Core Gui Widgets)
  target_link_libraries(qtimageviewer ${VTK_LIBRARIES})
else()
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
  # Use what VTK built with
  set(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
  set(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
  set(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
  include_directories(${QT_INCLUDE_DIR})

  add_executable(qtimageviewer ${Srcs})
  target_link_libraries(qtimageviewer ${QT_LIBRARIES} ${VTK_LIBRARIES})
endif()
