#extraction command: python3 extract.py > nn_values.hpp

import numpy as np

# Read in data that has been stored as raw 32-bit float samples
raw = list(np.fromfile("f32.bin", dtype="float32"))

print("#ifndef NN_VALUES_H\n#define NN_VALUES_H\n\n")

print("const float L1_WEIGHTS[] = {")
print(str(raw[:768*32])[1:-1]) #matrix is 768*32 (stored column-major)
print("};\nconst float L1_BIASES[] = {")
print(str(raw[768*32 : 768*32 + 32])[1:-1]) #32 biases

print("};\nconst float L2_WEIGHTS[] = {")
print(str(raw[768*32 + 32 : 768*32 + 32 + 64])[1:-1]) #matrix is 64*1
print("};\nconst float L2_BIAS =")
print(str(raw[768*32 + 32 + 64])[1:-1]) #single value

print(";\n#endif")

#DEBUG
#print(len(raw), 768*32 + 32 + 64 + 1)

