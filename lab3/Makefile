dep:
	sudo apt install python3
	sudo apt install python3-pip
	python3 -m pip install mpi4py
	
run4:
	mpiexec -n 4 python3 main.py ./files/big.txt 

run6: 
	mpiexec -n 6 python3 main.py ./files/big.txt

run8: 
	mpiexec -n 8 python3 main.py ./files/big.txt

choco:
	mpiexec -hosts chococino,cm1,cm2,cm3 -n 4 python3 main.py ./files/big.txt

