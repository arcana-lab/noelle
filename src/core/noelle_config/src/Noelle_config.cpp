#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include "../build/src/config.hpp"

void print_usage(char *argv[]) {
  auto binName = argv[0];
  std::cout << "USAGE: " << binName << " <OPTION>..." << std::endl;
  std::cout << std::endl;

  std::cout << "Options:" << std::endl;

  std::cout << "  --version     "
            << "\t\t"
            << "Print NOELLE version." << std::endl;

  std::cout << "  --prefix      "
            << "\t\t"
            << "Print the installation prefix." << std::endl;

  std::cout << "  --flags       "
            << "\t\t"
            << "Print the compilation options used to compile NOELLE."
            << std::endl;

  std::cout
      << "  --llvm-prefix "
      << "\t\t"
      << "Print the installation prefix of the specific LLVM used by NOELLE."
      << std::endl;

  std::cout << "  --llvm-version"
            << "\t\t"
            << "Print the version of the specific LLVM used by NOELLE."
            << std::endl;

  std::cout
      << "  --llvm-flags  "
      << "\t\t"
      << "Print the compilation options used to compile the specific LLVM used by NOELLE."
      << std::endl;

  return;
}

int main(int argc, char *argv[]) {

  /*
   * Fetch the inputs
   */
  if (argc < 2) {
    print_usage(argv);
    return 0;
  }

  int opt;
  static struct option long_options[] = { { "version", 0, NULL, 'n' },
                                          { "prefix", 0, NULL, 'p' },
                                          { "flags", 0, NULL, 'c' },
                                          { "llvm-version", 0, NULL, 'l' },
                                          { "llvm-prefix", 0, NULL, 'd' },
                                          { "llvm-flags", 0, NULL, 'o' },
                                          { NULL, 0, NULL, 0 } };
  while ((opt = getopt_long(argc, argv, "npcldo", long_options, NULL)) != -1) {

    switch (opt) {

      case 'n':
        std::cout << NOELLE_VERSION << std::endl;
        break;

      case 'p':
        std::cout << NOELLE_DESTINATION << std::endl;
        break;

      case 'c':
        std::cout << NOELLE_COMPILATION_OPTIONS << std::endl;
        break;

      case 'l':
        std::cout << LLVM_VERSION << std::endl;
        break;

      case 'd':
        std::cout << LLVM_DIR << std::endl;
        break;

      case 'o':
        std::cout << LLVM_COMPILATION_OPTIONS << std::endl;
        break;

      default:
        print_usage(argv);
    }
  }

  return 0;
}
