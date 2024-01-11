#extraction command: python3 extract.py > extracted.txt

import numpy as np

# Read in data that has been stored as raw signed 16-bit int samples (could work with quantized.bin directly?)
raw = list(np.fromfile("quantized_trunc.bin", dtype="int16"))

print("#ifndef NN_VALUES_H\n#define NN_VALUES_H\n\n")

print("const float L1_WEIGHTS[] = {")
print(str(raw[:768*16])[1:-1]) #matrix is 768*16 (stored column-major)
print("};\nconst float L1_BIASES[] = {")
print(str(raw[768*16 : 768*16 + 16])[1:-1]) #16 biases

print("};\nconst float L2_WEIGHTS[] = {")
print(str(raw[768*16 + 16 : 768*16 + 16 + 32])[1:-1]) #matrix is 32*1
print("};\nconst float L2_BIAS =")
print(str(raw[768*16 + 16 + 32])[1:-1]) #single value

print(";\n#endif")

#DEBUG
#print(len(raw), 768*16 + 16 + 32 + 1)

