# Install script for directory: /home/anu/kalzium/data/toolpics

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
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/kalzium/data/toolpics" TYPE FILE FILES
    "/home/anu/kalzium/data/toolpics/abzug.jpg"
    "/home/anu/kalzium/data/toolpics/becherglas.jpg"
    "/home/anu/kalzium/data/toolpics/brechungsmesser.jpg"
    "/home/anu/kalzium/data/toolpics/brenner.jpg"
    "/home/anu/kalzium/data/toolpics/destillierbruecke.jpg"
    "/home/anu/kalzium/data/toolpics/dsc.jpg"
    "/home/anu/kalzium/data/toolpics/erlenmeyerkolben.jpg"
    "/home/anu/kalzium/data/toolpics/exikator.jpg"
    "/home/anu/kalzium/data/toolpics/halter.jpg"
    "/home/anu/kalzium/data/toolpics/heizplatte.jpg"
    "/home/anu/kalzium/data/toolpics/hplc.jpg"
    "/home/anu/kalzium/data/toolpics/kolbenprober.png"
    "/home/anu/kalzium/data/toolpics/korkring.jpg"
    "/home/anu/kalzium/data/toolpics/messzylinder.jpg"
    "/home/anu/kalzium/data/toolpics/moerser.jpg"
    "/home/anu/kalzium/data/toolpics/phpapier.jpg"
    "/home/anu/kalzium/data/toolpics/pileusball.jpg"
    "/home/anu/kalzium/data/toolpics/pipette.jpg"
    "/home/anu/kalzium/data/toolpics/rg-halter.jpg"
    "/home/anu/kalzium/data/toolpics/rg.jpg"
    "/home/anu/kalzium/data/toolpics/rg-staender.jpg"
    "/home/anu/kalzium/data/toolpics/rotationsverdampfer.jpg"
    "/home/anu/kalzium/data/toolpics/rueckflusskuehler.jpg"
    "/home/anu/kalzium/data/toolpics/ruehrfisch.jpg"
    "/home/anu/kalzium/data/toolpics/rundkolben.jpg"
    "/home/anu/kalzium/data/toolpics/scheidetrichter.jpg"
    "/home/anu/kalzium/data/toolpics/schutzbrille.jpg"
    "/home/anu/kalzium/data/toolpics/spatel.jpg"
    "/home/anu/kalzium/data/toolpics/spritzflasche.jpg"
    "/home/anu/kalzium/data/toolpics/thermometer2.jpg"
    "/home/anu/kalzium/data/toolpics/thermometer.jpg"
    "/home/anu/kalzium/data/toolpics/tonschale.jpg"
    "/home/anu/kalzium/data/toolpics/trichter.jpg"
    "/home/anu/kalzium/data/toolpics/trockenrohr.jpg"
    "/home/anu/kalzium/data/toolpics/tropftrichter.jpg"
    "/home/anu/kalzium/data/toolpics/uhrglas.jpg"
    "/home/anu/kalzium/data/toolpics/verteiler.jpg"
    "/home/anu/kalzium/data/toolpics/vollpipette.jpg"
    "/home/anu/kalzium/data/toolpics/waage.jpg"
    "/home/anu/kalzium/data/toolpics/wasserstrahlpumpe.jpg"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "kalzium")

