## Dependencias de instalacao do projeto

- docker instalado.
- dependencias do C:

```shell
sudo apt install librdkafka-dev -y
sudo apt install pkg-config -y
sudo apt-get install libglib2.0-dev
```

## Comandos de configuracao do projeto

- construa os binarios e as configurações necessárias:

```shell
make start
make broker
```

Caso tenha problemas com configuracao do docker faca:

```shell
# talvez seja necessario acesso root (sudo)
docker compose up -d --wait
./make_topics.sh
```

## Rodando o projeto

```shell
# com os binarios ativos execute

# executando consumidor
./consumer config.ini
# config.ini = arquivo de  configuracoes padroes do projeto

# executando producer
./producer config.ini file_name.txt num_msgs
# config.ini = arquivo de  configuracoes padroes do projeto
# file_name.txt = nome do arquivo que sera enviado
# num msgs (integer) = parametro opcional, onde define quantas mensagens seram enviadas pelo producer
```

Apos isso, projeto devera rodar e retornar:

- quantidade de palavras no arquivo.
- quantidade de palavras com menos de 6 caracteres.
- quantidade de palavras com 6 a 10 caracteres.

## Removendo arquivos indesejaveis

```shell
# remove todos os binarios gerados 
make rm
```

## Exemplo de execucao

Terminal do consumer: 

```shell
# pressuponde que ja tenha feito as configuracoes do projeto
./consumer config.ini 
```

Terminal do producer:

```shell
# pressuponde que ja tenha feito as configuracoes do projeto
./producer config.ini test2.txt 4

# Poderia ser tambem, enviando apenas uma mensagem
# ./producer config.ini test2.txt 
```

Exemplo de resposta esperada:

```
** Message: 22:13:59.203: Total de palavras: 14
** Message: 22:13:59.203: Total de palavras menores de 6 caracteres: 2
** Message: 22:13:59.203: Total de palavras entre 6 e 10 caracteres 1
```