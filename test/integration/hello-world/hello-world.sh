#!/usr/bin/env bash

# Assumes `mondriaan` is built in build/
# Assumes that the runtime library is built in lib/build/src/

exit_code=0

# Test hello-world-1
base_1=$(mktemp -d /tmp/mondriaan-hello-world.XXXXXX)
prog_1="${base_1}/hello-world-1-small"
c_output_1=$(../../../build/mondriaan --output-file "$prog_1" hello-world-1-small.png)
LIBRARY_PATH=../../../lib/build/src/ clang++ "${prog_1}.o" -lMondriaanRuntime -o "${prog_1}"
output_1=$("${prog_1}")
if [[ "$output_1" != "Hello world!" ]]; then
	echo "Failed test 'hello-world-1-small'"
	echo "Expected 'Hello world!' but received '${output_1}'"
	echo "Compiler output: ${c_output_1}"
	exit_code=1
fi
rm -r "${base_1}"

# Test hello-world-1, big version
base_2=$(mktemp -d /tmp/mondriaan-hello-world.XXXXXX)
prog_2="${base_2}/hello-world-1-big"
c_output_2=$(../../../build/mondriaan --output-file "$prog_2" --codel-size 5 hello-world-1-big.png)
LIBRARY_PATH=../../../lib/build/src/ clang++ "${prog_2}.o" -lMondriaanRuntime -o "${prog_2}"
output_2=$("${prog_2}")
if [[ "$output_2" != "Hello world!" ]]; then
	echo "Failed test 'hello-world-1-big'"
	echo "Expected 'Hello world!' but received '${output_2}'"
	echo "Compiler output: ${c_output_2}"
	exit_code=1
fi
rm -r "${base_2}"

# Report success, if any
if [[ ${exit_code} -eq 0 ]]; then
	echo "Integration tests passed 🎉"
fi
exit ${exit_code}
