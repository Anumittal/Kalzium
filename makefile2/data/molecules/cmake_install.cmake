# Install script for directory: /home/anu/kalzium/data/molecules

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
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/kalzium/data/molecules" TYPE FILE FILES
    "/home/anu/kalzium/data/molecules/propan-1-ol.cml"
    "/home/anu/kalzium/data/molecules/carbon_dioxide.cml"
    "/home/anu/kalzium/data/molecules/butane.cml"
    "/home/anu/kalzium/data/molecules/methane.cml"
    "/home/anu/kalzium/data/molecules/propan-2-ol.cml"
    "/home/anu/kalzium/data/molecules/ethanol.cml"
    "/home/anu/kalzium/data/molecules/2_2_2-trifluoroethanol.cml"
    "/home/anu/kalzium/data/molecules/ethane.cml"
    "/home/anu/kalzium/data/molecules/2-thioethanol.cml"
    "/home/anu/kalzium/data/molecules/thiophene.cml"
    "/home/anu/kalzium/data/molecules/but-2-yne-1_4-diol.cml"
    "/home/anu/kalzium/data/molecules/water.cml"
    "/home/anu/kalzium/data/molecules/methanol.cml"
    "/home/anu/kalzium/data/molecules/2-aminoethanol.cml"
    "/home/anu/kalzium/data/molecules/carbon_monoxide.cml"
    "/home/anu/kalzium/data/molecules/propane.cml"
    "/home/anu/kalzium/data/molecules/porphyrin.cml"
    "/home/anu/kalzium/data/molecules/ethane-1_2-diol.cml"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "kalzium")

