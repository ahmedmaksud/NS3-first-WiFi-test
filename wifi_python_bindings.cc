/*
 * Copyright (c) 2025 Texas State University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Ahmed Maksud <ahmed.maksud@email.ucr.edu>
 * PI: Marcelo Menezes De Carvalho <mmcarvalho@txstate.edu>
 * Texas State University
 */

/**
 * @file wifi_python_bindings.cc
 * @brief Python bindings for WiFi Network Simulation
 *
 * This file creates Python bindings using pybind11 to enable communication
 * between NS3 C++ WiFi simulation and Python analysis/visualization scripts.
 *
 * Key components:
 * - EnvStruct binding for receiving WiFi network data from C++
 * - ActStruct binding for sending control commands to C++
 * - Message interface for synchronized data exchange
 */

// Include the WiFi simulation data structures
#include "wifi_data_structures.h"

// Include NS3 AI module for message interface functionality
#include <ns3/ai-module.h>

// Standard library and pybind11 includes
#include <iostream>
#include <pybind11/pybind11.h>

// Create namespace alias for cleaner code
namespace py = pybind11;

/**
 * PYBIND11_MODULE: Creates a Python module named 'ns3ai_wifi_py'
 * This module will be compiled into a .so file that Python can import
 *
 * @param ns3ai_wifi_py: Name of the Python module (matches CMakeLists.txt)
 * @param m: Module object variable name used in the binding definitions
 */
PYBIND11_MODULE(ns3ai_wifi_py, m)
{
    /**
     * Bind the EnvStruct C++ class to Python as "PyEnvStruct"
     * This structure contains WiFi network data sent FROM C++ TO Python
     * - Position coordinates (pos_x, pos_y)
     * - Network performance metrics (dl_tp, ul_tp)
     * - AP transmission parameters (get_ApTx)
     * - Timing and identification data (now_sec, sta_id)
     */
    py::class_<EnvStruct>(m, "PyEnvStruct")
        .def(py::init<>())                                   // Default constructor
        .def_readwrite("pos_x", &EnvStruct::env_pos_x)       // STA X position in meters
        .def_readwrite("pos_y", &EnvStruct::env_pos_y)       // STA Y position in meters
        .def_readwrite("distance", &EnvStruct::env_distance) // Distance to AP in meters
        .def_readwrite("dl_tp", &EnvStruct::env_dl_tp)       // Downlink throughput (Mbps)
        .def_readwrite("ul_tp", &EnvStruct::env_ul_tp)       // Uplink throughput (Mbps)
        .def_readwrite("get_ApTx", &EnvStruct::env_get_ApTx) // Current AP Tx power/MCS
        .def_readwrite("sta_id", &EnvStruct::env_sta_id)     // Station identifier
        .def_readwrite("now_sec", &EnvStruct::env_now_sec);  // Current simulation time

    /**
     * Bind the ActStruct C++ class to Python as "PyActStruct"
     * This structure contains control commands sent FROM Python TO C++
     * - AP transmission parameter adjustments (set_ApTx)
     * - Used for adaptive algorithms and power control
     */
    py::class_<ActStruct>(m, "PyActStruct")
        .def(py::init<>())                                    // Default constructor
        .def_readwrite("set_ApTx", &ActStruct::env_set_ApTx); // New AP Tx power/MCS

    /**
     * Bind the NS3 AI Message Interface to Python
     * This is the core communication interface for WiFi simulation data exchange
     * Template parameters: <EnvStruct, ActStruct> specify the data structures used
     */
    py::class_<ns3::Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>>(m, "Ns3AiMsgInterfaceImpl")
        /**
         * Constructor binding with all parameters for shared memory communication:
         * @param bool: useVector - whether to use vector-based communication
         * @param bool: isMemoryCreator - whether this process creates shared memory
         * @param bool: handleFinish - whether to handle finish signals
         * @param uint32_t: memoryKey - unique key for shared memory segment
         * @param const char*: memPoolName - name of memory pool
         * @param const char*: cppAddr - C++ process address
         * @param const char*: pyAddr - Python process address
         * @param const char*: ctrlAddr - control address for synchronization
         */
        .def(py::init<bool,
                      bool,
                      bool,
                      uint32_t,
                      const char *,
                      const char *,
                      const char *,
                      const char *>())

        /**
         * Communication methods for WiFi data exchange:
         */

        /**
         * PyRecvBegin: Start receiving WiFi data from C++ simulation
         * Call this before reading network performance data
         * Returns: Pointer to EnvStruct containing current WiFi network state
         */
        .def("PyRecvBegin", &ns3::Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>::PyRecvBegin)

        /**
         * PyRecvEnd: End receiving WiFi data from C++ simulation
         * Call this after finishing reading from shared memory
         * Signals to C++ that Python has finished processing current data
         */
        .def("PyRecvEnd", &ns3::Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>::PyRecvEnd)

        /**
         * PySendBegin: Start sending control commands to C++ simulation
         * Call this before writing adaptive control parameters
         * Returns: Pointer to ActStruct for writing Python control decisions
         */
        .def("PySendBegin", &ns3::Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>::PySendBegin)

        /**
         * PySendEnd: End sending control commands to C++ simulation
         * Call this after finishing writing control parameters
         * Signals to C++ that new control data is ready for application
         */
        .def("PySendEnd", &ns3::Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>::PySendEnd)

        /**
         * PyGetFinished: Check if WiFi simulation has finished
         * Returns: bool indicating whether C++ simulation is complete
         */
        .def("PyGetFinished", &ns3::Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>::PyGetFinished)

        /**
         * GetCpp2PyStruct: Get direct access to WiFi simulation data
         * Returns: Reference to EnvStruct (WiFi data from C++ to Python)
         * return_value_policy::reference: Return by reference (no copy)
         */
        .def("GetCpp2PyStruct",
             &ns3::Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>::GetCpp2PyStruct,
             py::return_value_policy::reference)

        /**
         * GetPy2CppStruct: Get direct access to control command structure
         * Returns: Reference to ActStruct (control commands from Python to C++)
         * return_value_policy::reference: Return by reference (no copy)
         */
        .def("GetPy2CppStruct",
             &ns3::Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>::GetPy2CppStruct,
             py::return_value_policy::reference);
}
