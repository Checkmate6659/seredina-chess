#extraction command: python3 extract.py > nn_values.hpp

import numpy as np

# Read in data that has been stored as raw 32-bit float samples
raw = list(np.fromfile("quantized.bin", dtype="int16"))

print("#ifndef NN_VALUES_H\n#define NN_VALUES_H\n\n")

print("const int16_t L1_WEIGHTS[] = {")
print(str(raw[:768*1024])[1:-1]) #matrix is 768*1024 (stored column-major)
print("};\nconst int16_t L1_BIASES[] = {")
print(str(raw[768*1024 : 768*1024 + 1024])[1:-1]) #1024 biases

print("};\nconst int16_t L2_WEIGHTS[] = {")
print(str(raw[768*1024 + 1024 : 768*1024 + 1024 + 2048])[1:-1]) #matrix is 2048*1
print("};\nconst int16_t L2_BIAS =")
print(str(raw[768*1024 + 1024 + 2048])[1:-1]) #single value

print(";\n#endif")

#DEBUG
#print(len(raw), 768*1024 + 1024 + 2048 + 1)

