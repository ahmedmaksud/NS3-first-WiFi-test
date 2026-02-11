#!/usr/bin/env python3
# Copyright (c) 2025 Texas State University
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Ahmed Maksud <ahmed.maksud@email.ucr.edu>
# PI: Marcelo Menezes De Carvalho <mmcarvalho@txstate.edu>
# Texas State University

"""
WiFi Network Simulation - Python Analysis Script

This script demonstrates the Python side of NS3-WiFi-AI communication:
- Receives real-time WiFi network data from C++ NS3 simulation
- Processes network performance metrics and station positions
- Implements adaptive control algorithms (transmission power/MCS)
- Exports data to CSV for visualization and analysis
- Handles communication synchronization and error cases

Key Features:
- Real-time WiFi performance monitoring
- Adaptive transmission control based on network conditions
- Data collection for network topology analysis
- CSV export for post-simulation visualization
"""

# Data analysis and processing libraries
import pandas as pd
# Import the compiled Python binding module (created from wifi_python_bindings.cc)
import ns3ai_wifi_py as py_binding
# Standard library imports for system operations and error handling
import sys
import traceback
import os
# Import NS3-AI utilities for experiment management
from ns3ai_utils import Experiment

print("python: WiFi Network Simulation - Python Analysis Started")

# === FILE SYSTEM SETUP ===
"""
Setup data export path for simulation results:
- Get the current script directory for relative file paths
- Create CSV file path for exporting network performance data
- Ensures data is saved in the same directory as the script
"""
script_dir = os.path.dirname(os.path.abspath(__file__))
csv_path = os.path.join(script_dir, "toy_data.csv")

# === EXPERIMENT INITIALIZATION ===
"""
Create an Experiment object to manage NS3-Python WiFi simulation communication:
- "ns3ai_wifi_simulation": The name of our compiled WiFi simulation example
- ".": Working directory relative to examples directory
- py_binding: Our compiled Python binding module for WiFi data structures
- handleFinish=True: Automatically handle simulation finish signals
"""
exp = Experiment("ns3ai_wifi_simulation", "../../../../", py_binding, handleFinish=True)
print("python: Calling the NS3 WiFi simulation script")

# Start the NS3 WiFi simulation and get the message interface
msgInterface = exp.run(show_output=True)

# === DATA COLLECTION SETUP ===
"""
Initialize data structures for network performance analysis:
- data_store: List to accumulate all WiFi network measurements
- prev_now_sec: Track simulation time for change detection
- current_dl_values: Buffer for current downlink throughput values
- prev_mean_dl: Previous mean downlink throughput for comparison
"""
data_store = []              # Master data collection list
prev_now_sec = -1.0         # Previous simulation timestamp
current_dl_values = []      # Current downlink throughput buffer
prev_mean_dl = None         # Previous mean downlink for adaptive control

# === MAIN COMMUNICATION AND ANALYSIS LOOP ===
try:
    while True:
        print("python: Starting WiFi data reception...")
        
        # === RECEIVE PHASE: Get WiFi network data from C++ ===
        msgInterface.PyRecvBegin()  # Lock shared memory and wait for C++ data
        
        # Check if WiFi simulation has finished
        print("python: WiFi simulation status:", msgInterface.PyGetFinished())
        if msgInterface.PyGetFinished():
            break  # Exit loop when C++ simulation is complete
        
        # === READ WIFI NETWORK DATA ===
        """
        Extract current WiFi network state from shared memory:
        - Station position coordinates (pos_x, pos_y)
        - Network performance metrics (dl_tp, ul_tp)
        - Distance from station to access point
        - Current transmission parameters (get_ApTx)
        - Station ID and simulation timestamp
        """
        wifi_data = msgInterface.GetCpp2PyStruct()
        
        # Extract individual WiFi network parameters
        pos_x = wifi_data.pos_x          # Station X position (meters)
        pos_y = wifi_data.pos_y          # Station Y position (meters)
        distance = wifi_data.distance    # Distance to AP (meters)
        dl_tp = wifi_data.dl_tp         # Downlink throughput (Mbps)
        ul_tp = wifi_data.ul_tp         # Uplink throughput (Mbps)
        get_ApTx = wifi_data.get_ApTx   # Current AP transmission parameter
        sta_id = wifi_data.sta_id       # Station identifier
        now_sec = wifi_data.now_sec     # Current simulation time
        
        msgInterface.PyRecvEnd()    # Unlock shared memory, signal C++ we're done reading
        print("python: WiFi data received successfully.")

        # === DATA PROCESSING AND ANALYSIS ===
        """
        Process WiFi network data for analysis and control decisions:
        - Collect data points for CSV export
        - Calculate network performance statistics
        - Implement adaptive transmission control algorithms
        """
        
        # === ADAPTIVE CONTROL ALGORITHM ===
        """
        Implement adaptive transmission control based on network performance:
        - Monitor throughput changes over time periods
        - Calculate mean throughput for stability
        - Adjust AP transmission parameters for optimization
        - Example: reduce power when throughput is high (less interference)
        """
        
        # Initialize control parameter with current value
        set_ApTx = 20.0  # Default transmission power (dBm)
        
        # Time-based analysis for adaptive control
        if now_sec != prev_now_sec:
            # Calculate mean DL throughput for previous timestamp period
            if current_dl_values:
                prev_mean_dl = sum(current_dl_values) / len(current_dl_values)
                print(f"python: Mean DL @ {prev_now_sec:.2f}s: {prev_mean_dl:.2f} Mbps")
            
            # Reset for new timestamp period
            prev_now_sec = now_sec
            current_dl_values = []
        
        # Accumulate current measurement
        current_dl_values.append(dl_tp)
        
        # Adaptive transmission power control based on historical performance
        if prev_mean_dl is not None:
            # Example adaptive algorithm:
            # Higher throughput -> reduce power (less interference)
            # Lower throughput -> maintain/increase power
            set_ApTx = max(1.0, min(30.0, 30.0 - 30.0 * prev_mean_dl / 100.0))
            print(f"python: Adaptive control - ApTx set to: {set_ApTx:.2f} dBm")
        
        # === COMPREHENSIVE DATA LOGGING ===
        print(
            f"python: WiFi Status - "
            f"time={now_sec:.5f} "
            f"STA_ID={sta_id} "
            f"Position=({pos_x:.5f},{pos_y:.5f}) "
            f"Distance={distance:.5f}m "
            f"DL={dl_tp:.5f}Mbps "
            f"UL={ul_tp:.5f}Mbps "
            f"old_Tx={get_ApTx:.5f}dBm "
            f"new_Tx={set_ApTx:.5f}dBm"
        )

        # Store comprehensive WiFi measurement data for analysis
        data_point = {
            "pos_x": pos_x,
            "pos_y": pos_y,
            "distance": distance,
            "dl_tp": dl_tp,
            "ul_tp": ul_tp,
            "get_ApTx": get_ApTx,
            "sta_id": sta_id,
            "now_sec": now_sec,
            "set_ApTx": set_ApTx,
        }
        data_store.append(data_point)

        # === SEND PHASE: Return control commands to C++ ===
        print("python: Sending adaptive control commands...")
        msgInterface.PySendBegin()  # Lock shared memory for writing control commands
        
        # Write the calculated control parameters to shared memory
        msgInterface.GetPy2CppStruct().set_ApTx = set_ApTx
        
        msgInterface.PySendEnd()    # Unlock shared memory, signal C++ that commands are ready
        print("python: Control commands sent successfully.")

# === ERROR HANDLING ===
except Exception as e:
    """
    Comprehensive error handling for WiFi simulation debugging:
    - Catches any Python exceptions during communication or analysis
    - Prints detailed error information and stack trace
    - Ensures clean exit and data preservation on errors
    """
    exc_type, exc_value, exc_traceback = sys.exc_info()
    print("python: Exception occurred in WiFi simulation: {}".format(e))
    print("python: Traceback:")
    traceback.print_tb(exc_traceback)
    
    # Save collected data even if error occurs
    if data_store:
        print("python: Saving collected WiFi data before exit...")
        df = pd.DataFrame(data_store)
        df.to_csv(csv_path, index=False)
        print(f"python: WiFi data saved to {csv_path}")
    
    exit(1)

# === NORMAL COMPLETION ===
else:
    """
    Normal completion workflow:
    - Save all collected WiFi network data to CSV
    - Provide summary statistics
    - Prepare data for visualization
    """
    print("python: WiFi simulation completed successfully.")

# === CLEANUP AND DATA EXPORT ===
finally:
    """
    Cleanup code that always executes:
    - Runs whether the script exits normally or with an error
    - Ensures proper cleanup of experiment resources
    - Exports collected data to CSV for analysis
    - Provides final status and statistics
    """
    print("python: Cleaning up WiFi simulation resources...")
    
    # Export collected data to CSV for analysis and visualization
    if data_store:
        df = pd.DataFrame(data_store)
        df.to_csv(csv_path, index=False)
        print(f"python: WiFi network data exported to {csv_path}")
        print(f"python: Total data points collected: {len(data_store)}")
        
        # Provide summary statistics if data was collected
        if len(data_store) > 0:
            print(f"python: Simulation duration: {max(df['now_sec']) - min(df['now_sec']):.2f} seconds")
            print(f"python: Average throughput: DL={df['dl_tp'].mean():.2f} Mbps, UL={df['ul_tp'].mean():.2f} Mbps")
            print(f"python: Distance range: {df['distance'].min():.2f}m - {df['distance'].max():.2f}m")
    else:
        print("python: No data collected during simulation.")
    
    # Clean up experiment object and shared memory
    del exp
    print("python: WiFi Network Simulation - Python Analysis Completed")
