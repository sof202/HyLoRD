#ifndef HYLORD_EXCEPTION_H_
#define HYLORD_EXCEPTION_H_

#include <stdexcept>
#include <string>

namespace Hylord {
class HylordException : public std::runtime_error {
  public:
   HylordException(const std::string& error_message) :
       std::runtime_error{"[HyLoRD] Error: " + error_message} {}
};

class FileReadException : public HylordException {
  public:
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
