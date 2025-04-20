go build

./ccrun run echo Hello, world!
echo $?
./ccrun run ls asdf
echo $?

hostname
./ccrun run hostname
echo $?