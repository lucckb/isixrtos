#!/bin/sh
#Final vimcomplete and generate vim helper files
CROSS_COMPILE='arm-none-eabi-'
CXX="${CROSS_COMPILE}g++"
COMPL_FILE=$1
if [ ! -f "$COMPL_FILE" ]; then
	echo "Unable to find compl file"
	exit
fi

#Add extra system paths
$CXX -Wp,-v -x c++ - -fsyntax-only 2>&1 </dev/null | \
	awk -v cfile=${COMPL_FILE} ' /^ / { 
					print "-isystem\n"$$1 >> cfile; 
			} 
			END { 
				print "-std=c++14" >> cfile; 
			} '
#Extra vim custom include directories generator
	awk 'BEGIN { 
			v="."; 
		}  
		/^-I/ { 
			gsub("-I","", $$1); v=v","$$1
		} 
		END { 
			print "set path="v  > ".vim.custom"; 
		}' "$COMPL_FILE"
cp -i "$COMPL_FILE" .
