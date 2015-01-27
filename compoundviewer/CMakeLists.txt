include(${Avogadro_USE_FILE})

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/..
  ${EIGEN3_INCLUDE_DIR}
  ${OPENBABEL2_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/compoundviewer)

set( compoundviewer_SRCS
  openbabel2wrapper.cpp
  kalziumglwidget.cpp)

#add_subdirectory(widgets)

add_library(compoundviewer SHARED ${compoundviewer_SRCS})

target_link_libraries(compoundviewer
   ${OPENBABEL2_LIBRARIES}
   KF5::I18n
   KF5::WidgetsAddons
   KF5::CoreAddons
   Qt5::Core
)

set_target_properties(compoundviewer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS compoundviewer ${INSTALL_TARGETS_DEFAULT_ARGS})
