import sys
import re
from mpi4py import MPI
from textwrap import wrap

if len(sys.argv) != 2:
    print(f'Insira o caminho do arquivo que sera lido.\nExemplo: mpiexec -n 4 python {sys.argv[0]} ./files/med.txt')
    sys.exit()

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

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
num_proc = comm.Get_size()
hostname = MPI.Get_processor_name()

print(f'rank({rank}) esta rodando no host: {hostname}')

res = [0, 0, 0]
data = ''

if rank == 0:
    print(f'Mestre esta lendo arquivo {sys.argv[1]}')

    arq = open(sys.argv[1])
    arq_buffer = arq.read()
    arq.close()
    print(f'Arquivo possui {len(arq_buffer)} bytes.')
    partes = []
    size = (len(arq_buffer) // (num_proc))
    if size % 2 == 0:
        partes = wrap(arq_buffer, (size))
    else: 
        partes = wrap(arq_buffer, (size + 1))
        
    for i in range(1, num_proc):
        print(f'Mestre enviou {len(partes[i])} bytes para o escravo {i}')
        comm.send(partes[i], dest=i, tag=2)
    data = partes[0]
else:
    data = comm.recv(source=0, tag=2)
    print(f'Escravo  recebeu {len(data)} bytes do processo Mestre ')

res = conta(data)
print(f'rank {rank} contabilizou {len(data)} bytes')

if rank != 0:
    print(f'rank {rank} enviando {res} para o mestre')
    comm.send(res, dest=0, tag=2)
else:
    output = ['size < 6       :', '6 < size < 10  :', 'size > 10      :']
    for proc in range(1, num_proc):
        temp = comm.recv(source=proc, tag=2)
        for i in range(0, 3):
            res[i] += temp[i]
    print('Resultado final')
    print(':::::::::::::::::::::::::::::::::::::')
    for i in range(0,3):
        print(f':: {output[i]} {res[i]}')
    print(':::::::::::::::::::::::::::::::::::::')