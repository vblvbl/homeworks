import os, cv2, re
import numpy as np
from sklearn.neighbors import KDTree
import math
import random
fourcc = cv2.cv.CV_FOURCC(*'DIB ')
out = cv2.VideoWriter('output.avi',fourcc, 25.0, (800,800))
grid = []
vecs = []
grid_uint = []

# jixian
seeds = []
seed_image = np.zeros((800,800,3), np.uint8)

def image_add(img1, img2):
    rows, cols, channels = img2.shape
    roi = img1[0:rows, 0:cols]
    img2gray = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY)
    ret, mask = cv2.threshold(img2gray, 10, 255, cv2.THRESH_BINARY)

    mask_inv = cv2.bitwise_not(mask)
    img1_bg = cv2.bitwise_and(roi, roi, mask=mask_inv)

    img2_fg = cv2.bitwise_and(img2, img2, mask=mask)

    dst = cv2.add(img1_bg, img2_fg)
    img1[0:rows, 0:cols] = dst

def initSeeds():
    for i in range(0, 51):
        seeds.append([799 , i * 16])


# x should be a float value between 0.0 to 1.0
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
            # result.append(line.split(','))
            content = line.split('[[[')[1]
            content = content.split(']]]')[0]
            content = content.split(']],[[')
            # 251 groups
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

    # jixian
    initSeeds()
    for outter_loop in range(0,251):
        print '********************** Step ' + str(outter_loop) +' ********************'
        single_frame = np.zeros((800,800,3), np.uint8)
        mat_vecs = np.zeros((800, 800, 2), np.float)
        # what should I do tomorrow: 1. store query in memory 2. change [0,800] to float [0,1]
        for i in range(0,800):
            for j in range(0,800):
                if single_frame[i,j,2] == 0:
                    dist = closestDis[i][j]
                    pos = closestPos[i][j]
                    sum_dist = sum(dist)
                    #print sum_dist
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
                voticityMat[j][i] = math.sqrt(mat_vecs[i][j][1] * mat_vecs[i][j][1]  + mat_vecs[i][j][0] * mat_vecs[i][j][0])
                # voticity
                #voticityMat[j][i] = (mat_vecs[next_i][j][1] - mat_vecs[i][j][1]) - (mat_vecs[i][next_j][0] - mat_vecs[i][j][0])
        voticity_max = voticityMat.max()
        voticity_min = voticityMat.min()

        for i in range(0,800):
            for j in range(0,800):
                voticityMat[i][j] = ( voticityMat[i][j] - voticity_min ) / (voticity_max - voticity_min)

        for i in range(0, 800):
            for j in range(0, 800):
                r,g,b = jetColor(voticityMat[i][j])
                single_frame[i][j] = [b,g,r]

        nb_seed = 0
        for i in range(0,len(seeds)):
            print 'current seed = ' ,nb_seed
            nb_seed = nb_seed + 1
            for j in range(0,10):
                if seeds[i][0] < 0 or seeds[i][0] >= 800 or seeds[i][1] < 0 or seeds[i][1] >= 800:
                    break
                start_x = int(seeds[i][0])
                start_y = int(seeds[i][1])
                seeds[i][0] = seeds[i][0] + mat_vecs[start_x][start_y][0]
                seeds[i][1] = seeds[i][1] + mat_vecs[start_x][start_y][1]
                if seeds[i][0] < 0 or seeds[i][0] >= 800 or seeds[i][1] < 0 or seeds[i][1] >= 800:
                    break
                cv2.line(seed_image, (start_x, start_y), (int(seeds[i][0]), int(seeds[i][1])), (0, 0, 255), 1)

        image_add(single_frame, seed_image)

        out.write(single_frame)
        cv2.imwrite('speed-'+str(outter_loop) +'.jpg', single_frame)
    out.release()