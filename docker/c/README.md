# Docker

Following the [Build Your Own Docker](https://codingchallenges.fyi/challenges/challenge-docker) challenge.

## How to run
Compile the executable with `make ccrun` and run `./ccrun run <image> <command> <args>`.

### Dependencies
This project depends on [libcurl](https://curl.se/libcurl/) and [jansson](https://jansson.readthedocs.io/en/latest/).

### Environment
The manipulation of namespaces demands a Linux environment. This project was developed with [Vagrant](https://developer.hashicorp.com/vagrant), specifically with Stretch on [VirtualBox](https://www.virtualbox.org/).
