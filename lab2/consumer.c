#include "common.c"

static volatile sig_atomic_t run = 1;

/**
 * @brief Signal termination of program
 */
static void stop(int sig) {
    run = 0;
}

int main (int argc, char **argv) {
    rd_kafka_t *worker, *responser;
    rd_kafka_conf_t *conf, *res_conf;
    rd_kafka_resp_err_t err;
    char errstr[512];

    // Parse the command line.
    if (argc != 2) {
        g_error("Usage: %s <config.ini>", argv[0]);
        return 1;
    }

    // Parse the configuration.
    // See https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md
    const char *config_file = argv[1];

    g_autoptr(GError) error = NULL;
    g_autoptr(GKeyFile) key_file = g_key_file_new();
    if (!g_key_file_load_from_file (key_file, config_file, G_KEY_FILE_NONE, &error)) {
        g_error ("Error loading config file: %s", error->message);
        return 1;
    }

    // Load the relevant configuration sections.
    conf = rd_kafka_conf_new();
    load_config_group(conf, key_file, "default");
    load_config_group(conf, key_file, "worker");

    res_conf = rd_kafka_conf_new();
    load_config_group(res_conf, key_file, "default");

    // Install a delivery-error callback.
    rd_kafka_conf_set_dr_msg_cb(res_conf, dr_msg_cb);

    // Create the Consumer instance.
    worker = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!worker) {
        g_error("Failed to create new worker: %s", errstr);
        return 1;
    }
    rd_kafka_poll_set_consumer(worker);

    // Create the Producer instance.
    responser = rd_kafka_new(RD_KAFKA_PRODUCER, res_conf, errstr, sizeof(errstr));
    if (!responser) {
        g_error("Failed to create new producer: %s", errstr);
        return 1;
    }

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;
    res_conf = NULL;

    // Convert the list of topics to a format suitable for librdkafka.
    const char *topic = "count-words";
    rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, topic, RD_KAFKA_PARTITION_UA);

    // Subscribe to the list of topics.
    err = rd_kafka_subscribe(worker, subscription);
    if (err) {
        g_error("Failed to subscribe to %d topics: %s", subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(worker);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    // Install a signal handler for clean shutdown.
    signal(SIGINT, stop);

    const char *responseTopic = "count-words-results";
    const char *key = "response";

    int n = 0, n_l_six = 0, n_g_six = 0;
    // Start polling for messages.
    while (run) {
        rd_kafka_message_t *consumer_message;

        consumer_message = rd_kafka_consumer_poll(worker, 500);
        if (!consumer_message) {
            g_message("Waiting...");
            continue;
        }

        if (consumer_message->err) {
            if (consumer_message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
                /* We can ignore this error - it just means we've read
                 * everything and are waiting for more data.
                 */
            } else {
                g_message("Consumer error: %s", rd_kafka_message_errstr(consumer_message));
                return 1;
            }
        } else {
            g_message("Consumed event from topic %s: counting...", rd_kafka_topic_name(consumer_message->rkt));

            char *buffer = (char*) consumer_message->payload;
            //g_message("msg: '%s'", buffer);
            
            unsigned long long int pos_ant = 0;
            for(unsigned long long int i=0; i<=strlen(buffer); i++){
                if(buffer[i] == ' ' || buffer[i] == '\0'){
                    unsigned long long int size_w = i - pos_ant - 1;    

                    if(!size_w){
                        pos_ant = i+1;
                        continue;
                    }
                    else if(size_w < 6) n_l_six++;
                    else if(size_w < 10) n_g_six++;

                    pos_ant = i+1;
                    n++;
                }
            }

            g_message("%d palavras: %d menores que 6 bytes e %d entre 6 e 10 bytes.", n, n_l_six, n_g_six);
            int res[3] = {n, n_l_six, n_g_six};

            //publish results
            rd_kafka_resp_err_t err;      
            err = rd_kafka_producev(responser,
                                RD_KAFKA_V_TOPIC(responseTopic),
                                RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                                RD_KAFKA_V_KEY((void*)key, 3),
                                RD_KAFKA_V_VALUE((void*) res, 12),
                                RD_KAFKA_V_OPAQUE(NULL),
                                RD_KAFKA_V_END);

            if (err) {
                g_error("Failed to produce to topic %s: %s", responseTopic, rd_kafka_err2str(err));
                return 1;
            } else {
                g_message("Response sent to topic %s!", responseTopic);
            }

            rd_kafka_poll(responser, 0);

            n = 0, n_l_six = 0, n_g_six = 0;
        }

        // Free the message when we're done.
        rd_kafka_message_destroy(consumer_message);
    }

    // Close the worker: commit final offsets and leave the group.
    g_message( "Closing worker");
    rd_kafka_consumer_close(worker);

    // Destroy the worker.
    rd_kafka_destroy(worker);

    // Block until the messages are all sent.
    g_message("Flushing final messages..");
    rd_kafka_flush(responser, 10 * 1000);

    if (rd_kafka_outq_len(responser) > 0) {
        g_error("%d message(s) were not delivered", rd_kafka_outq_len(responser));
    }

    rd_kafka_destroy(responser);

    return 0;
}