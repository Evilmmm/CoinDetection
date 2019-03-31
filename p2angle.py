import math

def distance(point,coef):
	return abs((coef[0]*point[0])-point[1]+coef[1])/math.sqrt((coef[0]*coef[0])+1)
	
lines = [[-60,3000]
,[19.2,-1958.4]
,[8.275862069,-1357.241379]
,[5.217391304,-1158.26087]
,[3.75,-1095]
,[2.774566474,-1032.138728]
,[2.142857143,-951.4285714]
,[1.785714286,-942.8571429]
]

angles = [15,10,5,0,-5,-10,-15,-20]

dist_list = []
x = 0

for line in lines:
	dist_list.append([distance([584,480-376],line), x])
	x = x + 1

#print(dist_list)
dist_list.sort()
print(angles[dist_list[0][1]])