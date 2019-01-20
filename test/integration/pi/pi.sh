#!/usr/bin/env bash

# Assumes `mondriaan` is built in build/
# Assumes that the runtime library is built in lib/build/src/

exit_code=0

# Test pi-small
base_1=$(mktemp -d /tmp/mondriaan-pi.XXXXXX)
prog_1="${base_1}/pi-small"
c_output_1=$(../../../build/mondriaan --output-file "$prog_1" pi-small.png)
LIBRARY_PATH=../../../lib/build/src/ clang++ "${prog_1}.o" -lMondriaanRuntime -o "${prog_1}"
output_1=$("${prog_1}")
if [[ "$output_1" != "31405" ]]; then
	echo "Failed test 'pi-small'"
	echo "Expected '31405' but received '${output_1}'"
	echo "Compiler output: ${c_output_1}"
	exit_code=1
fi
rm -r "${base_1}"

# Report success, if any
if [[ ${exit_code} -eq 0 ]]; then
	echo "Integration tests passed 🎉"
fi
exit ${exit_code}
