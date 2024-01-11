#extraction command: python3 extract.py > extracted.txt

import numpy as np

# Read in data that has been stored as raw signed 16-bit int samples (could work with quantized.bin directly?)
raw = list(np.fromfile("quantized.bin", dtype="int16"))
raw = raw[:768*16 + 16 + 32 + 1] #remove padding

print("#ifndef NN_VALUES_H\n#define NN_VALUES_H\n#include <stdint.h>\n\n")

print("const int32_t L1_WEIGHTS[] = {")
print(str(raw[:768*16])[1:-1]) #matrix is 768*16 (stored column-major)
print("};\nconst int32_t L1_BIASES[] = {")
print(str(raw[768*16 : 768*16 + 16])[1:-1]) #16 biases

print("};\nconst int32_t L2_WEIGHTS[] = {")
print(str(raw[768*16 + 16 : 768*16 + 16 + 32])[1:-1]) #matrix is 32*1
print("};\nconst int32_t L2_BIAS =")
print(str(raw[768*16 + 16 + 32])[1:-1]) #single value

print(";\n#endif")

#DEBUG
#print(len(raw), 768*16 + 16 + 32 + 1)

