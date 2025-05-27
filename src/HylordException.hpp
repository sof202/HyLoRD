#ifndef HYLORD_EXCEPTION_H_
#define HYLORD_EXCEPTION_H_

/**
 * @file    HylordException.hpp
 * @brief   Defines custom exception classes to be used by HyLoRD.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <stdexcept>
#include <string>
#include <system_error>

namespace Hylord {
class HylordException : public std::runtime_error {
  public:
   HylordException(const std::string& error_message) :
       std::runtime_error{"[HyLoRD] Error: " + error_message} {}
};

class FileReadException : public HylordException {
  public:
   explicit FileReadException(const std::string& file_name,
                              int error_number,
                              const std::string& details) :
       HylordException{"Failed to read file '" + file_name + "' (" +
                       std::system_category().message(error_number) +
                       "): " + details} {}
   explicit FileReadException(const std::string& file_name,
                              const std::string& details) :
       HylordException{"Failed to read file '" + file_name + "': " + details} {
   }
};

class FileWriteException : public HylordException {
  public:
   explicit FileWriteException(const std::string& file_name,
                               const std::string& details) :
       HylordException{"Failed to write to file '" + file_name +
                       "': " + details} {}
};

class PreprocessingException : public HylordException {
  public:
   explicit PreprocessingException(const std::string& step,
                                   const std::string& details) :
       HylordException{"Preprocesing failed at step '" + step +
                       "': " + details} {}
};

class DeconvolutionException : public HylordException {
  public:
   explicit DeconvolutionException(const std::string& step,
                                   const std::string& details) :
       HylordException{"Deconvolution failed at step '" + step +
                       "': " + details} {}
};
}  // namespace Hylord

#endif
