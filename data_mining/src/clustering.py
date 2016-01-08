# -*- coding: utf-8 -*-
"""
Created on Fri Jan  1 15:20:44 2016

@author: Chenming WU 2015210928
@affiliation: THU
"""

import numpy as np
import matplotlib.pyplot as plt
import math
import random

filename = input('请输入聚类文件名：')
data = np.loadtxt(filename) # 读文件
data_length = len(data) # 数据长度
dist = np.zeros((data_length,data_length))
# 计算距离
array_dc = []
for i in range(0, data_length):
    for j in range(i + 1, data_length):
        dist[i,j] = np.linalg.norm(data[i] - data[j])
        dist[j,i] = dist[i,j]
        array_dc.append(dist[i,j])
array_dc = np.array(array_dc)
array_dc = np.sort(array_dc)
# one can choose dc so that the average number of neighbors is around 1 to 2% of 
# the total number of points in the data set
point = int(0.015 * len(array_dc))
dc = array_dc[point]
print('dc = %lf\n' % dc)

p = np.zeros((data_length,1))
delta = np.ones((data_length,1)) * 1e30    # Maxmimal Values

# Choose 1: normal kernel
""" 
for i in range(0,data_length):
    p[i] = len(np.where(dist[i][:] < dc)[0])
"""
# Choose 2: exponential kernel
for i in range(0, data_length):
    for j in range (i + 1, data_length):
        p[i] += math.exp(-(dist[i][j] / dc) ** 2)
        p[j] += math.exp(-(dist[i][j] / dc) ** 2)

# 计算所有点的密度
p_max = np.max(p)
for i in range(0, data_length):
    for j in range(0, data_length):
        if p[i] < p_max:       
            if p[j] > p[i] and dist[i][j] < delta[i]:
                delta[i] = dist[i][j]
        else:
           delta[i] = np.max(dist[i][:])


thP = 0.55 * (np.max(p) - np.min(p)) + np.min(p)
thDelta = 0.2 * (np.max(delta) - np.min(delta)) +np.min(delta)

clusters = -1 * np.ones(data_length, dtype = np.int)
centers = 0

for i in range(0, data_length):
    if p[i] > thP and delta[i] > thDelta:
        clusters[i] = centers
        centers += 1

for i in range(data_length):
    dist[i][i] = 1e30

print("聚成了%d类\n" % centers)

# 构造clusters的颜色用于plot
colors = []
for i in range(centers):
    colors.append((random.uniform(0,1),random.uniform(0,1),random.uniform(0,1)))

plt.figure()
for i in range(0, data_length):
    if clusters[i] == -1:
        plt.plot(p[i],delta[i], color = [0, 0, 1.0],marker = '.')
    else:
        plt.plot(p[i],delta[i],color = colors[clusters[i]],marker = 'o')
plt.xlabel('p'), plt.ylabel('delta')
plt.show()
# 为每个点聚类
for i in range(0, data_length):
    tmp = i
    while clusters[tmp] == -1:
        MAX = 1e28
        index = -1
        #each remaining point is assigned to the same cluster as its nearest neighbor of higher density
        for j in range(0, data_length):
            if j == tmp:
                continue
            if p[tmp] < p[j] and dist[tmp][j] < MAX:
                MAX = dist[tmp][j]
                index = j
        tmp = index
    clusters[i] = clusters[tmp]

            
plt.figure()
for i in range(0, data_length):
    index = clusters[i]
    plt.plot(data[i][0], data[i][1], color = colors[index], marker = '.')
plt.xlabel('x'), plt.ylabel('y')
plt.show()

