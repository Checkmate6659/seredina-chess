#extraction command: python3 extract.py > nn_values.hpp

import numpy as np

# Read in data that has been stored as raw 32-bit float samples
raw = list(np.fromfile("f32.bin", dtype="float32"))

print("#ifndef NN_VALUES_H\n#define NN_VALUES_H\n\n")

print("const float L1_WEIGHTS[] = {")
print(str(raw[:768*64])[1:-1]) #matrix is 768*64 (stored column-major)
print("};\nconst float L1_BIASES[] = {")
print(str(raw[768*64 : 768*64 + 64])[1:-1]) #64 biases

print("};\nconst float L2_WEIGHTS[] = {")
print(str(raw[768*64 + 64 : 768*64 + 64 + 128])[1:-1]) #matrix is 128*1
print("};\nconst float L2_BIAS =")
print(str(raw[768*64 + 64 + 128])[1:-1]) #single value

print(";\n#endif")

#DEBUG
#print(len(raw), 768*64 + 64 + 128 + 1)

