# NetEm Based Simple Comms Emulator

## Build Instructions

1. Navigate to the `build` directory:
   ```bash
   cd build

2. Generate the build files with CMake:
    ```bash
    cmake ..

3. Compile the emulator:
    ```bash
    make InitialSetup RobotToRobotCommsEmulator

4. Run the initial setup:
    ```bash
    ./InitialSetup <device> <bands>

5. Run the robot to robot network link setup for all robot links:
    ```bash
    ./RobotToRobotCommsEmulator <source_robot> <target_robot> <parent_handle> <netem_handle> <port>

<!-- 3. Compile the emulator:
    ```bash
    make OhmsCommsEmulator

4. Run the emulator with robot names as arguments:
    ```bash
    ./OhmsCommsEmulator [robot1 name] [robot2 name] ... -->
