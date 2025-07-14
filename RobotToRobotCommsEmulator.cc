#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>   // For std::system()
#include <mutex>
#include <gz/msgs.hh>
#include <gz/transport.hh>

// Global variables for current network settings.
double currentDrop = 0.0;        // Loss percentage.
double currentBandwidth = 0.0;   // Bandwidth in Mbps.
double currentDelay = 0.0;       // Delay in ms.

// Global variables for the virtual ethernet interfaces.
// These will be populated from the command-line arguments.
std::string veth1;
std::string veth2;

// Mutex to protect the global variables.
std::mutex netMutex;

// Function to update the network settings by executing a combined command.
void updateNetwork()
{
  // Lock the mutex to safely read the current values.
  std::lock_guard<std::mutex> lock(netMutex);
  
  // Ensure the veth interface names have been set before proceeding.
  if (veth1.empty() || veth2.empty())
  {
      std::cerr << "[updateNetwork] Error: Virtual ethernet interface names are not set. "
                << "Cannot execute command." << std::endl;
      return;
  }

  // Build the command string using the globally stored interface names.
  // This command updates the two specified interfaces.
  std::string cmd = 
    "sudo tc qdisc replace dev " + veth1 + " root netem delay " + std::to_string(currentDelay) +
    "ms loss " + std::to_string(currentDrop) + "% rate " + std::to_string(currentBandwidth) + "mbit; "
    "sudo tc qdisc replace dev " + veth2 + " root netem delay " + std::to_string(currentDelay) +
    "ms loss " + std::to_string(currentDrop) + "% rate " + std::to_string(currentBandwidth) + "mbit";

  std::cout << "[updateNetwork] Executing command: " << cmd << std::endl;
  int retCode = std::system(cmd.c_str());
  if (retCode != 0)
  {
    std::cerr << "Failed to update network settings with command: " << cmd << std::endl;
  }
}

// Callback for packet drop rate messages.
void cbDrop(const gz::msgs::Double &_msg)
{
  {
    std::lock_guard<std::mutex> lock(netMutex);
    // _msg.data() is a fraction (e.g. 0.95), so we convert it to a percentage.
    currentDrop = _msg.data() * 100.0;
    std::cout << "[cbDrop] Updated drop rate to: " << currentDrop << "%" << std::endl;
  }
  updateNetwork();
}

// Callback for bandwidth messages.
void cbBandwidth(const gz::msgs::Double &_msg)
{
  {
    std::lock_guard<std::mutex> lock(netMutex);
    // _msg.data() is in Mbps.
    currentBandwidth = _msg.data();
    std::cout << "[cbBandwidth] Updated bandwidth to: " << currentBandwidth << " Mbps" << std::endl;
  }
  updateNetwork();
}

// Callback for delay messages.
void cbDelay(const gz::msgs::Double &_msg)
{
  {
    std::lock_guard<std::mutex> lock(netMutex);
    // _msg.data() is in ms.
    currentDelay = _msg.data();
    std::cout << "[cbDelay] Updated delay to: " << currentDelay << " ms" << std::endl;
  }
  updateNetwork();
}

int main(int argc, char **argv)
{
  // The program now expects 4 arguments in pairs: robot_name veth_name robot_name veth_name.
  // argc will be 5 (including the program name).
  if (argc != 5)
  {
    std::cerr << "Usage: " << argv[0] << " <robot_name_1> <veth_name_1> <robot_name_2> <veth_name_2>\n"
              << "Example: " << argv[0] << " atlas veth10b88fe bestla veth32a464a\n";
    return -1;
  }

  // Retrieve robot names and veth interfaces from the command line.
  std::string robot1 = argv[1];
  veth1 = argv[2];
  std::string robot2 = argv[3];
  veth2 = argv[4];

  std::cout << "Configuring network emulation for link between:\n"
            << " - Robot: " << robot1 << " on interface " << veth1 << "\n"
            << " - Robot: " << robot2 << " on interface " << veth2 << std::endl;

  gz::transport::Node node;

  // Build topic names based on the robot names.
  // The topic name implies a direction, so we'll use robot1 as the source and robot2 as the target.
  std::string prefix          = "/robot_comms_emu_helper/" + robot1 + "_to_" + robot2;
  std::string dropTopic       = prefix + "/packet_drop_rate";
  std::string bandwidthTopic  = prefix + "/bandwidth";
  std::string delayTopic      = prefix + "/delay";

  // Subscribe to the packet drop topic.
  if (!node.Subscribe(dropTopic, cbDrop))
  {
    std::cerr << "Error subscribing to " << dropTopic << std::endl;
    return -1;
  }
  std::cout << "Subscribed to " << dropTopic << ", waiting for messages...\n";

  // Subscribe to the bandwidth topic.
  if (!node.Subscribe(bandwidthTopic, cbBandwidth))
  {
    std::cerr << "Error subscribing to " << bandwidthTopic << std::endl;
    return -1;
  }
  std::cout << "Subscribed to " << bandwidthTopic << ", waiting for messages...\n";

  // Subscribe to the delay topic.
  if (!node.Subscribe(delayTopic, cbDelay))
  {
    std::cerr << "Error subscribing to " << delayTopic << std::endl;
    return -1;
  }
  std::cout << "Subscribed to " << delayTopic << ", waiting for messages...\n"
            << "Press Ctrl+C to exit.\n";

  // Block until a shutdown signal is received.
  gz::transport::waitForShutdown();

  return 0;
}
