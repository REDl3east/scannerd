#ifndef _CIRCULAR_ARRAY_H_
#define _CIRCULAR_ARRAY_H_

#include <algorithm>
#include <array>

template <typename T, std::size_t N>
class CircularArray {
public:
  void push(const T& value) {
    m_array[m_count++ % N] = value;
  }

  std::array<T, N>& array() {
    if (m_count > N) {
      std::rotate(m_array.begin(), m_array.begin() + (m_count % N), m_array.end());
      m_count = N;
    }
    return m_array;
  }

  std::size_t size() {
    return m_count;
  }

private:
  std::array<T, N> m_array;
  std::size_t m_count = 0;
};

#endif // _CIRCULAR_ARRAY_H_