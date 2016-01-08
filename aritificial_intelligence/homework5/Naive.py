import sys,os

# 转到data目录
os.chdir('data')
# 根目录
pwd = os.getcwd()
print (pwd)

name = os.listdir(pwd)
for filename in name:
	# print(filename)
	a = 1
	print("hello")