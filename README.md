# NetEm Based Simple Comms Emulator

This project provides a simple, yet powerful, network emulator using `tc` (traffic control) and `netem` (network emulator) to simulate various network conditions between robots. It is particularly useful for testing the resilience of robotic communication links to issues like **packet loss**, **latency**, and **bandwidth limitations**.

This emulator has two main components:

1.  **InitialSetup**: A C++ program that sets up the initial queuing discipline (`qdisc`) on a specified network device.
2.  **RobotToRobotCommsEmulator**: A C++ program that listens for Gazebo transport messages to dynamically adjust network parameters like packet drop rate, bandwidth, and delay between two specified robots.

-----

## Dependencies

  * **ignition-transport11**: This project requires the `ignition-transport11` library for communication.

-----

## Build Instructions

1.  **Navigate to the `build` directory**:

    ```bash
    cd build
    ```

2.  **Generate the build files with CMake**:

    ```bash
    cmake ..
    ```

3.  **Compile the emulator**:

    ```bash
    make InitialSetup RobotToRobotCommsEmulator
    ```

-----

## How to Use

### 1\. Initial Network Setup

First, you need to run the `InitialSetup` executable to configure the basic queuing discipline on your chosen network device. This only needs to be done once.

  * **Usage**:

    ```bash
    ./InitialSetup <device> <bands>
    ```

  * **Example**: The following command sets up a `prio` `qdisc` with **3** bands on the `lo` (loopback) device.

    ```bash
    ./InitialSetup lo 3
    ```

### 2\. Emulate Robot-to-Robot Communication Links

Next, for each communication link you want to emulate between two robots, you need to run an instance of the `RobotToRobotCommsEmulator`.

  * **Usage**:

    ```bash
    ./RobotToRobotCommsEmulator <source_robot> <target_robot>
    ```

  * **Examples**:

      * To emulate the link from a robot named `atlas` to a robot named `bestla`, run:
        ```bash
        ./RobotToRobotCommsEmulator atlas bestla
        ```
      * To emulate the return link from `bestla` to `atlas`, run:
        ```bash
        ./RobotToRobotCommsEmulator bestla atlas
        ```

### 3\. How the script Control Network Conditions

Once the `RobotToRobotCommsEmulator` is running, it subscribes to several Gazebo transport topics to receive commands for adjusting the network conditions. You can publish to these topics to dynamically change the simulated network link. The topics are constructed as follows:

`/robot_comms_emu_helper/<source_robot>_to_<target_robot>/<control_type>`

Here are the available controls:

  * **Packet Drop Rate**:

      * **Topic**: `/robot_comms_emu_helper/atlas_to_bestla/packet_drop_rate`
      * **Message Type**: `gz::msgs::Double`
      * **Value**: A value between `0.0` (0% drop) and `1.0` (100% drop).

  * **Bandwidth**:

      * **Topic**: `/robot_comms_emu_helper/atlas_to_bestla/bandwidth`
      * **Message Type**: `gz::msgs::Double`
      * **Value**: Bandwidth in Mbps.

  * **Delay**:

      * **Topic**: `/robot_comms_emu_helper/atlas_to_bestla/delay`
      * **Message Type**: `gz::msgs::Double`
      * **Value**: Delay in milliseconds.