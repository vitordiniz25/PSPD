// #include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define NONWORDS " .,;!\n\t"
#define OUTPUT 4

/*
The purpose of this tutorial is to show you how to transfer large amounts of data between threads.

The goal of this program is to let the user input a string, then count how many times that string appears in a text file.
For the purposes of this tutorial, the text file will be the bible, which is large enough to justify parallel computation.
It is also free to download, which is also very nice. At Supercomputingblog.com, I encourage completely free developement
environments, tools, and tutorials.
*/

#define MAX_NUM_LINES 34000 // define the maximum number of lines we support.
#define MAX_LINE_WIDTH 1536 // define the maximum number of characters per line we support.
#define MASTER 0

int *word_counter(char *s);

int main(int argc, char **argv)
{

	if (argc < 2)
	{
		printf("Insira caminho arquivo desejado para leitura.\ncaso queira um exemplo rode: make ex4\nEle rodara: mpirun -n 4 ./lab3 ./files/big.txt\n");
		return 1;
	}

	int maxNum;

	char searchWord[80] = "ola";
	int num_proc, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// When creating out buffer, we use the new operator to avoid stack overflow.
	// If you are programming in C instead of C++, you'll want to use malloc.
	// Please note that this is not the most efficient way to use memory.
	// We are sacrificing memory now in order to gain more performance later.

	int *lineStartIndex = calloc(MAX_NUM_LINES, sizeof(int)); // keep track of which lines start where. Mainly for debugging
	char *fileBuffer = calloc(MAX_NUM_LINES * MAX_LINE_WIDTH, sizeof(char));
	int totalLines = 0;
	int hasData = 0; // This helps us determine which thread read the file.
	// Which thread is the master
	int master = 0;			  // which thread gets user input? Either 0 or 1 :::: talvez poda
	int searchWordLength = 3; // vai poda

	if (rank == MASTER)
	{
		printf("Master Here! Thread (%d) is reading file\n", MASTER);
		printf("Number of threads = %d\n", num_proc);
		printf("Master Here! the word searched was: %s\n", argv[2]);
		// File I/O often takes as much time, or more time than the actual computation.
		// If you would like some exercise, modify this program so that one thread will
		// get user input, and another thread will read the file. Remember, the program
		// may be instantiated with 1 or more processors.

		char lineBuffer[MAX_LINE_WIDTH];

		FILE *file = fopen(argv[1], "rt"); // Open the file

		lineStartIndex[0] = 0;
		while (fgets(lineBuffer, MAX_LINE_WIDTH, file) != NULL)
		{
			strcpy(fileBuffer + lineStartIndex[totalLines], lineBuffer); // copy line into our buffer

			int length = strlen(lineBuffer);
			totalLines++;
			// store where the next line will start
			lineStartIndex[totalLines] = lineStartIndex[totalLines - 1] + length + 1;
		}
		printf("file size%ld", strlen(fileBuffer));
		fclose(file); // Close the file.
		hasData = 1;  // This thread read the data, and thus, has the data.
	}

	// Because all threads will need to know what we're searching for,
	// thread zero will have to broadcast that data to all threads.
	// Unlike the previous tutorial, we will be sending a string instead of an integer.
	// Notice how the second parameter is the length of the word + 1. This is because
	// We need to account for the '\0' at the end of the string.

	// Threads do not know how long the search word is, so we have to broadcast that first
	// Alternatively, we could just send all 80 possible characters. It's a tradeoff that deserves experimentation.

	MPI_Bcast(&searchWordLength, 1, MPI_INT, master, MPI_COMM_WORLD);

	// Now receive the Word. We're adding 1 to the length to allow for NULL termination.
	MPI_Bcast(searchWord, searchWordLength + 1, MPI_CHAR, master, MPI_COMM_WORLD);

	// All threads now know what word we're searching for.

	// Thread zero needs to distribute data to other threads.
	// Because this is a relatively large amount of data, we SHOULD NOT send the entire dataset to all threads.
	// Instead, it's best to intelligently break up the data, and only send relevant portions to each thread.
	// Data communication is an expensive resource, and we have to minimize it at all costs.
	// This is a key concept to learn in order to make high performce applications.

	int totalChars = 0;
	int portion = 0;
	int startNum = 0;
	int endNum = 0;

	// adicionar tratamento para ir ate o fim da palavra
	if (rank == MASTER)
	{
		totalChars = lineStartIndex[totalLines];

		portion = totalChars / num_proc;
		printf("part %d\n", portion);
		startNum = 0;

		endNum = portion;
		totalChars = endNum - startNum;

		for (int i = 1; i < num_proc; i++)
		{
			// calculate the data for each thread.
			int curStartNum = i * portion - searchWordLength + 1;
			int curEndNum = (i + 1) * portion;
			if (i == num_proc - 1)
			{
				curEndNum = lineStartIndex[totalLines] - 1;
			}
			if (curStartNum < 0)
			{
				curStartNum = 0;
			}

			// we need to send a thread the number of characters it will be receiving.
			int curLength = curEndNum - curStartNum;
			MPI_Send(&curLength, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
			MPI_Send(fileBuffer + curStartNum, curLength, MPI_CHAR, i, 2, MPI_COMM_WORLD);
		}
	}
	else
	{
		// We are not the thread that read the file.
		// We need to receive data from whichever thread
		MPI_Status status;
		MPI_Recv(&totalChars, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(fileBuffer, totalChars, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);

		portion = totalChars;
		startNum = rank * portion;
		endNum = (rank + 1) * portion;
		// Thread 0 is responsible for making sure the startNum and endNum calculated here are valid.
		// This is because thread 0 tells us exactly how many characters we were send.
	}

	// Do the search
	int total=0, six = 0, six_ten = 0, ten = 0;
	int curIndex = 0;
	int len = endNum - startNum;
	int word_size = 0;

	printf("rank: %d ", rank);
	while(curIndex < endNum-startNum)
	{
		// check to see if the current letter is the start of the match word
		int match = 1;
		for (int i=0; i < searchWordLength; i++)
		{
			if (fileBuffer[curIndex+i] != searchWord[i]) {match = 0; i = searchWordLength;}
		}
		if (match == 1) total++;
		curIndex++;

	}

	printf("Thread %d counted (total)         : %d\n", rank, total);
	printf("Thread %d counted (size < 6)      : %d\n", rank, six);
	printf("Thread %d counted (6 < size < 10) : %d\n", rank, six_ten);
	printf("Thread %d counted (size > 10)     : %d\n", rank, ten);

	// At this point, all threads need to communicate their results to thread 0.

	if (rank == MASTER)
	{
		// The master thread will need to receive all computations from all other threads.
		MPI_Status status;

		// MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
		// We need to go and receive the data from all other threads.
		// The arbitrary tag we choose is 1, for now.
		for (int i = 1; i < num_proc; i++)
		{
			int temp1, temp2, temp3, temp4;
			MPI_Recv(&temp1, 1, MPI_INT, i, 3, MPI_COMM_WORLD, &status);
			MPI_Recv(&temp2, 1, MPI_INT, i, 3, MPI_COMM_WORLD, &status);
			MPI_Recv(&temp3, 1, MPI_INT, i, 3, MPI_COMM_WORLD, &status);
			MPI_Recv(&temp4, 1, MPI_INT, i, 3, MPI_COMM_WORLD, &status);
			total += temp1;
			six += temp2;
			six_ten += temp3;
			ten += temp4;
		}
	}
	else
	{
		// We are finished with the results in this thread, and need to send the data to thread 1.
		// MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
		// The destination is thread 0, and the arbitrary tag we choose for now is 1.
		MPI_Send(&total, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
		MPI_Send(&six, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
		MPI_Send(&six_ten, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
		MPI_Send(&ten, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
	}

	if (rank == MASTER)
	{
		// Display the final calculated value
		printf(":::::::::::::::::::::::::::::::::::::::::::::\n");
		printf("Final result\n");
		printf("---------------------------------------------\n");
		printf("Total words                        : %d\n", total);
		printf("Total words, where (size < 6)      : %d\n", six);
		printf("Total words, where (6 < size < 10) : %d\n", six_ten);
		printf("Total words, where (size > 10)     : %d\n", ten);
		printf(":::::::::::::::::::::::::::::::::::::::::::::\n");
	}

	MPI_Finalize();
	return 0;
}