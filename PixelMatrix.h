#ifndef __PIXEL_MATRIX_H
#define __PIXEL_MATRIX_H

#include "Pixel.h"
#include <cstdint>

namespace ravecylinder {

enum MatrixType_t {
  HORIZONTAL_MATRIX,
  VERTICAL_MATRIX,
  HORIZONTAL_ZIGZAG_MATRIX,
  VERTICAL_ZIGZAG_MATRIX
};

class PixelMatrixBase {
protected:
  uint16_t m_Width, m_Height;
  struct CRGB *m_LED;
  struct CRGB m_OutOfBounds;

public:
  PixelMatrixBase();
  virtual uint32_t mXY(uint16_t x, uint16_t y) = 0;
  // Only used with externally defined LED arrays
  void SetLEDArray(struct CRGB *pLED);
};

template <uint16_t matrix_width, uint16_t matrix_height,
          MatrixType_t matrix_type>
class PixelMatrix : public PixelMatrixBase {

  virtual uint32_t GetPixelIndex(uint16_t row, uint16_t col) {
    if (matrix_type == HORIZONTAL_MATRIX)
      return ((col * matrix_width) + row);
    else if (matrix_type == VERTICAL_MATRIX)
      return ((row * matrix_height) + col);
    else if (tMType == HORIZONTAL_ZIGZAG_MATRIX) {
      if (col % 2)
        return ((((col + 1) * matrix_width) - 1) - row);
      else
        return ((col * matrix_width) + row);
    } else /* if (tMType == VERTICAL_ZIGZAG_MATRIX) */
    {
      if (row % 2)
        return ((((row + 1) * matrix_height) - 1) - col);
      else
        return ((row * matrix_height) + col);
    }
  }
};

} // namespace ravecylinder
#endif