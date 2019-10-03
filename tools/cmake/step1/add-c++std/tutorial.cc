#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "TutorialConfig.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << argv[0] << " Version " << Tutorial_VERSION_MAJOR << '.'
              << Tutorial_VERSION_MINOR << std::endl;
    std::cout << "Usage: " << argv[0] << " <number>\n";
    exit(1);
  }
  double input_value = atof(argv[1]);
  double output_value = sqrt(input_value);
  std::cout << "The square root of " << input_value << " is " << output_value
            << std::endl;
  return 0;
}