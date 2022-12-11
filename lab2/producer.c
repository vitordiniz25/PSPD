#include "common.c"
#include <unistd.h>
#include <sys/time.h>

#define ARR_SIZE(arr) ( sizeof((arr)) / sizeof((arr[0])) )

double calc_time(__time_t sec_ini, __time_t sec_end, __suseconds_t usec_ini, __suseconds_t usec_end){
	return (sec_end + (double) usec_end/1000000) - (sec_ini + (double) usec_ini/1000000);
}

int main (int argc, char **argv) {
    rd_kafka_t *producer, *solver;
    rd_kafka_conf_t *conf, *solver_conf;
    rd_kafka_resp_err_t err;
    char errstr[512];

    // Parse the command line.
    if (argc != 4) {
        g_error("Usage: %s configFile.ini fileName.txt num_msgs (default 0)", argv[0]);
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

    solver_conf = rd_kafka_conf_new();
    load_config_group(solver_conf, key_file, "default");
    load_config_group(solver_conf, key_file, "solver");

    // Install a delivery-error callback.
    rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);

    if (rd_kafka_conf_set(conf, "partitioner", "random", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK){
            g_error("%s", errstr);
            return 1;
    }

    // Create the Producer instance.
    producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!producer) {
        g_error("Failed to create new producer: %s", errstr);
        return 1;
    }

    solver = rd_kafka_new(RD_KAFKA_CONSUMER, solver_conf, errstr, sizeof(errstr));
    if (!solver) {
        g_error("Failed to create new solver: %s", errstr);
        return 1;
    }
    rd_kafka_poll_set_consumer(solver);

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;
    solver_conf = NULL;

     // Convert the list of topics to a format suitable for librdkafka.
    const char *responseTopic = "names-count-result";
    rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, responseTopic, RD_KAFKA_PARTITION_UA);

    // Subscribe to the list of topics.
    err = rd_kafka_subscribe(solver, subscription);
    if (err) {
        g_error("Failed to subscribe to %d topics: %s", subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(solver);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    
    FILE *names = fopen(argv[2], "r");

    int message_count = atoi(argv[3]);

    message_count = (!message_count) ? 1 : message_count;

  	if(!names){
        g_error("Can't onpen the file: %s", argv[2]);
  		return 1;
	}

    // Define o tamanho total do arquivo
    fseek(names, 0, SEEK_END);
    unsigned long long int file_size = ftell(names);
    fseek(names, 0, SEEK_SET);

    struct timeval total_ini, total_end;
    unsigned long long int n = 0, n_l_six = 0, n_g_six = 0, pos_ant = 0;

    const char *topic = "names-count";
    const char *key =  "msg";

    gettimeofday(&total_ini, NULL);
    for (unsigned long long int i = 0; i < message_count; i++) {

        //Divide o arquivo conforme a quantidade de msgs
        // TO DO: resolver o problema do lixo de memória em msg vazias
        
        unsigned long long int relative_pos = pos_ant + file_size/message_count;

        relative_pos = (relative_pos > file_size) ? file_size : relative_pos;

        fseek(names, relative_pos, SEEK_SET);

        if(relative_pos != file_size){
            int k=0;
            while(!feof(names) && fgetc(names) != ' ') k++;
            relative_pos+=k;
        }        

        unsigned long long int buffer_size = relative_pos - pos_ant;

        char *buffer = malloc(buffer_size + 1);

        fseek(names, pos_ant, SEEK_SET);

        for(unsigned long long int j = 0; j < buffer_size; j++){
            buffer[j] = fgetc(names);
        }

        buffer[buffer_size] = '\0';     
        
        pos_ant = ftell(names) + 1;

        //g_message("msg: '%s'", buffer);

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
        } else {
            g_message("Produced event to topic %s: mensage %llu sent!", topic, i+1);
        }

        rd_kafka_poll(producer, 0);
    }

    for(int i = 0; i < message_count; ){
        rd_kafka_message_t *response;

        response = rd_kafka_consumer_poll(solver, 500);

        if (!response){
            g_message("Waiting Response...");
            continue;
        }

        if (response->err) {
            if (response->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
                /* We can ignore this error - it just means we've read
                 * everything and are waiting for more data.
                 */
            } else {
                g_message("Response error: %s", rd_kafka_message_errstr(response));
                return 1;
            }
        } else {
            int *res = response->payload;
            n += res[0];
            n_l_six += res[1];
            n_g_six += res[2];
            i++;
        }

        // Free the message when we're done.
        rd_kafka_message_destroy(response);
    }
    gettimeofday(&total_end, NULL);

    fclose(names);

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

    double duration = calc_time(total_ini.tv_sec, total_end.tv_sec, total_ini.tv_usec, total_end.tv_usec);

    g_message("[%.8lf s] Total de %llu palavras: %llu menores de 6 bytes e %llu entre 6 e 10 bytes", duration, n, n_l_six, n_g_six);

    return 0;
}