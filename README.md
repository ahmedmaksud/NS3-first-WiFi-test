# WiFi Network Simulation with NS3-AI

This repository contains a WiFi network simulation example that demonstrates real-time communication between NS3 C++ simulation and Python analysis using the NS3-AI framework.

## Overview

This example simulates a WiFi network with:
- **8 mobile stations** moving in random patterns
- **1 access point** with adaptive transmission control
- **Real-time C++/Python communication** for network analysis
- **Adaptive algorithms** for transmission power optimization
- **Data export** for visualization and post-analysis

## Prerequisites

1. **NS3 Installation**: NS3.44 with NS3-AI module
2. **Python Environment**: Virtual environment with required packages
3. **Directory Structure**: Must be placed alongside NS3 installation

Expected directory structure:
```
NS3-project/
├── NS3AI-first-WiFi-test/     # This repository
└── ns-allinone-3.44/
    └── ns-3.44/               # NS3 installation
```

## Quick Start

```bash
# Clone this repository in the correct location
cd /path/to/NS3-project/
git clone <repository-url> NS3AI-first-WiFi-test
cd NS3AI-first-WiFi-test

# Run the complete simulation
./run.sh
```

The script will automatically:
- Deploy files to NS3 examples directory
- Configure and build the simulation  
- Execute the WiFi network simulation
- Optionally generate network visualizations

## Features

- **8 mobile WiFi stations** with random mobility
- **Real-time C++/Python communication** via NS3-AI
- **Adaptive transmission control** based on network performance
- **CSV data export** for analysis
- **Network topology visualization**

## Integration with NS3-AI

This example follows NS3-AI best practices:
- Uses standard `ns3ai_utils.Experiment` for communication
- Follows NS3 examples directory structure
- Integrates with NS3's build system
- Compatible with existing NS3-AI installations
