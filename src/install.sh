#!/bin/sh

# |
# | Script for installing qt_tools
# | dvb 2003 March 25
# |

# |
# | My favorite shell script convenience -- an easy
# | bailout function
# |

quit_on_error () 
	{ 
	ferr=$?;
	msg=$1
	if [ $ferr != 0 ]; then

		echo "."
		echo ". ### error $ferr: $msg"
		echo ". ### bye"
		echo "."

		exit $ferr;
	fi
	}

# +---------------------------
# | The main thing
# |

qmandir=/usr/local/share/man/man1/
qbindir=/usr/local/bin/
clear
echo "."
echo "."
echo "."
echo "."
echo "."
echo ". -------------------------------------------------------------"
echo ".                          qt_tools"
echo ".                 __version__"
echo ".                    by dvb, poly@omino.com"
echo ". -------------------------------------------------------------"
echo "."
echo "."
echo ". You are about to install the qt_tools suite."
echo ". This is a small collection of tools for"
echo ". manipulating QuickTime movies from the command"
echo ". line."
echo "."
echo ".             Tools: $qbindir"
echo ".         Man Pages: $qmandir"
echo "."
echo ". (This installer will only work if you have root"
echo ". privileges. Alternatively, you can use the executables"
echo ". right from the installer image.)"
echo "."

echo -n ".      Do you wish to install qt_tools? (y/n): "
read ok_to_go

if [ "X$ok_to_go" != "Xy" ] && [ "X$ok_to_go" != "XY" ] ; then
	echo "."
	echo ". Installation canceled."
	echo "."
	exit 0
fi

echo "."
echo "."

ditto -rsrcFork ./pieces/bin/* $qbindir
quit_on_error "Could not copy application files"
echo ". Installed tools."

mkdir -p $qmandir
quit_on_error "Could not ensure man pages directory $qmandir (tools will work ok)"

ditto -rsrcFork ./pieces/man/man1/* $qmandir
quit_on_error "Could not copy man pages to $qmandir (tools will work ok)"
echo ". Installed man pages."

echo "."
echo ". -------------------------------------------------------------"
echo ". Successfully installed qt_tools __version__."
echo ". -------------------------------------------------------------"

# end of file
