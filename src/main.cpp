/**
 * @file    main.cpp
 * @brief   Defines entry point for HyLoRD.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <thread>

#include "CLI/CLI.hpp"
#include "cli.hpp"
#include "core/hylord.hpp"

int main(int argc, char** argv) {
   try {
      CLI::App hylord_cli;
      Hylord::CMD::HylordConfig config;

      Hylord::CMD::setup_cli(hylord_cli, config);
      CLI11_PARSE(hylord_cli, argc, argv);

      if (config.num_threads == 0)
         config.num_threads =
             static_cast<int>(std::thread::hardware_concurrency());

      return Hylord::run(config);
   } catch (...) {
      std::cerr << "An unexpected fatal error occurred.\n";
      return 1;
   }
}
