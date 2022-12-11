#!/bin/bash

docker compose exec broker \
  kafka-topics --create \
    --topic names-count \
    --bootstrap-server localhost:9092 \
    --replication-factor 1 \
    --partitions 6

docker compose exec broker \
  kafka-topics --create \
    --topic names-count-result \
    --bootstrap-server localhost:9092 \
    --replication-factor 1 \
    --partitions 1