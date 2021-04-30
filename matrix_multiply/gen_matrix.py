import numpy as np 
import struct
from struct import Struct

def gen_random_bmatrix(m,n):
    mat1 = np.random.rand(m,n).astype(np.float64)
    with open("matrix.mat", 'wb') as f:
        f.write(np.array([m,n]).astype(np.int32))
        f.write(mat1)
    return mat1

def gen_random_bvector(n):
    vec = np.random.rand(n).astype(np.float64)
    with open("vector.mat", "wb") as f:
        f.write(np.array([n]).astype(np.int32))
        f.write(vec)
    return vec

if __name__=="__main__":
    m, n = 2000, 3000
    mat1 = gen_random_bmatrix(m, n)
    vec = gen_random_bvector(n)
    #print(mat1[:0].sum())
    #print(np.matmul(mat1,vec.reshape([n,1])))
    print(np.matmul(mat1,vec))


