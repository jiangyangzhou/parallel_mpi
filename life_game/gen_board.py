import numpy as np

def gen_myself():
    glider = [[1, 0, 0],
            [0, 1, 1],
            [1, 1, 0]]
    X = np.zeros((8, 8)).astype(np.int32)
    X[:3,:3] = glider
    with open("board.csv", 'w') as f:
        for i in range(8):
            for j in range(8):
                f.write("%d,"%X[i][j])
            f.write('\n')
#np.savetxt("board.csv", X, delimiter=",")

def data_to_csv():
    board = np.zeros((1000,1000)).astype(np.int32)
    with open("life.data1", 'r') as f:
        lines = f.readlines()
        for line in lines:
            x,y = [int(i) for i in line.split()]
            board[x,y] = 1
    with open("board.csv", 'w') as f:
        for i in range(1000):
            for j in range(1000):
                f.write("%d,"%board[i][j])
            f.write('\n')

data_to_csv()
#gen_myself()