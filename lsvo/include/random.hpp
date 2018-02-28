#ifndef __RANDOM_HPP__
#define __RANDOM_HPP__

__host__ __device__ __inline__ unsigned int lcg(unsigned int& prev) {
  const uint LCG_A = 1664525u;
  const uint LCG_C = 1013904223u;
  prev = (LCG_A * prev + LCG_C);
  return prev & 0x00FFFFFF;
}

// Generate random float in [0, 1)
__host__ __device__ __inline__ float rnd(unsigned int& prev) {
  return static_cast<float>(lcg(prev)) / static_cast<float>(0x01000000);
}

__host__ __device__ __inline__ float rnd01(const unsigned int& r) {
  return static_cast<float>(r & 0x00FFFFFF) / static_cast<float>(0x01000000);
}

#endif
