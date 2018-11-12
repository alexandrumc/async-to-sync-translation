gcc -fpreprocessed -dD -E  -P $1 > test.c
sed '/^#include/ d' test.c > $1
rm test.c
