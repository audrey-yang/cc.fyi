# sudo apt install libcurl4-openssl-dev
# cd jansson-2.14.1 && ./configure --prefix=/usr && make && sudo make install
sudo sysctl kernel.unprivileged_userns_clone=1

make clean
rm -rf /tmp/rootfs
make

# ./ccrun run echo Hello, world!
# echo $?
# ./ccrun run ls asdf
# echo $?

# hostname
# ./ccrun run hostname
# echo $?
./ccrun run /bin/busybox sh
# strace -f ./ccrun run /bin/busybox sh
