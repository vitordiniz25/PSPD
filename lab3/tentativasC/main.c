#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <sys/types.h>
#include <unistd.h>

#define OUTPUT 4
#define NONWORDS " .,;!\n\t"
#define MAX_LINES 34000 // define the maximum number of lines we support.
#define MAX_LINE_WIDTH 1536 // define the maximum number of characters per line we support.

typedef unsigned long long int ullong;
/*
	se processo mestre
		1. Pegar conteudo do arquivo passado no argv
		2. Pegar quantidade total de bytes do arquivo

		3. Dividir o arquivo em N mensagens, onde N eh igual ao numero de workers(escravos)
			N = size - 1; // visto que 1 dos processos eh responsavel
		4. Enviar os arquivos divididos para os escravos
		5. Enviar arquivo para os escravos
		6. Receber respostas de todos os escravos
		7. Juntar os resultados em variaveis separadas
		8. Printar resposta na tela
	se processo escravo
		1. Receber mensagem do processo mestre
		2. Contabilizar as palavras (OK)
			Total de palavras
			Total de palavras menores de 6 caracteres
			Total de palavras entre 6 a 10 caracteres
		3. Enviar para o mestre

		*/

int *word_counter(char *s);
FILE *open_file(char *path);

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Insira arquivo desejado para leitura, exemplo com 4 processos.\nmpirun -n 4 ./%s milhao.txt", argv[1]);
		return 1;
	}
	MPI_Init(&argc, &argv);
	int rank; // rank dos projessos
	int size; // armazena quantidade de processos
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	char maquina[200];
	gethostname(maquina, 199);

	char (*buffer_str)[MAX_LINE_WIDTH] = (int *) calloc(MAX_LINES, sizeof(int));
	int last_line=0;
	ullong file_size;

	if (rank == 0)
	{
		FILE *f = open_file(argv[1]);
		if(f == NULL) {
			printf("Nao foi possivel ler o arquivo\n");
			return 1;
		}
		int line =0;
		while (!feof(file) && !ferror(file)) {
			fgets(da)
			strcpy(buffer_str, lineBuffer); // copy line into our buffer
		}
		int message_count = size;

		fclose(f); // Close the file.
	}

	else
	{
		// escravos
		int recebido;

		printf("Escravo(%d/%s) recebeu %d\n", rank, maquina, recebido);
	}

	MPI_Finalize();
}

FILE *open_file(char *path)
{
	FILE *f = fopen(path, "r");
	if (!f)
	{
		return NULL;
	}
	return f;
}

int *word_counter(char *s)
{
	int len = strlen(s);
	int word_size = 0;
	int *res = calloc(OUTPUT, sizeof(int));

	for (int i = 0; i < len; i++)
	{
		word_size = 0;
		while (i < len)
		{ // Loop continua enquanto nao for achado um caracter especial
			if (strchr(NONWORDS, s[i]) != NULL)
			{
				break;
			}
			word_size++;
			i++;
		}
		res[0]++;
		if (word_size < 6)
			res[1]++;
		else if (word_size < 10)
			res[2]++;
		else
			res[3]++;
		while (i < len)
		{ // Loop continua enquanto somente for achado caracteres especiais
			if (strchr(NONWORDS, s[i]) == NULL)
			{
				break;
			}
			word_size++;
			i++;
		}
	}
	return res;
}