'''
@author: nvcleemp
'''
import sys

def writeGraphChar(lines, zeroBased):
    sys.stdout.write('{:c}'.format(len(lines)))
    for line in lines[:-1]:
        current, line = line.split(':')
        current = int(current) + (1 if zeroBased else 0)
        for neighbour in [int(n) + (1 if zeroBased else 0) for n in line.split(',')]:
            if neighbour > current:
                sys.stdout.write('{:c}'.format(neighbour))
        sys.stdout.write('{:c}'.format(0))

def writeShort(value):
    sys.stdout.write('{:c}'.format(value % 256))
    sys.stdout.write('{:c}'.format(value / 256))

def writeGraphShort(lines, zeroBased):
    sys.stdout.write('{:c}'.format(0))
    writeShort(len(lines))
    for line in lines[:-1]:
        current, line = line.split(':')
        current = int(current) + (1 if zeroBased else 0)
        for neighbour in [int(n) + (1 if zeroBased else 0) for n in line.split(',')]:
            if neighbour > current:
                writeShort(neighbour)
        writeShort(0)
        
def writeGraph(lines, zeroBased):
    if len(lines) < 253:
        writeGraphChar(lines, zeroBased)
    else:
        writeGraphShort(lines, zeroBased)
        
#parse command-line arguments
zeroBased = '-0' in sys.argv

sys.stdout.write('>>multi_code<<')

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
