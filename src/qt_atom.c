


#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <stdio.h>

#include "qtc_utils.h"

#include "qtc_manpages.h"

void man(void)
	{
	nr_print_version(__FILE__);

	#ifdef print_qt_atom_man
		print_qt_atom_man
	#else
		Uu
		U("No man page in this build.")
		Uu
	#endif

	exit(1);
	}

void version(void)
	{
	nr_print_version(__FILE__);
	exit(1);
	}

void usage(void)
	{
	nr_print_version(__FILE__);

	U("qt_atom is a command line tool")
	U("for manipulating settings files")
	U("produced by qt_export.")
	Uu
	U("usage: qt_atom <settings file>")
	Uu
	U("\"qt_atom --man | more\" will tell you the");
	U("same thing with a lot more words.");
	Uu

	nr_printf(1,"");
	exit(0);
	}
int main(int argc,char *argv[])
	{
	int i;
	int err = 0;
	char *atom_filename;
	OSType component_type = 0;
	int ai;
	QTAtomContainer ac = 0;

	EnterMovies();
	setbuf(stdout,0);

	if(nr_find_arg(argc,argv,"version",0,0))
		version();

	if(nr_find_arg(argc,argv,"help",0,0))
		usage();

	if(nr_find_arg(argc,argv,"man",0,0))
		man();

	g_verbosity = nr_find_arg_int(argc,argv,"verbosity",0,1);

	if(nr_find_arg(argc,argv,"1",&ai,&atom_filename))
		{
		err = nr_file_to_atom_container(atom_filename,&ac);
		obailerr(err,nr_sprintf("could not load settings file %s",atom_filename));

		nr_print_info("settings file",atom_filename);

		err = r_show_atom_contents(ac);
		obailerr(err,nr_sprintf("problem displaying atom %s",atom_filename));
		}
	else
		usage();

go_home:
	if(ac)
		QTDisposeAtomContainer(ac);

	nr_printf(1,"");
	if(err)
		exit(1);

	exit(0);
	}

