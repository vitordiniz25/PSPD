#include "common.c"
#include <unistd.h>

FILE * open_file(char *path) {
    FILE *f= fopen(path, "r");
    if(!f){
  		return NULL;
	}
    return f;
}

int main (int argc, char **argv) {
    // Verify arguments
    if (argc < 3 || argc > 4) {
        g_error("Usage: %s config.ini file_name.txt num_msgs(not necessary)\n", argv[0]);
        return 1;
    }

    char errstr[512];

    // Parse the configuration.
    // See https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md
    const char *config_file = argv[1];

    g_autoptr(GError) error = NULL;
    g_autoptr(GKeyFile) key_file = g_key_file_new();
    if (!g_key_file_load_from_file (key_file, config_file, G_KEY_FILE_NONE, &error)) {
        g_error ("Error loading config file: %s", error->message);
        return 1;
    }

    // Configuration sections for Producer.
    rd_kafka_conf_t *conf = rd_kafka_conf_new();
    load_config_group(conf, key_file, "default");

    // Install a delivery-error callback.
    rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);

    if (rd_kafka_conf_set(conf, "partitioner", "random", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK){
            g_error("%s", errstr);
            return 1;
    }

    // Make Producer instance.
    rd_kafka_t *producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!producer) {
        g_error("Failed to create new producer: %s", errstr);
        return 1;
    }

    rd_kafka_conf_t *solver_conf = rd_kafka_conf_new();
    load_config_group(solver_conf, key_file, "default");
    load_config_group(solver_conf, key_file, "solver");

    rd_kafka_t *solver = rd_kafka_new(RD_KAFKA_CONSUMER, solver_conf, errstr, sizeof(errstr));
    if (!solver) {
        g_error("Failed to create new solver: %s", errstr);
        return 1;
    }
    rd_kafka_poll_set_consumer(solver);

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;
    solver_conf = NULL;

     // Convert the list of topics to a format suitable for librdkafka.
    const char *responseTopic = "count-words-results";
    rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, responseTopic, RD_KAFKA_PARTITION_UA);

    // Subscribe to the list of Current number of topics.
    rd_kafka_resp_err_t err = rd_kafka_subscribe(solver, subscription);
    if (err) {
        g_error("Failed to subscribe to %d topics: %s", subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(solver);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    // Open file readonly
    FILE *file;
    if(!(file = open_file(argv[2]))) {
        g_error("Can't onpen the file: %s", argv[2]);
        return 1;
    }

    // Define message counter
    int message_count = argc == 3 ? 1: atoi(argv[3]);

    // Define o tamanho total do arquivo
    fseek(file, 0, SEEK_END);
    unsigned long long int file_size = ftell(file);
    // voltar a posicao inicial do arquivo
    fseek(file, 0, SEEK_SET);

    const char *topic = "count-words";
    const char *key =  "message";

    unsigned long long int pos_ant = 0;

    for (unsigned long long int i = 0; i < message_count; i++) {
        // Divide o arquivo conforme a quantidade de mensagens
        unsigned long long int relative_pos = pos_ant + file_size/message_count;

        relative_pos = (relative_pos > file_size) ? file_size : relative_pos;

        fseek(file, relative_pos, SEEK_SET);

        if(relative_pos != file_size){
            int k=0;
            while(!feof(file) && fgetc(file) != ' ') k++;
            relative_pos+=k;
        }        

        unsigned long long int buffer_size = relative_pos - pos_ant;

        char *buffer = malloc(buffer_size + 1);

        fseek(file, pos_ant, SEEK_SET);

        for(unsigned long long int j = 0; j < buffer_size; j++){
            buffer[j] = fgetc(file);
        }

        buffer[buffer_size] = '\0';     
        
        pos_ant = ftell(file) + 1;

        rd_kafka_resp_err_t err;      
        err = rd_kafka_producev(producer,
                                RD_KAFKA_V_TOPIC(topic),
                                RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                                RD_KAFKA_V_KEY((void*)key, 3),
                                RD_KAFKA_V_VALUE((void*)buffer, (size_t) strlen(buffer)),
                                RD_KAFKA_V_OPAQUE(NULL),
                                RD_KAFKA_V_END);

        if (err) {
            g_error("Failed to produce to topic %s: %s", topic, rd_kafka_err2str(err));
            return 1;
        }
        g_message("Produced event to topic %s: mensage %llu sent!", topic, i+1);
        rd_kafka_poll(producer, 0);
    }

    unsigned long long int total = 0, less_than_six = 0, between_six_ten = 0;

    for(int i = 0; i < message_count; ){
        rd_kafka_message_t *response = rd_kafka_consumer_poll(solver, 2000);
        if (!response){
            g_message("Waiting Response...");
            continue;
        }

        if (response->err) {
            if (response->err != RD_KAFKA_RESP_ERR__PARTITION_EOF) { // it just means we've read everything and are waiting for more data.
                g_message("Response error: %s", rd_kafka_message_errstr(response));
                return 1;
            }
        } else {
            int *res = response->payload;
            total += res[0];
            less_than_six += res[1];
            between_six_ten += res[2];
            i++;
        }

        // Free the message when we're done.
        rd_kafka_message_destroy(response);
    }

    fclose(file);

    // Block until the messages are all sent.
    g_message("Flushing final messages..");
    rd_kafka_flush(producer, 10 * 1000);

    if (rd_kafka_outq_len(producer) > 0) {
        g_error("%d message(s) were not delivered", rd_kafka_outq_len(producer));
    }

    g_message("%d events were produced to topic %s.", message_count, topic);

    rd_kafka_destroy(producer);

     // Close the solver: commit final offsets and leave the group.
    g_message( "Closing solver");
    rd_kafka_consumer_close(solver);

    // Destroy the solver.
    rd_kafka_destroy(solver);

    g_message("Total de palavras: %llu\n", total);
    g_message("Total de palavras menores de 6 caracteres: %llu", less_than_six);
    g_message("Total de palavras entre 6 e 10 caracteres %llu\n",between_six_ten);

    return 0;
}