# WiFi Network Simulation with NS3-AI

**Author:** Ahmed Maksud (ahmed.maksud@email.ucr.edu)  
**PI:** Marcelo Menezes De Carvalho (mmcarvalho@txstate.edu)  
**Institution:** Texas State University

A WiFi network simulation example demonstrating real-time communication between NS3 C++ simulation and Python analysis using the NS3-AI framework.

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![NS3 Version](https://img.shields.io/badge/NS3-3.44-blue.svg)](https://www.nsnam.org/)
[![Python](https://img.shields.io/badge/Python-3.11-green.svg)](https://www.python.org/)

_For NS3-AI integration details, check [NS3-AI GitHub](https://github.com/hust-diangroup/ns3-ai)_

> **Important**: This project requires the [NS3-NS3AI installation repository](https://github.com/ahmedmaksud/NS3-NS3AI--installation-and-tests.git) to be completed first. Both projects must be located in the same NS3-project directory structure.

## Overview

This example simulates a WiFi network with:

- **8 mobile stations** moving in random patterns
- **1 access point** at the center with adaptive transmission control
- **Real-time C++/Python communication** for network analysis via shared memory
- **Adaptive algorithms** for transmission power optimization
- **Data export** for visualization and post-analysis (CSV format)
- **Network performance monitoring** with uplink/downlink throughput tracking

## Prerequisites

1. **NS3 Installation**: NS3.44 with NS3-AI module installed and configured
2. **Python Environment**: Virtual environment (e.g., `EHRL`, as specified in `venv_name.txt`) with required packages:
   - pandas, matplotlib, numpy, tqdm, ns3ai-utils (installed via NS3-AI setup)
3. **Directory Structure**: Must be placed alongside NS3 installation

Expected directory structure:

```
NS3-project/
├── NS3-first-WiFi-test/       # This repository (you are here)
├── <venv_name>/               # Python virtual environment (e.g., EHRL)
└── ns-allinone-3.44/
    └── ns-3.44/               # NS3 installation with AI module
```

## Quick Start

```bash
# Navigate to this directory
cd /path/to/NS3-project/NS3-first-WiFi-test

# Activate virtual environment (automatically done by run.sh)
# The venv name is read from venv_name.txt (e.g., EHRL)
source ../<venv_name>/bin/activate

# Run the complete simulation
./run.sh
```

The script will automatically:

1. **Deploy files** to NS3 examples directory (`contrib/ai/examples/wifi-simulation/`)
2. **Configure and build** the simulation using NS3's CMake system
3. **Execute** the WiFi network simulation with real-time C++/Python communication
4. **Generate data** (toy_data.csv with network performance metrics)
5. **Optionally create** network topology visualizations

## Features

### Core Simulation

- **8 mobile WiFi stations** with random walk mobility (0.05 m/s speed)
- **802.11n WiFi standard** with HtMcs1 data rate and HtMcs0 control rate
- **Access Point** at coordinates (0,0,0) with fixed position
- **Bidirectional traffic** (UDP): AP→STAs (downlink, disabled here) and STAs→AP (uplink)
- **Path loss models**: Log-distance + Nakagami fading

### Real-time AI Integration

- **Shared memory communication** between C++ simulation and Python analysis
- **250ms reporting interval** for real-time network monitoring
- **Adaptive transmission power control** based on network conditions
- **Distance-based performance analysis** with throughput optimization

### Data Collection & Analysis

- **Comprehensive metrics**: Position, distance, throughput, transmission power
- **~1,600 data points** collected over 50-second simulation (200 intervals × 8 stations)
- **CSV export format** for further analysis and visualization
- **Performance statistics**: Average DL/UL throughput, distance ranges
- **Real-time visualization** support with network topology animation

## Files Description

### Core Simulation Files

- **`wifi_network_simulation.cc`**: Main C++ NS3 simulation with 8 mobile stations
- **`wifi_data_structures.h`**: Shared data structures for C++/Python communication
- **`wifi_python_bindings.cc`**: Pybind11 bindings for NS3-AI interface

### Python Analysis Scripts

- **`wifi_analysis_and_control.py`**: Main Python script with adaptive control algorithms
- **`wifi_network_visualization.py`**: Network topology visualization and animation

### Build & Deployment

- **`CMakeLists.txt`**: NS3 build configuration with proper library linking
- **`run.sh`**: Automated deployment, build, and execution script

## Integration with NS3-AI

This example follows NS3-AI best practices:

- Uses standard `ns3ai_utils.Experiment` for communication management
- Follows NS3 examples directory structure (`contrib/ai/examples/`)
- Integrates with NS3's CMake build system
- Compatible with existing NS3-AI installations
- Proper shared memory synchronization with `EnvStruct`/`ActStruct`

## Output Files

After successful execution, the following files are generated:

### Data Files

- **`toy_data.csv`**: Complete dataset with columns:
  - `now_sec`: Simulation timestamp (seconds)
  - `sta_id`: Station identifier (0-7)
  - `pos_x`, `pos_y`: Station coordinates (meters)
  - `distance`: Distance from AP to station (meters)
  - `dl_tp`: Downlink throughput (Mbps)
  - `ul_tp`: Uplink throughput (Mbps)
  - `get_ApTx`: Current AP transmission power before adaptation (dBm)
  - `set_ApTx`: New AP transmission power after adaptive control (dBm)

### Visualization Files (Optional)

- **`sta_animation.gif`**: Network topology animation showing station movement

## Troubleshooting

### Common Issues

1. **Virtual Environment Not Found**

   ```bash
   # Replace <venv_name> with the name in venv_name.txt (e.g., EHRL)
   source ../<venv_name>/bin/activate
   pip install pandas matplotlib numpy ns3ai-utils
   ```

2. **NS3 Build Errors**

   ```bash
   cd ../ns-allinone-3.44/ns-3.44
   ./ns3 configure --enable-examples --enable-tests
   ./ns3 build
   ```

3. **Shared Memory Communication Errors**
   - Ensure both C++ and Python processes start correctly
   - Check that NS3-AI module is properly installed
   - Verify working directory is set to NS3 root

4. **Permission Issues**
   ```bash
   chmod +x run.sh
   ```

### Debug Mode

For debugging, run components separately:

```bash
# 1. Create destination and deploy files
mkdir -p ../ns-allinone-3.44/ns-3.44/contrib/ai/examples/wifi-simulation/
cp *.cc *.h *.py CMakeLists.txt ../ns-allinone-3.44/ns-3.44/contrib/ai/examples/wifi-simulation/

# 2. Update parent CMakeLists.txt (only needed once)
echo "add_subdirectory(wifi-simulation)" >> ../ns-allinone-3.44/ns-3.44/contrib/ai/examples/CMakeLists.txt

# 3. Build manually
cd ../ns-allinone-3.44/ns-3.44
./ns3 build ns3ai_wifi_simulation

# 4. Run C++ simulation only
./ns3 run ns3ai_wifi_simulation

# 5. Run Python analysis (in separate terminal)
python contrib/ai/examples/wifi-simulation/wifi_analysis_and_control.py
```

## Advanced Usage

### Customizing Simulation Parameters

Edit `wifi_network_simulation.cc`:

- `g_nStas`: Number of stations (default: 8)
- `g_totalTime`: Simulation duration (default: 50s)
- `g_interval`: Reporting interval (default: 0.25s)
- `g_init_distance`: Initial station placement radius (default: 1.5m)

### Modifying Adaptive Algorithms

Edit `wifi_analysis_and_control.py`:

- Transmission power control logic
- Throughput analysis algorithms
- Data processing and export formats

### Custom Visualizations

Use `wifi_network_visualization.py` as a starting point for:

- Real-time network topology plots
- Performance heatmaps
- Custom animation styles
