cmake_minimum_required(VERSION 3.1)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
if(POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW) # CMake 3.0
endif()
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW) # CMake 3.0
endif()
if(POLICY CMP0053)
  cmake_policy(SET CMP0053 NEW) # CMake 3.1
endif()

set(TARGET_NAME "SPPD")
PROJECT(${TARGET_NAME})

find_package(VTK COMPONENTS
  vtkCommonCore
  vtkFiltersCore
  vtkInfovisCore
  vtkInteractionStyle
  vtkViewsQt
  vtkjsoncpp
)
include(${VTK_USE_FILE})

if("${VTK_QT_VERSION}" STREQUAL "")
  message(FATAL_ERROR "VTK was not built with Qt")
endif()

# Use the include path and library for Qt that is used by VTK.
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  libdocker/inc
)

# Set your files and resources here
SET(DataModelSrcs
    dataModel/Connect.cpp dataModel/Connect.h
    dataModel/Result.cpp dataModel/Result.h
    dataModel/Part.cpp dataModel/Part.h
    dataModel/Case.cpp dataModel/Case.h
    dataModel/Doc.cpp dataModel/Doc.h
)

SET(QtVTKSrcs 
    VtkExt/sppPlaneWidget.cpp VtkExt/sppPlaneWidget.h
    VtkExt/sppScalarBarActorAdapter.cpp VtkExt/sppScalarBarActorAdapter.h
    VtkExt/sppScalarBarRepresentation.cpp VtkExt/sppScalarBarRepresentation.h
    VtkExt/sppScalarBarWidget.cpp VtkExt/sppScalarBarWidget.h
    VtkExt/sppScalarBarActor.cpp VtkExt/sppScalarBarActor.h
    VtkExt/sppVtk.cpp VtkExt/sppVtk.h
)

SET(libdockerSrcs
    libdocker/src/docker.c libdocker/inc/docker.h
)


set( Srcs
    main.cxx
    dlgLogIn.cpp
    dlgOpenCase.cpp
    
    pageMeshDisplay.cpp
    pageResultDisplay.cpp
    pageQueryResults.cpp
    pageCuttingPlane.cpp
    sheetDisplayOptions.cpp
    editQueryResults.cpp
    QtVtkView.cxx
    RadioButtonTreeView.cpp
    MainWnd.cxx
    TreeItem.cpp
    TreeModel.cpp
        
  ${QtVTKSrcs}
  ${DataModelSrcs}
  ${libdockerSrcs}
)

set( Hdrs
    dlgLogIn.h
    dlgOpenCase.h

    pageMeshDisplay.h
    pageResultDisplay.h
    pageQueryResults.h
    pageCuttingPlane.h
    sheetDisplayOptions.h
    editQueryResults.h
    QtVtkView.h
    RadioButtonTreeView.h
    MainWnd.h
    TreeItem.h
    TreeModel.h
)

set( MOC_Hdrs QtVtkView.h )

set( UIs RltView.ui )

set( QRCs Icons/icons.qrc )

SOURCE_GROUP("Source Files/DataModel Files" FILES
  ${DataModelSrcs}
)
SOURCE_GROUP("Source Files/VtkExt Files" FILES
  ${QtVTKSrcs}
)
SOURCE_GROUP("Source Files/libdocker Files" FILES
  ${libdockerSrcs}
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

#libcurl
find_package(CURL REQUIRED)

include_directories (
    ${CURL_INCLUDE_DIR}
)

# The rest should just work (sure...)
  # We have ui files, this will bring in the macro: qt5_wrap_ui
  find_package(Qt5 COMPONENTS Widgets REQUIRED QUIET)
  qt5_wrap_ui(UI_Srcs ${UIs})
  qt5_add_resources(QRC_Srcs ${QRCs} )

  source_group("Resources" FILES
    ${UIs}
    ${QRCs}
    ${EXE_ICON} # Not present
  )

  source_group("Generated" FILES
    ${UI_Srcs}
    ${MOC_Srcs}
    ${QRC_Srcs}
  )

  add_executable(${TARGET_NAME} MACOSX_BUNDLE
    ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs} ${QRC_Srcs})
  qt5_use_modules(${TARGET_NAME} Core Gui Widgets)
  
  target_link_libraries(${TARGET_NAME}
    ${VTK_LIBRARIES}
    ${CURL_LIBRARY}
    ${jsoncpp_LIBRARIES}
  )
  
  target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
