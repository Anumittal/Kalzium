# Install script for directory: /home/anu/kalzium/doc

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "kalzium")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/HTML/en/kalzium" TYPE FILE FILES
    "/home/anu/kalzium/makefile2/doc//index.cache.bz2"
    "/home/anu/kalzium/doc/./index.docbook"
    "/home/anu/kalzium/doc/./man-kalzium.1.docbook"
    "/home/anu/kalzium/doc/./screenshot5.png"
    "/home/anu/kalzium/doc/./screenshot-mol-edit.png"
    "/home/anu/kalzium/doc/./quick-3.png"
    "/home/anu/kalzium/doc/./screenshot-calc-nucl.png"
    "/home/anu/kalzium/doc/./infodialog_data_overview.png"
    "/home/anu/kalzium/doc/./screenshot7.png"
    "/home/anu/kalzium/doc/./screenshot6.png"
    "/home/anu/kalzium/doc/./screenshot-calc-gas.png"
    "/home/anu/kalzium/doc/./infodialog_misc.png"
    "/home/anu/kalzium/doc/./infodialog_spectrum.png"
    "/home/anu/kalzium/doc/./screenshotnuclidboard.png"
    "/home/anu/kalzium/doc/./quick-1.png"
    "/home/anu/kalzium/doc/./infodialog_extra.png"
    "/home/anu/kalzium/doc/./settings2.png"
    "/home/anu/kalzium/doc/./settings4.png"
    "/home/anu/kalzium/doc/./screenshot-calc-mass.png"
    "/home/anu/kalzium/doc/./screenshot-tables.png"
    "/home/anu/kalzium/doc/./screenshot-calc-eq.png"
    "/home/anu/kalzium/doc/./screenshot1.png"
    "/home/anu/kalzium/doc/./screenshot-calc-titr.png"
    "/home/anu/kalzium/doc/./quick-4.png"
    "/home/anu/kalzium/doc/./settings1.png"
    "/home/anu/kalzium/doc/./settings3.png"
    "/home/anu/kalzium/doc/./screenshot2.png"
    "/home/anu/kalzium/doc/./sidebar1.png"
    "/home/anu/kalzium/doc/./gradient.png"
    "/home/anu/kalzium/doc/./screenshot-calc-conc.png"
    "/home/anu/kalzium/doc/./infodialog_bohr.png"
    "/home/anu/kalzium/doc/./infodialog_isotopes.png"
    "/home/anu/kalzium/doc/./screenshot-rs-phrases.png"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "kalzium")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "kalzium")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1" TYPE FILE FILES "/home/anu/kalzium/makefile2/doc//kalzium.1")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "kalzium")

