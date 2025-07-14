#include <iostream>
#include <string>
#include <cstdlib>   // For std::system()
#include <mutex>
#include <gz/msgs.hh>
#include <gz/transport.hh>

// Global variables for current network settings.
double currentDrop = 0.0;        // Loss percentage.
double currentBandwidth = 0.0;   // Bandwidth in Mbps.
double currentDelay = 0.0;       // Delay in ms.

// Mutex to protect the global variables.
std::mutex netMutex;

// Function to update the network settings by executing a combined command.
void updateNetwork()
{
  // Lock the mutex to safely read the current values.
  std::lock_guard<std::mutex> lock(netMutex);
  
  // Build the command string.
  // This command updates two interfaces (adjust interface names if required).
  std::string cmd = 
    "sudo tc qdisc replace dev veth10b88fe root netem delay " + std::to_string(currentDelay) +
    "ms loss " + std::to_string(currentDrop) + "% rate " + std::to_string(currentBandwidth) + "mbit; "
    "sudo tc qdisc replace dev veth32a464a root netem delay " + std::to_string(currentDelay) +
    "ms loss " + std::to_string(currentDrop) + "% rate " + std::to_string(currentBandwidth) + "mbit";

  // std::string cmd = 
  //   "sudo tc qdisc replace dev vethfee9e1c root netem delay 2.5ms loss " + std::to_string(currentDrop) + "% rate 72mbit; "
  //   "sudo tc qdisc replace dev veth8aa7092 root netem delay 2.5ms loss " + std::to_string(currentDrop) + "% rate 72mbit";
  
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
    // _msg.data() is a fraction (e.g. 0.95) and convert to percentage.
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
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " <source_robot> <target_robot>\n"
              << "Example: " << argv[0] << " atlas bestla\n";
    return -1;
  }

  // // Retrieve source and target robot names from the command line.
  std::string sourceRobot = argv[1];
  std::string targetRobot = argv[2];

  gz::transport::Node node;

  // Build topic names based on the robot names.
  std::string prefix          = "/robot_comms_emu_helper/" + sourceRobot + "_to_" + targetRobot;
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
