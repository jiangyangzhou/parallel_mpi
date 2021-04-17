from numpy import genfromtxt
import numpy as np
import csv
my_data = genfromtxt('life.out1', delimiter=',')

data = []
with open('life.out1','r') as f:
    lines = f.readlines()
    for line in lines:
        i=0
        data.append([int(a) for a in line.strip().split(',')])

my_data = np.array(data)

ans = np.zeros([250,250])
with open('life_out.ans', 'r') as f:
    lines = f.readlines()
    for line in lines:
        x, y = [int(a) for a in line.split()]
        ans[x,y] = 1
print(my_data.shape)
match = True
for i in range(250):
    for j in range(250):
        if my_data[i,j]!=ans[i,j]:
            match = False
            print(i,j,my_data[i,j], ans[i,j])
        elif my_data[i,j] and ans[i,j]:
            print(i,j)
print("Two matrix is match? %s"%(match))

def data_to_csv():
    board = np.zeros((250,250)).astype(np.int32)
    with open("life_out.ans", 'r') as f:
        lines = f.readlines()
        for line in lines:
            x,y = [int(i) for i in line.split()]
            board[x,y] = 1
    with open("board_ans.csv", 'w') as f:
        for i in range(250):
            for j in range(250):
                f.write("%d"%board[i][j])
            f.write('\n')
data_to_csv()