Laboratório 3 de PSPD.
Tema: Construindo aplicações distribuídas usando MPI

Alunos:
|Matrícula | Aluno |
| -- | -- |
| 180132385 | Vitor Diniz Pagani Vieira Ribeiro |
| 190058650 | Natanael Fernandes Coelho Filho |

Esse laboratório tem como objetivo a compreensão das características inerentes à construção de aplicações paralelas, envolvendo comunicação por passagem de mensagens, via padrão MPI.

Enunciado:

Os alunos devem criar uma aplicação MPI para ler um arquivo com mais de 10
milhões de caracteres, e identificar (i) a quantidade de palavras com menos de 6
caracteres, (ii) a quantidade de palavras que tem entre 6 e 10 caracteres, e (iii) a
quantidade de palavras com mais de 10 caracteres. 

Para rodar a aplicação, Insira o caminho do arquivo que será lido, como por exemplo:

`
mpiexec -n 4 python ./files/med.txt
`
Obs: o argumento -n é correspondente ao número de workers desejado.

Descrição da solução:

Opinião Geral:

Vitor:
Achei esse experimento bem interessante, pois ao mesmo que é possível criar e testar aplicações distribuídas usando o padrão MPI, o experimento permite que a gente percebe algumas limitações que essa abordagem possui.

Tabela p/ Comparação:

|nº workers | Tempo de execução |
| -- | -- |
| 2 |  |
| 4 |  |
| 6 |  |
| 8 |  |