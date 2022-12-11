CFLAGS=-Wall $(shell pkg-config --cflags glib-2.0 rdkafka)
LDLIBS=$(shell pkg-config --libs glib-2.0 rdkafka)



ALL: producer consumer generator

broker:
	docker compose up -d --wait
	./create_topics.sh

producer:
	gcc producer.c -o producer $(CFLAGS) $(LDLIBS)

consumer:
	gcc consumer.c -o consumer $(CFLAGS) $(LDLIBS)

generator:
	gcc gerador.c -o generator

clear:
	rm producer consumer generator