# Install script for directory: /home/anu/kalzium/data/htmlview

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
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/kalzium/data/htmlview" TYPE FILE FILES
    "/home/anu/kalzium/data/htmlview/radius.png"
    "/home/anu/kalzium/data/htmlview/structure.png"
    "/home/anu/kalzium/data/htmlview/ionization.png"
    "/home/anu/kalzium/data/htmlview/abundance.png"
    "/home/anu/kalzium/data/htmlview/book.png"
    "/home/anu/kalzium/data/htmlview/electronaffinity.png"
    "/home/anu/kalzium/data/htmlview/meltingpoint.png"
    "/home/anu/kalzium/data/htmlview/mass.png"
    "/home/anu/kalzium/data/htmlview/boilingpoint.png"
    "/home/anu/kalzium/data/htmlview/header.png"
    "/home/anu/kalzium/data/htmlview/characteristics.png"
    "/home/anu/kalzium/data/htmlview/discovery.png"
    "/home/anu/kalzium/data/htmlview/icons.svg"
    "/home/anu/kalzium/data/htmlview/style.css"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "kalzium")

