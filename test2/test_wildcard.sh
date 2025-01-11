cd testdir
cd test_wildcard
echo "=== Wildcard Tests ==="
echo "[Test 1] List files in test_wildcard directory, including hiddenfiles"
ls -a
echo "[Test 2] List files matching 'example*.txt' in 'test_wildcard' directory"
ls example*.txt
echo "[Test 3] List files in 'subdir' directory matching '*.txt'"
ls subdir/*.txt
echo "=== Wildcard Tests Completed ==="
exit
