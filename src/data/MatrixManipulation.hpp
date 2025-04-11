#ifndef MATRIX_MANIPULATION_H_
#define MATRIX_MANIPULATION_H_

#include "types.hpp"

namespace Hylord::MatrixManipulation {
Matrix gramMatrix(const Matrix& m);
Vector generateCoefficientVector(const Matrix& reference_matrix,
                                 const Vector& bulk_data);

}  // namespace Hylord::MatrixManipulation

#endif
