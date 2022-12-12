#!/bin/bash

docker compose exec broker \
  kafka-topics --create \
    --topic count-words \
    --bootstrap-server localhost:9092 \
    --replication-factor 1 \
    --partitions 3

docker compose exec broker \
  kafka-topics --create \
    --topic count-words-results \
    --bootstrap-server localhost:9092 \
    --replication-factor 1 \
    --partitions 3