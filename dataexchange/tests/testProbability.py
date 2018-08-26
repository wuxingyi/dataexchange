#!/usr/bin/python

from scipy.special import comb
import matplotlib.pyplot as plt
import numpy as np
from pylab import *
import sys

def prob(total_chunks_nr, revealed_chunks_nr, corrupted_chunks_nr):
    if total_chunks_nr <= corrupted_chunks_nr:
        return 0
    if total_chunks_nr <= revealed_chunks_nr:
        return 0
    s = comb(total_chunks_nr, revealed_chunks_nr)
    f=min(revealed_chunks_nr, corrupted_chunks_nr)
    q=0
    for i in range(1, f+1):
       q += comb(corrupted_chunks_nr,i) * comb(total_chunks_nr - corrupted_chunks_nr, revealed_chunks_nr-i)
    return (1 - q/s) 

def testmaxreveal():
    z = {}
    for i in range(2, 2000):
        z[i] = prob(i, i - 1, 1)
        print z[i]

    x=[]
    y=[]
    for i, p in z.items():
        x.append(i)
        y.append(p)
    
    figure()
    plt.plot(x ,y ,'b',linewidth = 2, label = 'corrupt_nr=1, revealed_nr=total_nr-1')
    plt.legend(loc="upper right")
    plt.grid()
    plt.show()
    

def testcorrupt():
    total_nr = 100
    revealed_nr = 60
    z = {}
    for i in range(1, 20):
        z[i] = prob(total_nr, revealed_nr, i)
        print z[i]

    x=[]
    y=[]
    for i, p in z.items():
        x.append(i)
        y.append(p)
    
    figure()
    plt.plot(x ,y ,'b',linewidth = 2, label = 'revealed_nr=60')
    plt.legend(loc="upper right")
    plt.grid()
    plt.show()
    
def testreveal():
    total_nr = 100
    corrupt_nr = 5
    z = {}
    for i in range(1, 80):
        z[i] = prob(total_nr, i, corrupt_nr)
        print z[i]

    x=[]
    y=[]
    for i, p in z.items():
        x.append(i)
        y.append(p)
    
    figure()
    plt.plot(x ,y ,'b',linewidth = 2, label = 'corrupt_nr=5')
    plt.legend(loc="upper right")
    plt.grid()
    plt.show()


if __name__ == '__main__':
    #testreveal()
    #testcorrupt()
    testmaxreveal()
