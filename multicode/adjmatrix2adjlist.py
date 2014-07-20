'''
@author: nvcleemp
'''
import sys

def writeAdjacencyList(lines, zeroBased):
    for current, line in enumerate(lines):
        sys.stdout.write('{}: '.format(current + (0 if zeroBased else 1)))
        sys.stdout.write(', '.join([str(i + (0 if zeroBased else 1)) for (i,adjacent) in enumerate(line) if adjacent=='1']))
        sys.stdout.write('\n')
        
#parse command-line arguments
zeroBased = '-0' in sys.argv

lines = []
templine = ''
for line in sys.stdin:
    line = line.strip()
    if line:
        templine += ' ' + line
    elif not templine:
        writeAdjacencyList(lines, zeroBased)
        lines=[]

    if ']' in templine:
        lines.append(templine.rstrip(']').lstrip('[').split())
        templine = ''

if lines:
    writeAdjacencyList(lines, zeroBased)
