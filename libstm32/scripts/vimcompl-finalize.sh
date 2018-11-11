#!/bin/sh
#Final vimcomplete and generate vim helper files
CROSS_COMPILE='arm-none-eabi-'
CXX="${CROSS_COMPILE}g++"
COMPL_FILE=$1
if [ ! -f "$COMPL_FILE" ]; then
	echo "Unable to find compl file"
	exit
fi
#Remove not existing directories
gawk -i inplace '@load "filefuncs"
	{ 
		if( $1 ~ /-I/ ) {
			fullname=$1
			gsub(/-I/,""); 
			dpath=$1
			rc=stat(dpath,status)
			if(rc==0 && status["type"]=="directory") {
				print fullname
			}
		} else {
			print $0
		}
	}'  $COMPL_FILE

#Extra version
echo "-I./build/isixrtos/libisix/include" >> $COMPL_FILE

#Add extra system paths
$CXX -Wp,-v -x c++ - -fsyntax-only 2>&1 </dev/null | \
	gawk -v cfile=${COMPL_FILE} ' /^ / { 
					gsub(/ /, "", $$1)
					print "-isystem\n"$$1 >> cfile; 
			} 
			END { 
				print "-std=c++17" >> cfile; 
				print "-m32" >> cfile; 
			} '
#Extra vim custom include directories generator
	gawk 'BEGIN { 
			v="."; 
		}  
		/^-I/ { 
			gsub("-I","", $$1); v=v","$$1
		} 
		END { 
			print "set path="v  > ".vim.custom"; 
		}' "$COMPL_FILE"
	cp -i "$COMPL_FILE" .
