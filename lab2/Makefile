CFLAGS=-Wall $(shell pkg-config --cflags glib-2.0 rdkafka)
LDLIBS=$(shell pkg-config --libs glib-2.0 rdkafka)

start: producer consumer

broker:
	docker compose up -d --wait
	./make_topics.sh

producer:
	gcc producer.c -o producer $(CFLAGS) $(LDLIBS)

consumer:
	gcc consumer.c -o consumer $(CFLAGS) $(LDLIBS)

example: producer
	./producer config.ini test.txt 0

rm:
	rm producer consumer