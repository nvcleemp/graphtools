'''
Created on Sep 22, 2013

@author: nvcleemp
'''
import sys

def writeGraph(lines, zeroBased):
    sys.stdout.write('{:c}'.format(len(lines)))
    for line in lines:
        _,line = line.split(':')
        for neighbour in [int(n) + (1 if zeroBased else 0) for n in line.split(',')]:
            sys.stdout.write('{:c}'.format(neighbour))
        sys.stdout.write('{:c}'.format(0))
        
#parse command-line arguments
zeroBased = '-0' in sys.argv

sys.stdout.write('>>planar_code<<')

lines = []
for line in sys.stdin:
    line = line.strip()
    if line:
        lines.append(line)
    else:
        writeGraph(lines, zeroBased)
        lines=[]

if lines:
    writeGraph(lines, zeroBased)
