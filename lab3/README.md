Laboratório 3 de PSPD.
Tema: Construindo aplicações distribuídas usando MPI

Alunos:
|Matrícula | Aluno |
| -- | -- |
| 180132385 | Vitor Diniz Pagani Vieira Ribeiro |
| 190058650 | Natanael Fernandes Coelho Filho |

Esse laboratório tem como objetivo a compreensão das características inerentes à construção de aplicações paralelas, envolvendo comunicação por passagem de mensagens, via padrão MPI.

## Enunciado:

Os alunos devem criar uma aplicação MPI para ler um arquivo com mais de 10
milhões de caracteres, e identificar (i) a quantidade de palavras com menos de 6
caracteres, (ii) a quantidade de palavras que tem entre 6 e 10 caracteres, e (iii) a
quantidade de palavras com mais de 10 caracteres. 

Para rodar a aplicação, Insira o caminho do arquivo que será lido, como por exemplo:

```shell
# Exemplo rodando arquivo tamanho medio 
mpiexec -n 4 python3 ./files/med.txt
```
Obs: o argumento -n é correspondente ao número de workers desejado.

## Descrição da solução:

A solução foi desenvolvida em python, utilizando a biblioteca `mpi4py`, para executa-lo é necessário ter o python3 instalado e execute o comando `make dep` para instalar a biblioteca em sua máquina.

Com a biblioteca instalada basta executar o arquivo `main.py`, com o comandos correto, sendo ele como exemplo `mpiexec -n 4 python3 main.py ./files/big.txt`, onde temos 4 processos é o arquivo que será análisado é o big.txt, que possui mais de 10 milhoes de bytes.

Explicando melhor a solução desenvolvida temos o fluxograma a baixo: 

[fluxograma do funciomento do algoritmo](./img/funcionamento.png)

É definido como processo mestre o processo que possui o **rank** 0, ele será responsável por fazer a leitura do arquivo e destribui-lo entre os workers. Usando de exemplo o caso onde tempos 4 workers, e o arquivo lido terá 100 bytes, o mestre irá ler o arquivo, dividi-lo em 4, logo serão 25 bytes para cara worker, 25 bytes para cada um dos escravos, ou seja, worker 1, 2 e 3 receberão uma parte da mensagem, enquanto o mestre também fica com 25 bytes da mensagem.

Após, cada um dos processos faz as contabilizações relacionadas ao problema proposto, e os processos que não são o mestre enviam os resultados obtidos para o processo mestre. Ao final, processo mestre junta as respostas obtidas e mostra o resultado final para o usuário.

## Opinião Geral:

Vitor:
Achei esse experimento bem interessante, pois ao mesmo que é possível criar e testar aplicações distribuídas usando o padrão MPI, o experimento permite que a gente percebe algumas limitações que essa abordagem possui.

Natanael:
Tive uma esperiencia bem frustrante com esse trabalho, tentei de varias formas em fazer a aplicacao rodar em C, entretanto nao conseguia entender corretamente como funcionava, optamos em fazer em python apenas para demonstrar o quao poderoso pode ser o MPI, embora nao vi resultados satisfatorios rodando o projeto, talvez a forma como foi implementado nao foi satisfatoria, o que acarretou em um pior resultado em execuções com maior numero de processos.

## Tabela p/ Comparação:

Voce pode observar o print de execução das imagens salvas no diretorio `img`.

|nº workers | Tempo de execução |
| -- | -- |
| 4 | 1.758s |
| 6 | 1.805 |
| 8 | 2.085 |