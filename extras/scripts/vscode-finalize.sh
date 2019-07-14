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
# Generate cpp properties
mkdir -p .vscode
HDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
gawk -v fname="$HDIR/vscode_tpl/c_cpp_properties.json" '
	BEGIN {
		includes = ""
		defines = ""
	} 
	{
		if( $1 ~ /-I/ &&  $1 !~ /\.\./ ) {
			gsub(/-I/,""); 
			gsub(/\.\//,""); 
			line = "\t\t\"${workspaceFolder}/" $1 "\",\n";
			includes = includes line
		}
		if( $1 ~ /-D/ ) {
			gsub(/-D/,""); 
			line = "\t\t\t\"" $1 "\",\n";
			defines = defines line
		}
	}	
    END {
		 includes =  substr(includes, 1, length(includes)-2)
		 defines = substr(defines, 1, length(defines)-2)
		 while(( getline line<fname) > 0 ) {
			 gsub("<INCLUDES_REPLACE>",includes,line)
			 gsub("<DEFINES_REPLACE>",defines,line)
			 print line
		}
	}
' $COMPL_FILE > .vscode/c_cpp_properties.json
cp -v "$HDIR/vscode_tpl/launch.json" .vscode/
cp -v "$HDIR/vscode_tpl/settings.json" .vscode/
cp -v "$HDIR/vscode_tpl/tasks.json" .vscode/
