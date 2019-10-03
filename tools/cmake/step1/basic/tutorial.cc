#include <math.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <number>\n";
    exit(1);
  }
  double input_value = atof(argv[1]);
  double output_value = sqrt(input_value);
  std::cout << "The square root of " << input_value << " is " << output_value
            << std::endl;
  return 0;
}