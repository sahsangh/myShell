cd testdir
cd test_pipeline
echo "=== Pipeline Tests ==="
echo "[Test 1] Pipe between echo and grep"
echo Hello World | grep Hello
echo "[Test 2] Pipe between cat and grep"
echo hello world > testfile.txt
cat testfile.txt | grep hello
echo "[Test 3] Pipe between cat and grep with output redirection"
echo apple > input.txt
cat input.txt | grep apple > output.txt
cat output.txt
rm input.txt output.txt testfile.txt
echo "[Test 4] Pipe between cat with redirection and another cat with wildcard expansion"
echo bacon content > bacon.txt
echo egg content > egg.txt
touch bar
touch baz
touch spam
cat bar < baz | cat *.txt > spam
cat spam
rm bar baz egg.txt bacon.txt spam
echo "=== Pipeline Tests Completed ==="
exit