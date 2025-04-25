#ifndef HYLORDCONCEPTS_H_
#define HYLORDCONCEPTS_H_

/**
 * @file    concepts.hpp
 * @brief   Defines concepts to be used in HyLoRD.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "types.hpp"

/// Holds concept and template for working with TSVRecords
namespace Hylord::Records {

template <typename T>
/**
 * @concept TSVRecord
 * @brief Requirements for types that can be parsed from TSV fields.
 *
 * This concept specifies the interface that record types must implement to be
 * compatible with the TSVFileReader class.
 *
 * ### Requirements
 * - Must provide a static `fromFields` method that:
 *   - Takes a `const Fields&` parameter (vector of strings representing TSV
 * fields)
 *   - Returns an instance of the type T
 *   - May throw exceptions (not marked noexcept)
 *   - Must be a static method
 *
 * ### Example
 * A conforming type would look like:
 * @code
 * struct Record {
 *     int id;
 *     std::string name;
 *
 *     static Record fromFields(const Fields& fields) {
 *         if (fields.size() < 2) throw std::runtime_error("Not enough
 * fields"); return {std::stoi(fields[0]), fields[1]};
 *     }
 * };
 * @endcode
 *
 * @tparam T The type to check against the TSVRecord requirements
 * @see TSVFileReader
 */
concept TSVRecord = requires(const Fields& fields) {
   requires std::is_same_v<decltype(T::fromFields(fields)), T>;
   requires !noexcept(T::fromFields(fields));
   requires !std::is_member_function_pointer_v<decltype(&T::fromFields)>;
};

template <TSVRecord T>
using Collection = std::vector<T>;
}  // namespace Hylord::Records

#endif
