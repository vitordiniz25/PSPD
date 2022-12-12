CFLAGS=-Wall $(shell pkg-config --cflags glib-2.0 rdkafka)
LDLIBS=$(shell pkg-config --libs glib-2.0 rdkafka)

start: producer consumer

broker:
	docker compose up -d --wait
	./create_topics.sh

producer:
	gcc producer.c -o producer $(CFLAGS) $(LDLIBS)

consumer:
	gcc consumer.c -o consumer $(CFLAGS) $(LDLIBS)

file_gen:
	gcc file_gen.c -o file_gen

example: producer file_gen
	./file_gen 20 30 40

rm:
	rm producer consumer file_gen test.txt