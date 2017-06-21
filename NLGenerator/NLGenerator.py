#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import heapq
import numpy as np
import random

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm


class MAP:
    def __init__(self, X=8, Y=8, Z=3):
        self.X=X
        self.Y=Y
        self.Z=Z
        self.name="%d-%d-%d" % (self.X, self.Y, self.Z)
        self.map=np.zeros((self.X, self.Y, self.Z))
        self.mapdic=0
        self.n_line=0
        self.line=np.zeros((0,2,3))

    def show(self):
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        colors=["#aa0000", "#00aa00", "#0000aa", "#aaaa00", "#aa00aa", "#00aaaa", "#aaffaa", "#555500", "#550055", "#005555"]
        for i in range(1,self.n_line):
            x,y,z=np.where(self.map==i)
            x,y,z=self.sortline(self.line[i-1][0],self.line[i-1][1],x,y,z)
            ax.plot(x,y,z, "-", color=colors[i%len(colors)], ms=4, mew=0.5)
            x=[x[0],x[len(x)-1]]
            y=[y[0],y[len(y)-1]]
            z=[z[0],z[len(z)-1]]
            ax.scatter(x,y,z,s=50,c=colors[i%len(colors)])
        plt.show()

    def sortline(self, start, end, x, y, z):
        length=len(x)
        lines=np.append(np.append(x,y),z).reshape((3,length))
        lines=lines.T
        for i in range(1,length):
            if np.equal(start,lines[i]).all():
                lines[0],lines[i]=np.copy(lines[i]),np.copy(lines[0])
                break

        for i in range(0,len(x)-1):
            if np.linalg.norm(lines[i]-lines[i+1])==1:continue
            for j in range(i+2,len(x)):
                if np.linalg.norm(lines[i]-lines[j])==1:
                    lines[j],lines[i+1]=np.copy(lines[i+1]),np.copy(lines[j])
                    break

        lines=lines.T
        x,y,z=lines[0],lines[1],lines[2]
        return x,y,z

    def print(self):
        self.printQ()
        self.printA()

    def printQ(self):
        print(self.strQ())

    def printA(self):
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

    def save(self):
        self.saveQ()
        self.saveA()

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
        MAXLOOP=1000
        MAXLENGTH=10
        for i in range(MAXLOOP):
            start=np.array([[random.randrange(self.X), random.randrange(self.Y), random.randrange(self.Z)]])
            if len(self.isblank(start))!=0:break
        else:
            return False
        self.n_line=self.n_line+1
        point = start[0]
        self.map[tuple(point)]=self.n_line
        for i in range(MAXLENGTH):
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
            self.map[tuple(end)]=0
            self.n_line=self.n_line-1
            return False
        self.line=np.append(self.line, [[start[0], end]], axis=0)
        return True

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


    def generate(self, linenum):
        self.name="".join([self.name,"-%d" % (linenum)])
        for i in range(linenum):
            self.addLine()
        #for i in range(self.n_line):
        #    self.optLine(i)


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='NLGenerator')
    parser.add_argument('--x', '-x', default=32, type=int,
                        help='X size')
    parser.add_argument('--y', '-y', default=32, type=int,
                        help='Y size')
    parser.add_argument('--z', '-z', default=4, type=int,
                        help='Z size')
    parser.add_argument('--linenum', '-l', default=100, type=int,
                        help='Maximum number of lines')
    args = parser.parse_args()

    m=MAP(args.x, args.y, args.z)
    m.generate(args.linenum)

    m.save() 
    #m.saveQ()
    #m.saveA()
    #m.show()

