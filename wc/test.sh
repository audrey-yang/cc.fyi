go build

echo "\n*****\nTEST WITH FILE ARG"
echo "\n-c"
./ccwc -c test.txt
wc -c test.txt

echo "\n-l"
./ccwc -l test.txt
wc -l test.txt

echo "\n-w"
./ccwc -w test.txt
wc -w test.txt

echo "\n-m"
./ccwc -m test.txt
wc -m test.txt

echo "\nno flags"
./ccwc test.txt
wc test.txt

echo "\n*****\nTEST WITH INPUT STREAM"
echo "\n-c"
cat test.txt | ./ccwc -c
cat test.txt | wc -c

echo "\n-l"
cat test.txt | ./ccwc -l
cat test.txt | wc -l

echo "\n-w"
cat test.txt | ./ccwc -w
cat test.txt | wc -w

echo "\n-m"
cat test.txt | ./ccwc -m
cat test.txt | wc -m

echo "\nno flags"
cat test.txt | ./ccwc
cat test.txt | wc