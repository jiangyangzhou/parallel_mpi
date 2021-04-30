import numpy as np
n = 500
prob = np.random.rand(n).astype(np.float32)
prob = prob / np.sum(prob)
with open('prob.txt', 'wb') as f:
    f.write(np.array([n]).astype(np.int32))
    f.write(prob)