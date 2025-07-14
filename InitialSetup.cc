#include <iostream>
#include <string>
#include <cstdlib> // For std::system()

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " <device> <bands>\n"
              << "Example: " << argv[0] << " lo 8\n";
    return -1;
  }

  std::string dev = argv[1];
  std::string bands = argv[2];

  // Clean slate: remove any existing qdisc
  {
    std::string cmd = "sudo tc qdisc del dev " + dev + " root";
    std::system(cmd.c_str()); // Might fail if none exists, ignore.
  }

  // Setup prio qdisc with specified number of bands
  {
    // Create priomap string: "0 0 0 0 ..." repeated 16 times (one for each priority)
    // All mapped to band 0 by default.
    // For simplicity, we can just map all to 0; advanced setups might vary.
    std::string priomap = "";
    for (int i = 0; i < 16; ++i)
    {
      priomap += "0 ";
    }

    std::string cmd = "sudo tc qdisc add dev " + dev + " root handle 1: prio bands " + bands + " priomap " + priomap;
    int ret = std::system(cmd.c_str());
    if (ret != 0)
    {
      std::cerr << "Failed to set prio qdisc with " << bands << " bands on " << dev << std::endl;
      return -1;
    }
    else
    {
      std::cout << "Prio qdisc set up on " << dev << " with " << bands << " bands." << std::endl;
    }
  }

  return 0;
}