#include_directories(    ${CMAKE_SOURCE_DIR}/libkdeedu/libscience/   )

set(kalzium_plasma_SRCS
    kalzium_plasma.cpp
    )

qt5_wrap_ui(kalzium_plasma_SRCS config.ui)

add_plugin(plasma_applet_kalzium ${kalzium_plasma_SRCS})

target_link_libraries(plasma_applet_kalzium
    KF5::Plasma KF5::KIOWidgets
    science
    )

install(
    TARGETS plasma_applet_kalzium
    DESTINATION ${PLUGIN_INSTALL_DIR}
    )

install(
    FILES plasma_kalzium.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
    )

install(
    FILES testtube.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/
    )
