import os, cv2, re
import numpy as np
from sklearn.neighbors import KDTree
import math
import random
fourcc = cv2.cv.CV_FOURCC(*'DIB ')
out = cv2.VideoWriter('output.avi',fourcc, 20.0, (800,800))
grid = []
vecs = []
grid_uint = []

def jetColor(v, vmin = 0, vmax = 1):
    dv = vmax - vmin;
    r = 1
    g = 1
    b = 1
    if v < (vmin + 0.25 * dv):
        r = 0
        g = 4 * (v - vmin) / dv
    elif v < (vmin + 0.5 * dv):
        r = 0
        b = 1 + 4 * (vmin + 0.25 * dv - v) / dv
    elif v < (vmin + 0.75 * dv):
        r = 4 * (v - vmin - 0.5 * dv) / dv;
        b = 0;
    else:
        g = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
        b = 0;
    return np.uint8(255 * r), np.uint8(255 * g), np.uint8(255 * b)

def loadGridFile():
    with open('grid.txt') as f:
        for line in f:
            # result.append(line.split(','))
            content = line.split('[[')[1]
            content = content.split(']]')[0]
            content = content.split('],[')
            for group in content:
                parts = re.findall(r"[-+]?\d*\.\d+|[-+]?\d+", group)
                grid.append(parts)

def loadSimulationFile():
    with open('simulation.txt') as f:
        for line in f:
            content = line.split('[[[')[1]
            content = content.split(']]]')[0]
            content = content.split(']],[[')
            for group in content:
                vecs.append([])
                for pair in group.split('],['):
                    arr = pair.split(',')
                    if len(arr) != 2:
                        print 'error'
                        exit()
                    vecs[-1].append([float(arr[0]), float(arr[1])])

def convertGridFormat():
    for i in grid:
        x = int(800 * float(i[0]))
        y = int(800 * float(i[1]))
        grid_uint.append([x,y])

closestPos = []
closestDis = []

def constructKd():
    grid_float = []
    for i in grid:
        x = float(i[0])
        y = float(i[1])
        grid_float.append([x,y])
    kdt = KDTree(np.array(grid_float), leaf_size=30, metric='euclidean')
    for i in range(0,800):
        closestPos.append([])
        closestDis.append([])
        for j in range(0,800):
            dist, pos = kdt.query(np.array([float(i) / 800.0, float(j) / 800.0]).reshape(1, -1), k=8, return_distance=True)
            dist = dist[0]
            pos = pos[0]
            closestPos[i].append(pos)
            closestDis[i].append(dist)
    print 'constructed done'

if __name__ == "__main__":
    loadSimulationFile()
    loadGridFile()
    convertGridFormat()
    constructKd()

    for outter_loop in range(0,251):
        print '********************** Step ' + str(outter_loop) +' ********************'
        single_frame = np.zeros((800,800,3), np.uint8)
        mat_vecs = np.zeros((800, 800, 2), np.float)

        for i in range(0,800):
            for j in range(0,800):
                if single_frame[i,j,2] == 0:
                    dist = closestDis[i][j]
                    pos = closestPos[i][j]
                    sum_dist = sum(dist)
                    for k in range(0,len(dist)):
                        mat_vecs[i][j][0] = mat_vecs[i][j][0] + dist[k] * vecs[outter_loop][pos[k]][0]
                        mat_vecs[i][j][1] = mat_vecs[i][j][1] + dist[k] * vecs[outter_loop][pos[k]][1]
                    mat_vecs[i][j][0] = mat_vecs[i][j][0] / sum_dist
                    mat_vecs[i][j][1] = mat_vecs[i][j][1] / sum_dist

        voticityMat = np.zeros((800,800), np.float)
        for i in range(0,800):
            for j in range(0,800):
                next_i = i + 1
                next_j = j + 1
                if next_i == 800:
                    next_i = 799
                if next_j == 800:
                    next_j = 799
                # speed
                #voticityMat[j][i] = math.sqrt(mat_vecs[i][j][1] * mat_vecs[i][j][1]  + mat_vecs[i][j][0] * mat_vecs[i][j][0])
                # voticity
                voticityMat[j][i] = (mat_vecs[next_i][j][1] - mat_vecs[i][j][1]) - (mat_vecs[i][next_j][0] - mat_vecs[i][j][0])

        for i in range(0,800):
            for j in range(0,800):
                if voticityMat[i][j] < -0.1:
                    voticityMat[i][j] = -0.1
                elif voticityMat[i][j]  > 0.1:
                    voticityMat[i][j] = 0.1

        voticity_max = voticityMat.max()
        voticity_min = voticityMat.min()

        for i in range(0,800):
            for j in range(0,800):
                voticityMat[i][j] = ( voticityMat[i][j] - voticity_min ) / (voticity_max - voticity_min)

        # draw heat map
        gray_image = np.zeros((800, 800, 1), np.uint8)
        for i in range(0, 800):
            for j in range(0, 800):
                gray_image[i][j] = np.uint8(255 * voticityMat[i][j])

        cv2.applyColorMap(gray_image,3,single_frame)
        out.write(single_frame)
        cv2.imwrite('speed-'+str(outter_loop) +'.jpg', single_frame)
    out.release()