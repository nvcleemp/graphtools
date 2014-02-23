'''
Created on Sep 22, 2013

@author: nvcleemp
'''
import sys

def writeGraph(lines, zeroBased):
    sys.stdout.write('{:c}'.format(len(lines)))
    for v,line in enumerate(lines):
        _,line = line.split(':')
        if not line.strip():
            continue
        for neighbour,sign in [(int(n.strip()[:-1]) + (1 if zeroBased else 0),n.strip()[-1]) for n in line.split(',')]:
            if v+1 < neighbour:
                sys.stdout.write('{:c}'.format(neighbour))
                sys.stdout.write('{:c}'.format(1) if sign=='+' else '{:c}'.format(0))
        sys.stdout.write('{:c}'.format(0))
        
#parse command-line arguments
zeroBased = '-0' in sys.argv

sys.stdout.write('>>signed_code<<')

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
