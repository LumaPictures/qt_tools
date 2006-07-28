


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

int r_find_child_depth(QTAtomContainer ac,QTAtom a,int level)
	{
	int child_count;
	QTAtom ch;
	int result = level;
	int err = level;
	int i;

	child_count = QTCountChildrenOfType(ac,a,0);
	ch = 0;
	for(i = 0; i < child_count; i++)
		{
		err = QTNextChildAnyType(ac,a,ch,&ch);
		bailerr(err,"could not get next child");
		err = r_find_child_depth(ac,ch,level + 1);
		if(err > result)
			result = err;
		}

go_home:
	if(err < 0)
		return err;

	return result;
	}

int r_dump_hex(int indent,int size,unsigned char *p)
	{
	int bytes_per_row = 16;
	int show;
	int i;
	int err = 0;

	while(size)
		{
		show = size;
		if(show > bytes_per_row)
			show = bytes_per_row;

		size = size - show;  // how many left after this?

		for(i = 0; i < show; i++)
			printf("%02x ",p[i]);
		for(i = i; i < bytes_per_row; i++)
			printf("   ");
		printf("| ");

		for(i = 0; i < show; i++)
			printf("%c",(p[i] < 0x20 || p[i] > 0x7e) ? '.' : p[i]);

		printf("\n");
		p = p + show;
		if(size)
			{
			printf("+ ");
			for(i = 0; i < indent; i++)
				printf(" ");
			}
		}

	return err;
	}


int r_show_atom_level(QTAtomContainer ac,QTAtom a,int indent)
	{
	QTAtomType a_type;
	QTAtomID a_id;
	QTAtom ch;
	QTAtomID ch_id;
	int child_count;
	int i;
	int err = 0;
	Ptr a_ptr;
	long a_size;

	err = QTGetAtomTypeAndID(ac,a,&a_type,&a_id);
	bailerr(err,"could not get atom type");

	child_count = QTCountChildrenOfType(ac,a,0);

	// |
	// | maybe do hexdump, maybe walk children
	// | either way, indent and show type[id]
	// |

	printf("+ ");
	for(i = 0; i < indent; i++)
		printf(".");
	printf("%s[%d]: ",o2c(a_type),a_id);

	if(child_count == 0)
		{
		err= QTGetAtomDataPtr(ac,a,&a_size,&a_ptr);
		bailerr(err,"could not get atom data ptr");
		if(a_size && a_ptr)
			{
			err = r_dump_hex(indent + 9,a_size,(unsigned char *)a_ptr);
			bailerr(err,"could not dump hex");
			}
		else
			printf("-\n");
		}
	else
		printf("(%d children)\n",child_count);
	
	ch = 0;
	for(i = 1; i <= child_count; i++)
		{
		err = QTNextChildAnyType(ac,a,ch,&ch);
		bailerr(err,nr_sprintf("could not load child %d",ch_id));

		err = r_show_atom_level(ac,ch,indent+1);
		bailerr(err,nr_sprintf("could not show level %d",indent+1));
		}


go_home:
	return err;
	}

int r_show_atom_contents(QTAtomContainer ac)
	{
	int err = 0;
	int depth;

	err = QTLockContainer(ac);
	bailerr(err,"could not lock container");

	depth = r_find_child_depth(ac,0,0);
	nr_print_int("depth",depth);

	nr_print_int("file size",GetHandleSize(ac));

	err = r_show_atom_level(ac,0,0);
	bailerr(err,nr_sprintf("could now show atom"));

go_home:
	QTUnlockContainer(ac);
	return err;
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

