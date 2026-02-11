# CMakeLists.txt for WiFi Network Simulation with NS3-AI Integration
#
# Copyright (c) 2025 Texas State University
#
# Author: Ahmed Maksud <ahmed.maksud@email.ucr.edu>
# PI: Marcelo Menezes De Carvalho <mmcarvalho@txstate.edu>
# Texas State University
#
# This CMake configuration builds:
# 1. C++ WiFi simulation executable with NS3-AI communication
# 2. Python binding library for real-time data exchange
# 3. Complete WiFi network simulation with adaptive control
#
# The simulation demonstrates:
# - Real-time C++ to Python communication via shared memory
# - Adaptive WiFi parameter control based on AI/ML analysis
# - Mobile station movement with performance monitoring
# - Network topology analysis and visualization support

# Build the main WiFi simulation C++ executable
build_lib_example(
    NAME ns3ai_wifi_simulation
    SOURCE_FILES wifi_network_simulation.cc
    LIBRARIES_TO_LINK ${libai}
                      ${libcore}
                      ${libnetwork}
                      ${libwifi}
                      ${libmobility}
                      ${libinternet}
                      ${libapplications}
                      ${libflow-monitor}
                      ${libpoint-to-point}
)

# Build the Python binding library for WiFi data structures
pybind11_add_module(ns3ai_wifi_py wifi_python_bindings.cc)
set_target_properties(ns3ai_wifi_py PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Ensure Python binding is built along with C++ simulation
add_dependencies(ns3ai_wifi_simulation ns3ai_wifi_py)
