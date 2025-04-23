sudo sysctl kernel.unprivileged_userns_clone=1
make clean
make

./ccrun run echo Hello, world!
echo $?
./ccrun run ls asdf
echo $?

hostname
./ccrun run hostname
echo $?

./ccrun run /bin/busybox sh
