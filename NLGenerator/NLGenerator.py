import numpy as np
import random
import heapq

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm


class MAP:
    def __init__(self, X=8, Y=8, Z=3, n_lineseed=-1):
        self.X=X
        self.Y=Y
        self.Z=Z
        self.name="%d-%d-%d" % (self.X, self.Y, self.Z)
        self.map=np.zeros((self.X, self.Y, self.Z))
        self.mapdic=0
        self.n_line=0
        self.line=np.zeros((0,2,3))

    def show(self):
        #x = np.arange(-5,5,0.05)
        x = np.arange(0,self.X,0.05)
        y = np.arange(0,self.Y,0.05)
        X, Y = np.meshgrid(x, y)
        #Z = X*X+Y*Y

        def func(X, Y):
            return X*X+Y*Y

        Z=func(X,Y)
        xp=x
        yp=y
        zp=func(xp,yp)

        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        #ax.plot_surface(X,Y,Z,cmap=cm.coolwarm)
        #plt.plot([0,4],[2,3],[1,2])
        ax.plot(xp,yp,zp, "o-", color="#aaffee", ms=4, mew=0.5)
        plt.show()

    def print(self):
        print(self.strQ())
        print(self.strA())

    def strQ(self):
        str=[]
        str.append("SIZE %dX%dX%d\n" % (self.X, self.Y, self.Z))
        str.append("LINE_NUM %d\n\n" % (self.n_line))
        for i,line in enumerate(self.line):
            str.append("LINE#%d (%d,%d,%d) (%d,%d,%d)\n" % (i+1, line[0][0], line[0][1], line[0][2]+1, line[1][0], line[1][1], line[1][2]+1))
        return "".join(str)

    def strA(self):
        str=[]
        str.append("SIZE %dX%dX%d\n" % (self.X, self.Y, self.Z))
        for zm in range(self.Z):
            z=zm+1
            str.append("LAYER %d\n" % (z))
            for y in range(self.Y):
                for x in range(self.X-1):
                    str.append("%d," % self.map[x][y][zm])
                str.append("%d\n" % self.map[self.X-1][y][zm])
        return "".join(str)

    def saveQ(self):
        filename="Q-"+self.name+".txt"
        f=open(filename, 'w')
        f.write(self.strQ())
        f.close()

    def saveA(self):
        filename="A-"+self.name+".txt"
        f=open(filename, 'w')
        f.write(self.strA())
        f.close()

    def isregular(self, points):
        a=np.all(points>=0,axis=1)
        b=points[:,0]<self.X#0-col vector
        c=points[:,1]<self.Y#1-col vector
        d=points[:,2]<self.Z#2-col vector

        e=a*b*c*d
        return points[e]

    def isblank(self, points):
        l=np.array([],dtype=bool)
        for point in points:
            if self.map[tuple(point)]==0:
                l=np.append(l, True)
            else:
                l=np.append(l, False)
        return points[l]

    def istip(self, points):
        l=np.array([],dtype=bool)
        for point in points:
            nlist=self.neighbour(point)
            nlist=self.isregular(nlist)
            sum=0
            for n in nlist:
                if self.map[tuple(n)]==self.n_line:
                    sum=sum+1
            if sum>=2:
                l=np.append(l,True)
            else:
                l=np.append(l,False)
        return points[l==False]

    def neighbour(self, point):
        dlist=np.array([[0,0,-1], [0,0,1], [0,-1,0], [0,1,0], [-1,0,0], [1,0,0]])
        nlist=point+dlist
        return nlist

    def addLine(self):
        start=np.array([[random.randrange(self.X), random.randrange(self.Y), random.randrange(self.Z)]])
        #start=np.array([[0,0,-1], [0,0,1], [0,-1,0], [0,1,0], [-1,0,0], [1,0,0]])
        if len(self.isblank(start))==0:
            return
        self.n_line=self.n_line+1
        point = start[0]
        self.map[tuple(point)]=self.n_line
        for i in range(10):
            #self.print()
            points = self.neighbour(point)
            points = self.isregular(points)
            points = self.isblank(points)
            points = self.istip(points)
            if len(points)==0:
                break
            point=random.choice(points)
            self.map[tuple(point)]=self.n_line

        end=point
        if np.array_equal(start[0],end):
            self.map[self.map==self.n_line]=0
            self.n_line=self.n_line-1
            return
        self.line=np.append(self.line, [[start[0], end]], axis=0)

    def optLine(self, n_line):
        MAX=72*72*8
        dlist=np.array([[0,0,-1], [0,0,1], [0,-1,0], [0,1,0], [-1,0,0], [1,0,0]])
        self.map[self.map==n_line]=0
        q=[]

        heapq.heappush(q, (0, self.line[i][0], [0,0,0]))
         
        while True:
            if q == []:
                break
            (priority, point, direction) = heapq.heappop(q)
            next_points=point+dlist
            boollist=isregular(next_points)
            for n, d in zip(next_points[boollist],dlist[boollist]):
                if self.map[tuple(n)]!=0:continue
                if len(isregular(next_point))==0:continue
                if np.array_equal(direction, d):
                    i


    def generate(self):
        for i in range(100):
            self.addLine()
        #for i in range(self.n_line):
       #     self.optLine(i)


if __name__ == "__main__":
    m=MAP(4,4,2)
    m.generate()

    m.print() 
    #m.saveQ()
    #m.saveA()
    #m.show()

