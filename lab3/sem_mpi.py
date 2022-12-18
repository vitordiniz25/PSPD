import sys
import re

def conta(data):
    six = 0
    six_ten = 0
    ten = 0
    for word in data.split():
        size = len(word)
        if size < 6:
            six += 1
        elif size < 10:
            six_ten += 1
        else:
            ten += 1 

    return [six, six_ten, ten]

arq = open(sys.argv[1])
arq_buffer = arq.read()
arq.close()
res = conta(arq_buffer)

output = ['size < 6       :', '6 < size < 10  :', 'size > 10      :']
for i in range(0,3):
        print(f':: {output[i]} {res[i]}')