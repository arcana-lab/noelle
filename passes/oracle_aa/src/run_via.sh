#!/usr/bin/env bash


name="$1"
execution_args="$2"

ir_in="${name}.bc"
instrumented_ir="${name}.via.bc"
bin_name=${name}

# run VIAInstrument

${oracle_opt_cmd} -load LLVMVIAInstrument.so -via -S ${ir_in} -o ${instrumented_ir}

# Compile Instrumented file

export VIACONF_SETTING="${name}-oracle-ddg.viaconf"

${oracle_clang_cmd} ${instrumented_ir} -o ${bin_name} -L"${via_install_dir}/lib" -lvia


# Execute Instrumented binary

echo "executing ${bin_name} with args '${execution_args}'"

./${bin_name} ${execution_args}

echo "done"