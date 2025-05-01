# sudo apt install libcurl4-openssl-dev
# cd jansson-2.14.1 && ./configure --prefix=/usr && make && sudo make install
sudo sysctl kernel.unprivileged_userns_clone=1

make clean
rm -rf /tmp/rootfs
make

echo -e "TEST BASIC"
./ccrun run alpine echo Hello, world!
echo $?

echo -e "\n\nTEST ERROR"
./ccrun run alpine ls asdf
echo $?

echo -e "\n\nTEST HOSTNAME"
hostname
./ccrun run alpine hostname
echo $?

echo -e "\n\nTEST SHELL"
./ccrun run alpine /bin/busybox sh
# strace -f ./ccrun run /bin/busybox sh
