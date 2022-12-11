#include <glib.h>
#include <librdkafka/rdkafka.h>

/* Optional per-message delivery callback (triggered by poll() or flush())
 * when a message has been successfully delivered or permanently
 * failed delivery (after retries).
 */
static void dr_msg_cb (rd_kafka_t *kafka_handle, const rd_kafka_message_t *rkmessage, void *opaque) {
    if (rkmessage->err) {
        g_error("Message delivery failed: %s", rd_kafka_err2str(rkmessage->err));
    }
}

static void load_config_group(rd_kafka_conf_t *conf, GKeyFile *key_file, const char *group) {
    char errstr[512];
    g_autoptr(GError) error = NULL;

    gchar **ptr = g_key_file_get_keys(key_file, group, NULL, &error);
    if (error) {
        g_error("%s", error->message);
        exit(1);
    }

    while (*ptr) {
        const char *key = *ptr;
        g_autofree gchar *value = g_key_file_get_string(key_file, group, key, &error);

        if (error) {
            g_error("Reading key: %s", error->message);
            exit(1);
        }

        if (rd_kafka_conf_set(conf, key, value, errstr, sizeof(errstr))
            != RD_KAFKA_CONF_OK
            ) {
            g_error("%s", errstr);
            exit(1);
        }

        ptr++;
    }
}