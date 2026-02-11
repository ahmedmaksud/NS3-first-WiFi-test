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
 * @file wifi_network_simulation.cc
 * @brief WiFi Network Simulation with NS3-AI Integration
 *
 * This simulation demonstrates NS3-Python communication using the AI module:
 * - Real-time C++ to Python communication via shared memory
 * - Adaptive WiFi parameter control based on AI/ML analysis
 * - Mobile station movement with performance monitoring
 * - Network topology analysis and visualization support
 *
 * Communication Flow:
 * 1. C++ simulation generates WiFi network data
 * 2. Data sent to Python via NS3-AI shared memory
 * 3. Python performs analysis and calculates adaptations
 * 4. Python sends control commands back to C++
 * 5. C++ applies adaptations to WiFi parameters
 */

// === NS3 CORE MODULES AND WIFI DATA STRUCTURES ===
#include "wifi_data_structures.h"  // WiFi data structures for C++/Python communication

// === NS3 SIMULATION FRAMEWORK ===
// Core NS3 modules for network simulation, mobility, and traffic
#include "ns3/applications-module.h"    // Traffic generation applications
#include "ns3/core-module.h"           // Core NS3 functionality and utilities
#include "ns3/internet-module.h"       // TCP/IP stack implementation
#include "ns3/mobility-module.h"       // Node movement and positioning
#include "ns3/network-module.h"        // Basic network components
#include "ns3/point-to-point-module.h" // Wired network connections
#include "ns3/wifi-module.h"           // WiFi/802.11 network simulation
#include "ns3/flow-monitor-module.h"   // Network flow monitoring

// === NS3-AI INTEGRATION ===
#include "ns3/ai-module.h"              // NS3-AI communication framework

// === STANDARD C++ LIBRARIES ===
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>

using namespace ns3;

// === SIMULATION CONFIGURATION PARAMETERS ===
/*
 * Global configuration for WiFi network simulation:
 * - Network topology: 1 AP + multiple mobile stations
 * - Movement pattern: Random waypoint mobility
 * - Traffic pattern: Bidirectional UDP traffic
 * - AI integration: Real-time parameter adaptation
 */
uint32_t g_nStas = 8;         // Number of station nodes (STAs) in WiFi network
double g_init_distance = 0.5; // Initial distance from AP to each STA (meters)
double g_totalTime = 50.0;    // Total simulation time (seconds)
double g_interval = 0.25;     // Reporting interval for Python communication (seconds)

// === NETWORK TOPOLOGY AND DEVICE CONTAINERS ===
/*
 * Core network components for WiFi simulation:
 * - Node containers for AP and stations
 * - Network device containers for WiFi interfaces
 * - Mobility models for node movement
 * - Traffic generation servers (UDP)
 */
std::vector<Ptr<UdpServer>> g_staServers;      // UDP servers on stations for uplink traffic
Ptr<UdpServer> g_apServer;                     // UDP server on AP for downlink traffic
Ptr<MobilityModel> g_apMobility;               // Mobility model for Access Point
std::vector<Ptr<MobilityModel>> g_staMobility; // Mobility models for all stations
std::vector<uint64_t> g_lastStaRx;             // Last received packet count per STA
uint64_t g_lastApRx = 0;                       // Last received packet count for AP
NodeContainer wifiApNode;                      // AP node container
NodeContainer wifiStaNodes;                    // Station node container
NetDeviceContainer apDevice;                   // AP network device
NetDeviceContainer staDevices;                 // Station network devices

// === NS3-AI COMMUNICATION INTERFACE ===
/*
 * Message interface for bidirectional C++/Python communication:
 * - EnvStruct: WiFi environment data sent to Python
 * - ActStruct: Control actions received from Python
 * - Real-time shared memory communication
 */
Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>* msgInterface; // AI message interface

// === NETWORK LAYER INTERFACES ===
/*
 * IPv4 interface containers for network layer connectivity:
 * - Separate interfaces for AP and stations
 * - Enables bidirectional traffic flow measurement
 */
Ipv4InterfaceContainer g_apIf;                             // IPv4 interface for AP
Ipv4InterfaceContainer g_staIf;                            // IPv4 interfaces for stations

// === PERFORMANCE MONITORING AND PHY LAYER ===
/*
 * Physical layer monitoring for performance analysis:
 * - Track packet drops for performance analysis
 * - PHY layer access for transmission parameter control
 * - Real-time throughput measurement
 */
std::vector<uint32_t> g_staPhyRxDrops;                     // Per-STA PHY RX drop counter
uint32_t g_apPhyRxDrops = 0;                               // AP PHY RX drop counter
std::vector<Ptr<YansWifiPhy>> g_staPhys;                   // PHY pointers for stations
Ptr<YansWifiPhy> g_apPhy;                                  // PHY pointer for AP

// Exchanges information with Python AI via the message interface and returns the new AP Tx power
double
LetsTalk(Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>* msgInterface,
         double pos_x,
         double pos_y,
         double distance,
         double dl_tp,
         double ul_tp,
         int get_ApTx,
         int sta_id,
         double now_sec)
{
    std::cout << "C++;LetsTalk: Starting sending msg.\n";
    msgInterface->CppSendBegin();

    // Populate the shared struct with environment information
    EnvStruct* env = msgInterface->GetCpp2PyStruct();
    env->env_pos_x = pos_x;
    env->env_pos_y = pos_y;
    env->env_distance = distance;
    env->env_dl_tp = dl_tp;
    env->env_ul_tp = ul_tp;
    env->env_get_ApTx = get_ApTx;
    env->env_sta_id = sta_id;
    env->env_now_sec = now_sec;

    msgInterface->CppSendEnd();
    std::cout << "C++;LetsTalk: Stopped sending msg.\n";

    // Wait for and receive the result from Python
    std::cout << "C++;LetsTalk: Starting receiving msg.\n";
    msgInterface->CppRecvBegin();
    std::cout << "C++;LetsTalk: Started receiving msg.\n";

    // Retrieve the output value set by Python
    std::cout << "C++;LetsTalk: Got msg.\n";
    double py_output = msgInterface->GetPy2CppStruct()->env_set_ApTx;

    msgInterface->CppRecvEnd();
    std::cout << "C++;LetsTalk: End receiving msg.\n";

    return py_output;
}

// Initializes the AI message interface for communication with Python
Ns3AiMsgInterfaceImpl<EnvStruct, ActStruct>*
InitializeNs3AiInterface()
{
    std::cout << "C++;InitializeNs3AiInterface: Initializing the interface.\n";
    auto interface = Ns3AiMsgInterface::Get();
    interface->SetIsMemoryCreator(false); // This process does not create shared memory
    interface->SetUseVector(false);       // Not using vectorized communication
    interface->SetHandleFinish(true);     // Handle finish signal
    std::cout << "C++;InitializeNs3AiInterface: The interface has been initialized.\n";
    return interface->GetInterface<EnvStruct, ActStruct>();
}

// Reports throughput, distance, and energy for each STA and AP, and interacts with AI for AP Tx
// power
void
GetReport(Time interval)
{
    if (!g_apPhy)
    {
        std::cerr << "AP PHY pointer is null at report time!" << std::endl;
    }
    if (apDevice.GetN() == 0)
    {
        std::cerr << "apDevice is empty!" << std::endl;
    }

    // Get AP device, SSID, and position
    Ptr<WifiNetDevice> apDev = DynamicCast<WifiNetDevice>(apDevice.Get(0));
    Ssid apSsid = apDev->GetMac()->GetSsid();
    Vector apPos = g_apMobility->GetPosition();

    // Retrieve current AP Tx power
    double old_txPower = 20.0;
    double new_txPower = 20.0;
    if (g_apPhy)
    {
        old_txPower = g_apPhy->GetTxPowerStart();
        new_txPower = old_txPower;
    }
    else
    {
        std::cerr << "AP PHY pointer is null at report time!\n";
    }

    // Get current simulation time
    Time now = Simulator::Now();
    double nowSeconds = now.GetSeconds();

    // Print AP information and simulation time
    std::cout << "\n=== Report @ " << nowSeconds << "s ==="
              << "\nAP SSID: " << apSsid << "\nAP Position: (" << apPos.x << ", " << apPos.y
              << ")\n";

    // Print total uplink throughput for AP
    uint64_t curApRx = g_apServer->GetReceived();
    double ulThroughput = (curApRx - g_lastApRx) * 1472 * 8.0 / 1e6;
    g_lastApRx = curApRx;
    std::cout << "Total UL Throughput: " << ulThroughput << "Mbps\n";

    // For each STA, print position, distance to AP, downlink throughput, and energy info
    for (uint32_t i = 0; i < g_staServers.size(); ++i)
    {
        Ptr<WifiNetDevice> staDev = DynamicCast<WifiNetDevice>(staDevices.Get(i));
        Ptr<YansWifiPhy> staPhy = DynamicCast<YansWifiPhy>(staDev->GetPhy());
        Ipv4Address staIp = g_staIf.GetAddress(i);

        uint64_t curStaRx = g_staServers[i]->GetReceived();
        double dlThroughput = (curStaRx - g_lastStaRx[i]) * 1472 * 8.0 / 1e6; // Mbps
        g_lastStaRx[i] = curStaRx;

        Vector staPos = g_staMobility[i]->GetPosition();
        double distance = g_apMobility->GetDistanceFrom(g_staMobility[i]);

        // Print STA information
        std::cout << "STA[" << i << "]"
                  << " IP: " << staIp << " Position: (" << staPos.x << ", " << staPos.y << ")"
                  << " Distance: " << distance << "m"
                  << "\n  DL: " << dlThroughput << "Mbps\n";

        // Interact with AI (Python) for new AP Tx power
        new_txPower = LetsTalk(msgInterface,
                               staPos.x,
                               staPos.y,
                               distance,
                               dlThroughput,
                               ulThroughput,
                               old_txPower,
                               i,
                               nowSeconds);
        std::cout << "C++;GetReport: Python Response TX: " << new_txPower << "\n";

        // Display station performance metrics
        std::cout << "   [Station " << i << "] "
                  << "Position: (" << staPos.x << ", " << staPos.y << "), "
                  << "Distance: " << distance << "m, "
                  << "DL: " << dlThroughput << "Mbps, "
                  << "UL: " << ulThroughput << "Mbps\n";
    }

    // Set new AP Tx power using the global pointer (only once per report)
    if (g_apPhy)
    {
        g_apPhy->SetTxPowerStart(new_txPower);
        g_apPhy->SetTxPowerEnd(new_txPower);
    }
    else
    {
        std::cerr << "AP PHY is null, cannot set Tx power!\n";
    }

    // Schedule the next report if simulation time not exceeded
    if (Simulator::Now().GetSeconds() + interval.GetSeconds() <= g_totalTime)
    {
        Simulator::Schedule(interval, &GetReport, interval);
    }
}

// Sets up the WiFi scenario: nodes, devices, mobility, IP, UDP apps
void
InitializeScenario()
{
    using namespace ns3::energy;
    std::cout << "C++;InitializeScenario: Initializing the scenario.\n";

    // Create AP and STA nodes
    std::cout << "C++;InitializeScenario: Creating AP and STA nodes.\n";
    wifiApNode.Create(1);
    wifiStaNodes.Create(g_nStas);

    // Set up WiFi channel and PHY layer with 1 antenna and 1 spatial stream
    std::cout << "C++;InitializeScenario: Setting up WiFi channel and PHY layer.\n";
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                               "Exponent",
                               DoubleValue(3.0),
                               "ReferenceLoss",
                               DoubleValue(40.0459));
    channel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
                               "m0",
                               DoubleValue(1.0),
                               "m1",
                               DoubleValue(1.0),
                               "m2",
                               DoubleValue(1.0));
    phy.SetChannel(channel.Create());
    phy.Set("Antennas", UintegerValue(1));
    phy.Set("MaxSupportedTxSpatialStreams", UintegerValue(1));
    phy.Set("MaxSupportedRxSpatialStreams", UintegerValue(1));

    // Configure MAC and WiFi standard (802.11n), and set rate control
    std::cout << "C++;InitializeScenario: Configuring MAC and WiFi standard.\n";
    WifiMacHelper mac;
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211n);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("HtMcs1"),
                                 "ControlMode",
                                 StringValue("HtMcs0"));
    Ssid ssid = Ssid("ns3-80211n-mimo");

    // Install STA devices with specified SSID and disable active probing
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, wifiStaNodes);
    // when you do something difficult, the labour passes quickly but the pride endures
    // when you do something shameful for pleasure, the pleasure passes quickly but the shame
    // endures

    // Install AP device with specified SSID
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevice = wifi.Install(phy, mac, wifiApNode);

    // Set global AP PHY pointer for later use
    Ptr<WifiNetDevice> apDev = DynamicCast<WifiNetDevice>(apDevice.Get(0));
    g_apPhy = DynamicCast<YansWifiPhy>(apDev->GetPhy());

    // Set up mobility for AP: fixed position at (0,0,0)
    std::cout << "C++;InitializeScenario: Setting up mobility for AP and STAs.\n";
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(wifiApNode);

    // Set up mobility for STAs: random walk within a rectangle, slow speed
    MobilityHelper staMobility;
    Ptr<ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator>();
    for (uint32_t i = 0; i < g_nStas; ++i)
    {
        double angle = (2 * M_PI * i) / g_nStas;
        double x = g_init_distance * cos(angle);
        double y = g_init_distance * sin(angle);
        staPositionAlloc->Add(Vector(x, y, 0.0));
    }
    staMobility.SetPositionAllocator(staPositionAlloc);
    staMobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                 "Bounds",
                                 RectangleValue(Rectangle(-50, 50, -50, 50)),
                                 "Speed",
                                 StringValue("ns3::ConstantRandomVariable[Constant=0.05]"));
    staMobility.Install(wifiStaNodes);

    // Store references to mobility models for AP and STAs
    g_apMobility = wifiApNode.Get(0)->GetObject<MobilityModel>();
    g_staMobility.clear();
    for (uint32_t i = 0; i < g_nStas; ++i)
    {
        g_staMobility.push_back(wifiStaNodes.Get(i)->GetObject<MobilityModel>());
    }

    // Install Internet stack (TCP/IP) on all nodes
    std::cout << "C++;InitializeScenario: Installing Internet stack on nodes.\n";
    InternetStackHelper stack;
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);

    // Assign IP addresses to AP and STA devices
    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer apIf = address.Assign(apDevice);
    Ipv4InterfaceContainer staIf = address.Assign(staDevices);

    g_apIf = apIf;
    g_staIf = staIf;

    // Set up UDP servers on each STA and the AP, and track received packets
    std::cout << "C++;InitializeScenario: Setting up UDP servers on STAs and AP.\n";
    uint16_t port = 9;
    g_staServers.clear();
    g_lastStaRx.clear();
    for (uint32_t i = 0; i < g_nStas; ++i)
    {
        UdpServerHelper staServer(port);
        ApplicationContainer staServerApp = staServer.Install(wifiStaNodes.Get(i));
        staServerApp.Start(Seconds(0.0));
        staServerApp.Stop(Seconds(g_totalTime));
        g_staServers.push_back(DynamicCast<UdpServer>(staServerApp.Get(0)));
        g_lastStaRx.push_back(0);
    }
    UdpServerHelper apServer(port);
    ApplicationContainer apServerApp = apServer.Install(wifiApNode.Get(0));
    apServerApp.Start(Seconds(0.0));
    apServerApp.Stop(Seconds(g_totalTime));
    g_apServer = DynamicCast<UdpServer>(apServerApp.Get(0));

    // Set up UDP clients for both downlink (AP→STA) and uplink (STA→AP)
    ApplicationContainer apToStaApps;
    ApplicationContainer staToApApps;

    for (uint32_t i = 0; i < g_nStas; ++i)
    {
        // Downlink: AP sends to STA[i]
        UdpClientHelper apToStaClient(staIf.GetAddress(i), port);
        apToStaClient.SetAttribute("MaxPackets", UintegerValue(4294967295U));
        apToStaClient.SetAttribute("Interval", TimeValue(MilliSeconds(1)));
        apToStaClient.SetAttribute("PacketSize", UintegerValue(1472));
        apToStaApps.Add(apToStaClient.Install(wifiApNode.Get(0)));

        // Uplink: STA[i] sends to AP
        UdpClientHelper staToApClient(apIf.GetAddress(0), port);
        staToApClient.SetAttribute("MaxPackets", UintegerValue(4294967295U));
        staToApClient.SetAttribute("Interval", TimeValue(MilliSeconds(1)));
        staToApClient.SetAttribute("PacketSize", UintegerValue(1472));
        staToApApps.Add(staToApClient.Install(wifiStaNodes.Get(i)));
    }
    // Start and stop UDP client applications at the correct times
    apToStaApps.Start(Seconds(g_interval));
    apToStaApps.Stop(Seconds(g_totalTime));
    staToApApps.Start(Seconds(g_interval));
    staToApApps.Stop(Seconds(g_totalTime));

    // Store PHY pointers for each STA for later use
    for (uint32_t i = 0; i < g_nStas; ++i)
    {
        Ptr<WifiNetDevice> staDev = DynamicCast<WifiNetDevice>(staDevices.Get(i));
        g_staPhys.push_back(DynamicCast<YansWifiPhy>(staDev->GetPhy()));
    }

    std::cout << "C++;InitializeScenario: Scenario initialized successfully.\n";
}

// Main function: entry point for the simulation
int
main(int argc, char* argv[])
{
    // Initialize the AI message interface for communication with Python
    msgInterface = InitializeNs3AiInterface();

    // Set up the WiFi scenario (nodes, devices, mobility, IP, UDP apps, etc.)
    InitializeScenario();

    // Schedule periodic reporting of throughput and distance
    Simulator::Schedule(Seconds(g_interval), &GetReport, Seconds(g_interval));

    // Set simulation stop time and run the simulation
    Simulator::Stop(Seconds(g_totalTime));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}