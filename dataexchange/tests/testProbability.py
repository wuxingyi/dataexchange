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
    return 1 - q/s

if __name__ == '__main__':
    total_nr = 100
    corrupt_nr = 10
    z = {}
    for i in range(1, total_nr, 1):
        z[i] = prob(total_nr, i, corrupt_nr)

    x=[]
    y=[]
    for i, p in z.items():
        x.append(i)
        y.append(p)
    
    plt.plot(x ,y ,'b',linewidth = 2, label = 'Line1')
    plt.show()

        
        
