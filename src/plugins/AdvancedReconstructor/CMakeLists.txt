cmake_minimum_required(VERSION 3.15)

project(AdvancedReconstructor)

# Plugin source files
set(PLUGIN_SOURCES
    plugin.cpp
    PackDetector.cpp
    RuntimeInstrumentor.cpp
    OEPFinder.cpp
    Dumper.cpp
    MultiLayerController.cpp
)

# Plugin header files
set(PLUGIN_HEADERS
    plugin.h
    PackDetector.h
    RuntimeInstrumentor.h
    OEPFinder.h
    Dumper.h
    MultiLayerController.h
)

# Create the plugin DLL
add_library(AdvancedReconstructor SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

# Include directories
target_include_directories(AdvancedReconstructor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/dbg
    ${CMAKE_SOURCE_DIR}/src/bridge
)

# Set output name
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set_target_properties(AdvancedReconstructor PROPERTIES
        OUTPUT_NAME "x32_AdvancedReconstructor"
    )
else()
    set_target_properties(AdvancedReconstructor PROPERTIES
        OUTPUT_NAME "x64_AdvancedReconstructor"
    )
endif()

# Link libraries
target_link_libraries(AdvancedReconstructor PRIVATE
    
)

# Install to bin/plugins directory
install(TARGETS AdvancedReconstructor
    RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin/plugins
    LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/bin/plugins
)
