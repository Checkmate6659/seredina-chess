#extraction command: python3 extract.py > extracted.txt

import numpy as np

# Read in data that has been stored as raw 32-bit float samples
raw = list(np.fromfile("f32.bin", dtype="float32"))
#print("Raw data:")
#print(raw)

print("\nL1 Weights (stored column-major):")
print(raw[:768*16]) #matrix is 768*16 (stored column-major)
print("L1 Biases:")
print(raw[768*16 : 768*16 + 16]) #16 biases

print("L2 Weights:")
print(raw[768*16 + 16 : 768*16 + 16 + 32]) #matrix is 32*1
print("L2 Bias:")
print(raw[768*16 + 16 + 32]) #single value
print(len(raw), 768*16 + 16 + 32 + 1)

