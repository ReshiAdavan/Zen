#include <signal.h>
#ifdef __linux__
#include <sys/eventfd.h>
#else
#include <unistd.h>
#endif

// Stops epoll_wait loops by sending an event
static void sigint_handler(int signum) {
    (void) signum;
    for (int i = 0; i < THREADSNR + 1; ++i) {
    #ifdef __linux__
        eventfd_write(conf->run, 1);
    #else
        (void) write(conf->run[0], &(unsigned long) {1}, sizeof(unsigned long));
    #endif
        usleep(1500);
    }
}

static const char *flag_description[] = {
    "Print this help",
    "Set a configuration file to load and use",
    "Specify a mode of usage, can be either http or tcp",
    "Specify a list of backends in the form of host:port:weight, weight is optional",
    "Specify the load-balancing type, between [round-robin, random, hash, leastconn, weighted-round-robin]",
    "Enable all logs, setting log level to DEBUG",
    "Run in daemon mode"
};

void print_help(char *me) {
    printf("\nzen v%s ZEN\n\n", VERSION);
    printf("Usage: %s [-c conf] [-b backends..] [-l load-balancing]"
           " [-m mode] [-v|-d|-h]\n\n", me);
    const char flags[7] = "hcmblvd";
    for (int i = 0; i < 7; ++i)
        printf(" -%c: %s\n", flags[i], flag_description[i]);
    printf("\n");
}

int main (int argc, char **argv) {

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

     /* Initialize random seed */
    srand((unsigned) time(NULL));

    char *confpath = DEFAULT_CONF_PATH;
    char *backends = NULL, *load_balancing = NULL, *mode = NULL;
    int debug = 0, daemon = 0;
    int opt;

    while ((opt = getopt(argc, argv, "c:b:l:m:vhd:")) != -1) {
        switch (opt) {
            case 'c':
                confpath = optarg;
                break;
            case 'b':
                backends = optarg;
                break;
            case 'l':
                load_balancing = optarg;
                break;
            case 'm':
                mode = optarg;
                break;
            case 'v':
                debug = 1;
                break;
            case 'd':
                daemon = 1;
                break;
            case 'h':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Usage: %s [-c conf] -b [backends..][-vhd]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Set default configuration
    config_set_default();

    // Override default DEBUG mode
    conf->loglevel = debug == 1 ? DEBUG : WARNING;

    conf->mode = (mode && STREQ(mode, "tcp", 3)) ? ZEN_TCP_MODE : ZEN_HTTP_MODE;

    // Try to load a configuration, if found
    if (!config_load(confpath)) {
        if (!backends) {
            print_help(argv[0]);
            exit(EXIT_FAILURE);
        }
        char *end_str;
        char *token = strtok_r((char *) backends, ",", &end_str);
        if (!token) {
            PARSE_CONFIG_COMMAS(backends,
                                &conf->backends[conf->backends_nr],
                                struct backend);
            conf->backends[conf->backends_nr].active_connections =
                ATOMIC_VAR_INIT(0);
        } else {
            do {
                if (conf->backends_nr >= conf->max_backends_nr) {
                    conf->max_backends_nr *= 2;
                    conf->backends =
                        zen_realloc(conf->backends,
                                    conf->max_backends_nr * sizeof(struct backend));
                }
                PARSE_CONFIG_COMMAS(token,
                                    &conf->backends[conf->backends_nr++],
                                    struct backend);
                conf->backends[conf->backends_nr-1].active_connections =
                    ATOMIC_VAR_INIT(0);
            } while ((token = strtok_r(NULL, ",", &end_str)));
        }
        /*
         * If load-balancing is specifyed set it, disk config has precedence
         * anyway on this
         */
        if (load_balancing) {
            if (STREQ("round-robin", load_balancing, 11)
                || STREQ("roundrobin", load_balancing, 10))
                conf->load_balancing = ROUND_ROBIN;
            else if (STREQ("hash", load_balancing, 4))
                conf->load_balancing = HASH_BALANCING;
            else if (STREQ("random", load_balancing, 6))
                conf->load_balancing = RANDOM_BALANCING;
            else if (STREQ("leastconn", load_balancing, 9))
                conf->load_balancing = LEASTCONN;
            else if (STREQ("leasttraffic", load_balancing, 12))
                conf->load_balancing = LEASTTRAFFIC;
            else if (STREQ("weighted-round-robin", load_balancing, 20))
                conf->load_balancing = WEIGHTED_ROUND_ROBIN;
            else
                log_warning("WARNING: Unsupported load-balancing algorithm, "
                            "fallbacking to round-robin");
        }
    }

    zen_log_init(conf->logpath);

    if (daemon == 1)
        daemonize();

    // Print configuration
    config_print();

    start_server(conf->frontends, conf->frontends_nr);

    zen_log_close();
    config_unload();

    return 0;
}
