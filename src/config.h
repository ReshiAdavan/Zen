#ifndef CONFIG_H
#define CONFIG_H

#include <string.h>
#include <stdbool.h>

// Eventloop backend check
#ifdef __linux__
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 44)
#define EPOLL 1
#define EVENTLOOP_BACKEND "epoll"
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 1, 23)
#define POLL 1
#define EVENTLOOP_BACKEND "poll"
#else
#define SELECT 1
#define EVENTLOOP_BACKEND "select"
#endif

#elif defined(__APPLE__) || defined(__FreeBSD__) \
       || defined(__OpenBSD__) || defined (__NetBSD__)
#define KQUEUE 1
#define EVENTLOOP_BACKEND "kqueue"
#else
#define SELECT 1
#define EVENTLOOP_BACKEND "select"
#endif // __linux__

// TLS versions

#define ZEN_TLSv1       0x01
#define ZEN_TLSv1_1     0x02
#define ZEN_TLSv1_2     0x04
#define ZEN_TLSv1_3     0x08

// Default parameters

#define VERSION                     "0.1.7"
#define DEFAULT_MODE                ZEN_HTTP_MODE
#define DEFAULT_LOG_LEVEL           DEBUG
#define DEFAULT_LOG_PATH            "/var/log/zen.log"
#define DEFAULT_CONF_PATH           "/etc/zen/zen.conf"
#define DEFAULT_HOSTNAME            "127.0.0.1"
#define DEFAULT_PORT                8789
#define DEFAULT_LOAD_BALANCING      ROUND_ROBIN
#ifdef TLS1_3_VERSION
#define DEFAULT_TLS_PROTOCOLS       (ZEN_TLSv1_2 | ZEN_TLSv1_3)
#else
#define DEFAULT_TLS_PROTOCOLS       ZEN_TLSv1_2
#endif

#define STREQ(s1, s2, len) strncasecmp(s1, s2, len) == 0 ? true : false

#define PARSE_CONFIG_COMMAS(token, target, type) do {           \
    type *t = (type *) (target);                                \
    char *end_token;                                            \
    size_t toklen = strlen((token));                            \
    char tmp[toklen + 1];                                       \
    snprintf(tmp, toklen + 1, "%s", (token));                   \
    char *host = strtok_r(tmp, ":", &end_token);                \
    char *port = strtok_r(NULL, ":", &end_token);               \
    char *weight = strtok_r(NULL, ":", &end_token);             \
    if (weight != NULL) {                                       \
        t->weight = atoi(weight);                               \
    }                                                           \
    snprintf(t->host, strlen(host) + 1, "%s", host);            \
    t->port = atoi(port);                                       \
} while (0);

struct config {
    /* zen version <MAJOR.MINOR.PATCH> */
    const char *version;
    /* Eventfd to break the epoll_wait loop in case of signals */
#ifdef __linux__
    int run;
#else
    int run[2];
#endif // __linux__
    /* Logging level, to be set by reading configuration */
    int loglevel;
    /* Log file path */
    char logpath[0xFFF];
    /* List of frontends to listen on */
    struct frontend *frontends;
    int frontends_nr;
    int max_frontends_nr;
    /* List of backend to load-balance */
    struct backend *backends;
    int backends_nr;
    int max_backends_nr;
    /* TCP backlog size */
    int tcp_backlog;
    /* TLS flag */
    bool tls;
    /* TLS protocol version */
    int tls_protocols;
    /* Certificate authority file path */
    char cafile[0xFFF];
    /* SSL - Cert file location on filesystem */
    char certfile[0xFFF];
    /* SSL - Key file location on filesystem */
    char keyfile[0xFFF];
    /* Load-balancing algorithms */
    int load_balancing;
    /* Load balncing execution mode */
    int mode;
};

extern struct config *conf;

void config_set_default(void);
void config_print(void);
bool config_load(const char *);
void config_unload(void);

#endif
