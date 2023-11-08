# Zen

My Load Balancer written in C.

### Inspiration

- I am looking to expand my knowledge in distributed systems, one project at a time. So far I covered Sentinel, a distributed KV storage which covered consensus algorithms, distributed networking, etc.
- I am looking to create a load balancer and database from scratch and maybe create a large-scale distributed system leveraging these components.

### Topics

- Languages: C
- Concepts:
  - Techniques: Multi-threading, HTTP and TCP load-balancing
  - Algorithms: (Weighted) Round-Robin, Hash/Random-Balancing, Least Connections/Traffic,
  - Networks: TLS encryption
  - Other: Logging, Daemon mode

### Build

**Dependencies**

- gcc, make, cmake, OpenSSL (libssl-dev)

```sh
$ cmake . && make
```

### Quickstart

Covers a bit of architecture...not that it is that verbose.

Backend servers at http://localhost:8080 and http://localhost:8081, balancing strategy (weighted)-Round-Robin

```sh
$ ./zen -v -b http://localhost:8080:2,http://localhost:8081:4 -l weighted-round-robin
```

Simple config can be passed with `-c`:

```sh
$ ./zen -c path/to/zen.conf
```

Config is very small and self-explanatory, default path located to `lib.conf`:

```sh
# zen configuration file, uncomment and edit desired configuration

# accepts http | tcp
mode http

# Load-balancing settings

# Frontends are the endpoint exposed as entry point for connecting clients
frontends 127.0.0.1:8789,127.0.0.1:8790,127.0.0.1:8791

# Backends is a pool of server to load-balance requests from clients
backends 127.0.0.1:6090,127.0.0.1:6090,127.0.0.1:6090

# Set round robin as balancing algorithm
# Other options available can be
# random-balancing | hash-balancing | leastconn | leasttraffic | weighted-round-robin
load_balancing round-robin

# Logging configuration

# Could be either DEBUG, INFO/INFORMATION, WARNING, ERROR
log_level DEBUG

log_path /tmp/zen.log

# TCP backlog, size of the complete connection queue
tcp_backlog 128

# TLS certs

#cafile certs/ca.crt
#certfile certs/alaptop.crt
#keyfile certs/alaptop.key

# TLS supported versions
#tls_protocols tlsv1,tlsv1_1,tlsv1_2,tlsv1_3
```

Executable accepts simple args, in case of config overwriting it.

- `-h`: print an usage help
- `-v`: enable verbose debug messages
- `-d`: run in daemon mode
- `-c <path/to/conf>`: specify a configuration path, defaults to `/etc/zen/zen.conf`
- `-l <load-balancing-algorithm>`: specify a load-balancing algorithm to be
  used. Choices are `round-robin`, `hash`, `random`, `leastconn`, `leasttraffic`
  and `weighted-round-robin`
- `-m <mode>`: choose the run mode between `tcp` and `http`. Defaults to `http`
- `-b <host:port:weight>,<host:port:weight>..`: add backend machines with a
  weight associated, it will be ignored for load-balancing algorithms other
  than `weighted-round-robin`
