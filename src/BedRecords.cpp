#include "BedRecords.hpp"

#include <algorithm>
#include <iostream>
#include <string>

int BedRecords::parseChromosomeNumber(const std::string_view chr) {
   std::string chromosomeNumber{chr.substr(3, chr.size())};
   std::transform(chromosomeNumber.begin(),
                  chromosomeNumber.end(),
                  chromosomeNumber.begin(),
                  [](unsigned char c) { return std::tolower(c); });
   if (chromosomeNumber == "x") return 23;
   if (chromosomeNumber == "y") return 24;
   if (chromosomeNumber == "m") return 25;
   try {
      return std::stoi(chromosomeNumber);
   } catch (const std::invalid_argument& e) {
      std::cerr << "Could not obtain chromosome number for: " << chr << '\n';
      throw;
   }
}
