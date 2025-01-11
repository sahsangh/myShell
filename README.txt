Sahil Sanghvi sss426
Kevin Kuriakose kk1286


Testing Strategy and Test Cases

The test plan for mysh aims to ensure that the shell works as expected for all required functionalities, including built-in commands, redirection, pipelines, wildcards, and more. Below is a detailed description of the scenarios tested, along with how each was validated.

1. Built-in Commands

	•	cd Command: Tested with valid and invalid directories, as well as incorrect argument counts.
        •	How we tested:
cd testdir
ls
*outputs test_pipeline test_redirection test_wildcard
cd nonexistent_dir
*outputs cd: No such file or directory
	•	pwd Command: Validated that it prints the correct current working directory.
        •	How we tested:
cd subdir
cd subsubdir
pwd
*outputs current/path/subdir/subsubdir
	•	which Command: Tested with valid and invalid command names.
        •	How we tested:
which ls
*output /usr/bin/ls
which nonexistent_command
*outputs nothing
	•	exit Command: Tested exiting the shell, both with and without arguments.
        •	How we tested
exit
*outputs Exiting my shell.
        •	shell is terminated

2. External Commands

	•	Basic Execution: Verified execution of basic external commands like ls and echo.
        •	Example: ls, /bin/echo Hello World
echo Hello World
*outputs Hello World
ls
*outputs current directory

3. Input and Output Redirection (< and >)

	•   Input Redirection: Tested with commands that read from files.
We created a file called test_redirection.sh that tests how our shell handles redirections:

cd testdir
cd test_redirection
echo "This is a test file" > input.txt
touch empty.txt
echo "Old content" > output.txt
echo "=== Redirection Tests ==="
echo "[Test 1] Input redirection with a valid file:"
cat < input.txt
echo "[Test 2] Input redirection with a nonexistent file:"
cat < non_existent_file.txt
echo "[Test 3] Input redirection with an empty file:"
cat < empty.txt
echo "[Test 4] Output redirection to a new file:"
echo Hello World > new_output.txt
cat new_output.txt
echo "[Test 5] Output redirection overwriting an existing file:"
echo 'Overwritten content' > output.txt
cat output.txt
echo "[Test 7] Input and output redirection with valid files:"
cat < input.txt > new_output.txt
cat new_output.txt
echo "[Test 8] Input redirection with missing input file:"
cat < missing_file.txt > output.txt
rm -f input.txt output.txt new_output.txt empty.txt
echo "=== Redirection Tests Completed ==="
exit

Our shell correctly outputted the following:

"=== Redirection Tests ==="
"[Test 1] Input redirection with a valid file:"
"This is a test file"
"[Test 2] Input redirection with a nonexistent file:"
Error opening input file: No such file or directory
"[Test 3] Input redirection with an empty file:"
"[Test 4] Output redirection to a new file:"
Hello World
"[Test 5] Output redirection overwriting an existing file:"
'Overwritten content'
"[Test 7] Input and output redirection with valid files:"
"This is a test file"
"[Test 8] Input redirection with missing input file:"
Error opening input file: No such file or directory
"=== Redirection Tests Completed ==="

4. Pipes (|)

	•	Basic Pipeline: Tested connecting commands with a pipe.
We created a file called test_pipeline.sh that tests how our shell handles pipelines:

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

Our shell correctly outputted the following:

"=== Pipeline Tests ==="
"[Test 1] Pipe between echo and grep"
Hello World
"[Test 2] Pipe between cat and grep"
hello world
"[Test 3] Pipe between cat and grep with output redirection"
apple
"[Test 4] Pipe between cat with redirection and another cat with wildcard expansion"
bacon content
egg content
"=== Pipeline Tests Completed ==="

5. Wildcards (*)

	•	Wildcard Expansion: Verified commands that expand wildcard tokens to match files.
We created a file called test_wildcard.sh that tests how our shell handles wildcard tokens:

For context - in the test_wildcard folder, there are 5 files including a .hiddenfile. there is also a subdir with a sub.txt file

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

Our shell correctly outputted the following:

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


6. Error Handling

	•	Invalid Commands: Tested commands that do not exist.
If we run:

nonexistent_command

Our shell outputs:

nonexistent_command: Command not found

    	•	Syntax Errors: Tested commands with incorrect syntax.
If we run:

cat <

Our shell outputs:

Syntax error: expected input file after '<'


8. Batch Mode Testing

	•	Run a script (myscript.sh) with multiple commands in batch mode:
	    •	Example script (myscript.sh):
echo hello
cd subdir
ls
cd subsubdir
Pwd
ls asdf

Our shell's output:

hello
subsubdir
Current/path/subdir/subsubdir
/usr/bin/ls: cannot access 'asdf': No such file or directory

	•	Run ./mysh myscript.sh and verify that the output matches the expected output.

9. Interactive Mode Testing

	•	Basic Prompt: Ensure that the shell displays mysh>  when running interactively.
	•	Exit from Interactive Mode: Type exit to verify the shell terminates properly.


Summary

Having a comprehensive set of test cases ensures that mysh behaves as expected in both typical and edge cases. It also helps identify errors that may be introduced during future modifications or extensions to the shell. The test cases listed above aim to cover all major functionalities, including command execution, redirection, piping, wildcard expansion, and built-in command handling.