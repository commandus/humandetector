cmake_minimum_required(VERSION 3.9)
project(humandetector)
project(humandetector VERSION 1.0)

INCLUDE_DIRECTORIES("third_party")

set(HUMAN_DETECTOR_SRC 
	human-detector.cpp errlist.cpp daemonize.cpp utiltty.cpp
	strptime.cpp utildate.cpp
	third_party/argtable3/argtable3.c
)
add_executable(human-detector ${HUMAN_DETECTOR_SRC})
# target_link_libraries(human-detector ev)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cuman-detector DESTINATION cmake)
INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${loracli_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${loracli_VERSION_MINOR}")
include(CPack)