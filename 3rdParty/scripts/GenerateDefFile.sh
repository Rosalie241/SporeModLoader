#!/usr/bin/env bash
set -e

if [[ "x$2" == "x" ]] ||
	! [[ -f "$1" ]] || ! [[ -f "$2" ]]
then
	echo "$0 [SporeModAPI dll compiled with mingw without def file] [text file with msvc symbols] [output def file]"
	echo "msvc symbols file can be generated using: \"dumpbin /exports /nologo SporeModAPI.dll > msvc_symbols.txt\" on windows"
	exit 1
fi

gcc_symbol_list=$(i686-w64-mingw32-nm "$1" | awk '{ print $3 }')
msvc_symbols=$(cat "$2" | awk '{ print $4 }')
output_file="$3"

readarray -t gcc_symbols <<< "${gcc_symbol_list}"

# prepare gcc symbol list
for ((i=0; i < ${#gcc_symbols[@]}; i++))
do
	if [[ "x${gcc_symbols[$i]}" == "x" ]]
	then
		unset "gcc_symbols[$i]"
	fi
done

# demangle gcc symbols once
echo "-> demangling gcc symbols..."
gcc_demangled_symbols=()
for ((i=0; i < ${#gcc_symbols[@]}; i++))
do
	gcc_demangled_symbols+=("$(echo "${gcc_symbols[$i]}" | i686-w64-mingw32-c++filt)")
done

echo "LIBRARY \"SporeModAPI.dll\"" > "$output_file"
echo "EXPORTS" >> "$output_file"

for msvc_symbol in ${msvc_symbols}
do
	IFS='@' read -r -a msvc_symbol_array <<< "${msvc_symbol%%@@*}"
	if [[ "${#msvc_symbol_array[@]}" == "1" ]]
	then
		msvc_symbol_namespace=""
		msvc_symbol_class=""
		msvc_symbol_function=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 1))]}
		msvc_symbol_function="$(echo "$msvc_symbol_function" | tr -d '?')"
		msvc_converted_symbol="${msvc_symbol_function}"
	elif [[ "${#msvc_symbol_array[@]}" == "2" ]]
	then
		msvc_symbol_namespace=""
		msvc_symbol_class=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 1))]}
		msvc_symbol_function=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 2))]}
		msvc_symbol_function="$(echo "$msvc_symbol_function" | tr -d '?')"
		msvc_converted_symbol="${msvc_symbol_class}::${msvc_symbol_function}"
	elif [[ "${#msvc_symbol_array[@]}" == "3" ]]
	then
		msvc_symbol_namespace=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 1))]}
		msvc_symbol_class=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 2))]}
		msvc_symbol_function=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 3))]}
		msvc_symbol_function="$(echo "$msvc_symbol_function" | tr -d '?')"
		msvc_converted_symbol="${msvc_symbol_namespace}::${msvc_symbol_class}::${msvc_symbol_function}"
	elif [[ "${#msvc_symbol_array[@]}" == "4"  ]]
	then
		msvc_symbol_namespace=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 1))]}
		msvc_symbol_class=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 2))]}
		msvc_symbol_function=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 3))]}
		msvc_symbol_variable=${msvc_symbol_array[$((${#msvc_symbol_array[@]} - 4))]}
		msvc_symbol_variable="$(echo "$msvc_symbol_variable" | tr -d '?')"
		msvc_converted_symbol="${msvc_symbol_namespace}::${msvc_symbol_class}::${msvc_symbol_function}::${msvc_symbol_variable}"
	else
		echo "unknown symbol format: ${msvc_symbol}: ${#msvc_symbol_array[@]}"
		exit 1
	fi

	echo "-> Searching ${msvc_converted_symbol}..."

	found_symbol=false
	for ((i=0; i < ${#gcc_symbols[@]}; i++))
	do
		gcc_symbol="${gcc_symbols[$i]}"
		if [[ "${gcc_demangled_symbols[$i]}" == *"$msvc_converted_symbol"* ]]
		then
			echo "-> Found: ${gcc_demangled_symbols[$i]}"
			echo "    ${msvc_symbol}=${gcc_symbol:1}" >> "$output_file"
			found_symbol=true
			break
		fi
	done
	if [ "$found_symbol" = "false" ]
	then
		echo "failed to find $msvc_symbol: $msvc_converted_symbol"
		exit 1
	fi
done
