echo "{}" > test.json

# dict read/write
../build/cmc set test.json /a/b/c abc
../build/cmc set test.json /A/B/C ABC

../build/cmc get test.json /a/b/c
../build/cmc get test.json /A/B/C

# save
../build/cmc save test.json
cat test.json

# array
../build/cmc set test.json /array[5] apple
../build/cmc save test.json
cat test.json
