go build

echo STEP 1
echo --valid
./cc-json-parser tests/step1/valid.json
echo --invalid
./cc-json-parser tests/step1/invalid.json
echo

echo STEP 2
echo --valid
./cc-json-parser tests/step2/valid.json
echo --valid
./cc-json-parser tests/step2/valid2.json
echo --invalid
./cc-json-parser tests/step2/invalid.json
echo --invalid
./cc-json-parser tests/step2/invalid2.json
echo

echo STEP 3
echo --valid
./cc-json-parser tests/step3/valid.json
echo --invalid
./cc-json-parser tests/step3/invalid.json
echo

echo STEP 4
echo --valid
./cc-json-parser tests/step4/valid.json
echo --valid
./cc-json-parser tests/step4/valid2.json
echo --invalid
./cc-json-parser tests/step4/invalid.json
