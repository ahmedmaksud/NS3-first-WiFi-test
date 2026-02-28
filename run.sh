#!/bin/bash
#
# Author: Ahmed Maksud; email: ahmed.maksud@email.ucr.edu
# PI: Marcelo Menezes De Carvalho; email: mmcarvalho@txstate.edu
# Texas State University
#
# WiFi Network Simulation Example - Single Run Script
# ===================================================
# This script automates the complete workflow for running the WiFi NS3-AI example:
# 1. Deploy source files to NS3 examples directory
# 2. Setup Python virtual environment from previous installation
# 3. Configure and build NS3 with the WiFi example
# 4. Execute the Python script to start the WiFi simulation
#
# Usage: ./run.sh (must be run from NS3-first-WiFi-test directory)

set -e # Exit immediately if any command fails

echo "============================================"
echo "WiFi Network Simulation Example - Single Run"
echo "============================================"

# === DIRECTORY VALIDATION ===
# Ensure script is run from the correct directory (NS3-first-WiFi-test or NS3AI-first-WiFi-test)
CURRENT_DIR="$(basename $(pwd))"
if [[ ! "$CURRENT_DIR" == "NS3-first-WiFi-test" && ! "$CURRENT_DIR" == "NS3AI-first-WiFi-test" ]]; then
    echo "Error: Please run this script from the NS3-first-WiFi-test directory"
    echo "Current directory: $(pwd)"
    echo "Expected to be in NS3-first-WiFi-test or NS3AI-first-WiFi-test directory"
    echo "Usage: cd /path/to/NS3-project/NS3-first-WiFi-test && ./run.sh"
    exit 1
fi

# Verify NS3 installation exists in expected location
if [[ ! -d "../ns-allinone-3.44/ns-3.44" ]]; then
    echo "Error: NS3 installation not found at ../ns-allinone-3.44/ns-3.44"
    echo "Please ensure you're in the correct directory structure:"
    echo "NS3-project/"
    echo "├── NS3-first-WiFi-test/       (you are here)"
    echo "└── ns-allinone-3.44/"
    echo "    └── ns-3.44/"
    exit 1
fi

# === STEP 1: DEPLOY SOURCE FILES ===
echo ""
echo "Step 1: Deploying WiFi simulation example..."
echo "============================================="

# Define source and destination directories
REPO_DIR="."                                              # Current directory
NS3_DIR="../ns-allinone-3.44/ns-3.44/contrib/ai/examples" # NS3 examples location
DEST_DIR="$NS3_DIR/wifi-simulation"                       # Our WiFi example destination

# Create destination directory if it doesn't exist
echo "Creating destination directory: $DEST_DIR"
mkdir -p "$DEST_DIR"

# Copy all files from current directory except run.sh and .git to wifi-simulation
echo "Copying WiFi simulation files..."
for file in *; do
    if [ "$file" != "run.sh" ] && [ "$file" != ".git" ] && [ -f "$file" ]; then
        cp "$file" "$DEST_DIR/"
        echo "  Copied: $file"
    fi
done

# Update the parent CMakeLists.txt to include our subdirectory
CMAKE_FILE="$NS3_DIR/CMakeLists.txt"
echo "Updating parent CMakeLists.txt to include wifi-simulation subdirectory..."

# Check if our subdirectory is already added
if ! grep -q "add_subdirectory(wifi-simulation)" "$CMAKE_FILE"; then
    # Add our subdirectory to the parent CMakeLists.txt
    echo "" >>"$CMAKE_FILE"
    echo "# WiFi Network Simulation example" >>"$CMAKE_FILE"
    echo "add_subdirectory(wifi-simulation)" >>"$CMAKE_FILE"
    echo "Parent CMakeLists.txt updated successfully!"
else
    echo "wifi-simulation subdirectory already added to parent CMakeLists.txt."
fi

echo "WiFi simulation deployed successfully!"

echo "Deployment completed!"

# === STEP 2: SETUP ENVIRONMENT AND BUILD NS3 ===
echo ""
echo "Step 2: Setting up environment and building ns3..."
echo "=================================================="

# Read the virtual environment name from the previous NS3-NS3AI installation
if [[ -f "../NS3-NS3AI--installation-and-tests/venv_name.txt" ]]; then
    VENV_NAME=$(cat ../NS3-NS3AI--installation-and-tests/venv_name.txt)
    echo "Using virtual environment: $VENV_NAME"

    # Verify the virtual environment exists
    if [ ! -d "../$VENV_NAME" ]; then
        echo "Error: Virtual environment '$VENV_NAME' not found at ../$VENV_NAME"
        echo "Please ensure the previous NS3-NS3AI installation is complete."
        exit 1
    fi
else
    # Use default environment name if file not found
    VENV_NAME="EHRL"
    echo "Using default virtual environment: $VENV_NAME"

    if [ ! -d "../$VENV_NAME" ]; then
        echo "Error: Virtual environment '$VENV_NAME' not found at ../$VENV_NAME"
        echo "Please run the NS3-NS3AI installation first or create the environment manually."
        exit 1
    fi
fi

# Activate the existing virtual environment (contains all required Python packages)
echo "Activating virtual environment..."
source ../$VENV_NAME/bin/activate

# Install additional packages for WiFi simulation if needed
echo "Installing/updating packages for WiFi simulation..."
pip install pandas matplotlib tqdm numpy

# Navigate to NS3 build directory
echo "Navigating to ns3.44 directory..."
cd ../ns-allinone-3.44/ns-3.44

# Configure NS3 with examples and tests enabled
echo "Configuring ns3..."
./ns3 configure --enable-examples --enable-tests

# Build the entire NS3 project including our WiFi example
echo "Building ns3 and the WiFi simulation example..."
./ns3 build

# Specifically build our WiFi example to ensure it's compiled
echo "Building ns3ai_wifi_simulation..."
./ns3 build ns3ai_wifi_simulation

# Check if build was successful
if [ $? -ne 0 ]; then
    echo "Build failed. Please check the error messages."
    exit 1
fi

echo "Build successful!"

# === STEP 3: EXECUTE THE WIFI SIMULATION ===
echo ""
echo "Step 3: Running the WiFi simulation..."
echo "====================================="

# Navigate to the deployed WiFi example directory
EXAMPLE_DIR="contrib/ai/examples/wifi-simulation"

# Verify the example was deployed correctly
if [ -d "$EXAMPLE_DIR" ] && [ -f "$EXAMPLE_DIR/wifi_analysis_and_control.py" ]; then
    echo "Changing to WiFi simulation example directory..."
    cd "$EXAMPLE_DIR"

    # Execute the Python script from the example directory
    # This will:
    # 1. Start the NS3 WiFi C++ simulation in the background
    # 2. Establish shared memory communication for real-time data exchange
    # 3. Process WiFi network performance data
    # 4. Apply adaptive control algorithms
    # 5. Export data for visualization and analysis
    echo "Starting WiFi simulation..."
    python3 wifi_analysis_and_control.py

    # Check if simulation completed and data was generated
    if [ -f "toy_data.csv" ]; then
        echo ""
        echo "WiFi simulation completed successfully!"
        echo "Data exported to toy_data.csv"
        echo "Data points collected: $(wc -l <toy_data.csv)"

        # Optional: Run visualization if requested
        echo ""
        echo "Would you like to generate WiFi network topology animation? (y/n)"
        read -r response
        if [[ "$response" =~ ^[Yy]$ ]]; then
            echo "Generating WiFi network visualization..."
            python3 wifi_network_visualization.py
            if [ -f "sta_animation.gif" ]; then
                echo "Animation saved as sta_animation.gif"
            fi
        fi
    else
        echo "Warning: No simulation data file found. Check simulation logs for issues."
    fi

    # Return to NS3 root directory
    cd ../../../..
else
    echo "WiFi simulation example not found at $EXAMPLE_DIR"
    echo "Please ensure the WiFi files are properly deployed."
    exit 1
fi

echo ""
echo "============================================"
echo "WiFi Network Simulation Example - Completed!"
echo "============================================"
echo ""
echo "Summary:"
echo "- WiFi network simulation executed successfully"
echo "- Real-time C++/Python communication demonstrated"
echo "- 8 mobile WiFi stations simulated with adaptive control"
echo "- Network performance data collected and analyzed"
echo "- Data exported for visualization and further analysis"
echo ""
echo "Files generated in contrib/ai/examples/wifi-simulation/:"
echo "- toy_data.csv: Complete WiFi network performance dataset"
echo "- sta_animation.gif: Network topology animation (if generated)"
echo ""
echo "Next steps:"
echo "- Analyze toy_data.csv for network performance insights"
echo "- Experiment with different adaptive control algorithms"
echo "- Modify simulation parameters in wifi_network_simulation.cc"
echo "- Use wifi_network_visualization.py for custom visualizations"
