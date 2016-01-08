import numpy as np
from scipy.optimize import fmin_l_bfgs_b
import scipy.sparse as sparse
import scipy.io
import matplotlib.pyplot as plt

def func(w, *args):
	X,Y,c = args
	yp = np.dot(X,w)
	idx = np.where(yp*Y < 1)[0]
	e = yp[idx] - Y[idx]
	cost = np.dot(e,e) + c * np.dot(w,w)
	grad = 2 * (np.dot(X[idx].T, e) + c*w)
	return cost, grad

'''
# 训练4个SVM分类器，使用一对其余法进行分类
# 使用L-BFGS方法求解二次凸优化问题
# 初始化一个随机的w，代入fmin_l_bfgs_b中
'''
w = []
def train_svm():
	global w
	x = np.loadtxt('svm-x.txt')
	Y = np.loadtxt('svm-y.txt')
	c = 0.001
	X = np.append(np.ones([x.shape[0], 1]), x , 1)
	for i in range(0,3):
		train_y = np.copy(Y)
		# 调整label
		train_y[np.where(train_y != i)[0]] = -1 
		train_y[np.where(train_y == i)[0]] = 1
		RET = fmin_l_bfgs_b(func, x0 = np.random.rand(X.shape[1]), args = (X, train_y, c), approx_grad = False)
		w.append(RET[0])
	return

def predict_svm(x,y):
	global w
	v_best = -1e30
	i_best = 0
	for i in range(0,3):
		v = w[i][1] * x + w[i][2] * y + w[i][0]
		print("Class %d 's value = %f" % (i ,v))
		if( v > 0 and v > v_best):
			v_best = v
			i_best = i
	print("SVM classify the input to Label %d" % i_best)


train_svm()
predict_svm(1,0.9)