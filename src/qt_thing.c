


#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <stdio.h>

#include "qtc_utils.h"

#include "qtc_manpages.h"

void man(void)
	{
	nr_print_version(__FILE__);

	#ifdef print_qt_thing_man
		print_qt_thing_man
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

	U("qt_thing is a command line tool")
	U("which lists components installed")
	U("in your system")
	Uu
	U("usage: qt_thing [--type=<type>]")
	Uu
	U("\"qt_thing --man | more\" will tell you the");
	U("same thing with a lot more words.");
	Uu

	nr_printf(1,"");
	exit(0);
	}


void list_components(OSType ct,int show_type_hex)
	{
	ComponentDescription cd = {0};
	ComponentDescription cd2 = {0};
	Component co = 0;
	int c = 0;
	Handle component_name_h = NewHandle(0);

	cd.componentType = ct;
	c = CountComponents(&cd);

	printf("+\n");
	printf("+ There are %d components of type \'%s\' (0x%08x)\n",c,o2c(ct),ct);
	printf("+\n");

	c = 0;
	cd.componentType = 0;
	while(1)
		{
		co = FindNextComponent(co,&cd);
		if(!co)
			goto go_home; // done

		GetComponentInfo(co,&cd2,component_name_h,0,0);
		if(ct && cd2.componentType != ct)
			goto next_one;
		c++;
		printf("+ %4d @0x%08x    %s:%s:%s",c,co,
				o2c(cd2.componentType),
				o2c(cd2.componentSubType),
				o2c(cd2.componentManufacturer));

		if(show_type_hex)
			{
			printf(" (%08x:%08x:%08x)",
					cd2.componentType,
					cd2.componentSubType,
					cd2.componentManufacturer);
			}
		
		printf("     \"%.*s\"\n",**(char **)component_name_h,(*(char **)component_name_h) + 1);
next_one:
		;
		}
go_home:
	printf("+\n");
	return;
	}

int main(int argc,char *argv[])
	{
	int i;
	char *component_type_string;
	int show_type_hex = 0;
	OSType component_type = 0;
	int ai;

	EnterMovies();
	setbuf(stdout,0);

	if(nr_find_arg(argc,argv,"version",0,0))
		version();

	if(nr_find_arg(argc,argv,"help",0,0))
		usage();

	if(nr_find_arg(argc,argv,"man",0,0))
		man();

	if(nr_find_arg(argc,argv,"type",&ai,&component_type_string))
		component_type = c2o(component_type_string);

	show_type_hex = nr_find_arg_int(argc,argv,"showhex",0,show_type_hex);
	list_components(component_type,show_type_hex);
	}

