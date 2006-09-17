// file: qtc_utils.c
//
// started 2003.03.02
// various helpful routines for command line QuickTime utilities
//

#include "qtc_utils.h"
#include <stdarg.h>
#include "version.h"

#ifndef HOSTNAME
	#define HOSTNAME "(missing hostname)"
#endif

// |
// | verbosity... 0: silent
// |              1: normal
// |              2: chatty
// |              3: debug
// |
int g_verbosity = 1;

void nr_printf(int verbosity_level,char *format,...)
	{
	if(verbosity_level <= g_verbosity)
		{
		va_list v;

		va_start(v,format);
		vprintf(format,v);

		// | end of line?

		if(format[strlen(format) - 1] >= 0x20)
			printf("\n");
		}
	}

char *p2c(StringPtr p)
	{
	char *c = malloc(p[0] + 1);
	
	// this is so tiny, it must not fail. If it does, our so-called
	// "modern" digital computing device is in very bad shape, and
	// frankly the user has worse troubles than this tool poof-ing.

	// Note that we (I say "we" haha, its me, all me) don't ever
	// dispose these little mallocs. Well, these utilities are for
	// command line tools which run for a little while, and
	// then exit. Exit. Think of exiting as garbage collection, at
	// a programmatically convenient granularity.

	memcpy(c,p+1,p[0]);
	c[p[0]] = 0;

	return c;
	}

char *h2c(Handle h)
	{
	int len = GetHandleSize(h);
	char *c = malloc(len + 1);

	memcpy(c,*h,len);
	c[len] = 0;

	return c;
	}

Handle c2h(char *c)
	{
	Handle h;
	int len = strlen(c);

	h = NewHandle(len);
	memcpy(*h,c,len);
	return h;
	}

StringPtr c2p(char *c)
	{
	int l;
	StringPtr p;

	l = strlen(c);
	if(l > 255)
		l = 255;

	p = malloc(l + 1);
	p[0] = l;
	memcpy(p + 1,c,l);

	return p;
	}

char *o2c(OSType o)
	{
	char *c = malloc(5);
	int i;

	memcpy(c,&o,4);
	c[4] = 0;

	for(i = 0; i < 4; i++)
		if(c[i] < 0x20 || c[i] > 0x7e)
			c[i] = '.';

	return c;
	}

OSType c2o(char *c)
	{
	OSType o = '    ';
	int p = 32;

	//!!!if(!c || (c[0] == '0' && c[1] == 0))  // let '0   ' be 0
		//!!!return 0;

	if(c[0] == '.'  // let '....' be 0
			& c[1] == '.'
			& c[2] == '.'
			& c[3] == '.')
		return 0;

	while(p && *c && (*c != ','))   // means we cant have a comma in the OSType...
		{
		p -= 8;
		o &= ~(0x000000ff << p);    // knock out the space...
		o |= *c << p;
		c++;
		}

	return o;
	}

Fixed d2f(double d)
	{
	UnsignedFixed uf;
	Fixed f;

	uf = (UnsignedFixed)(d * 65536);

	f = uf;
	return f;
	}

double f2d(Fixed f)
	{
	double d;
	unsigned long uf = f;

	d = uf;
	d = d / 65536.0;

	return d;
	}

char *csubstr(char *x,int first,int len)
	{
	char *result = malloc(len + 1);
	int i;
	for(i = 0; i < len; i++)
		result[i] = x[first + i];
	result[i] = 0;
	return result;
	}


// +--------------------------------
// | int nr_find_arg
// |    (
// |	int argc,                 argc and argv from main()
// |	char *argv[],
// |	char *argname,            a --parameter to look for
// |	int *value_int_out,       its numeric value (decimal or 0xHex)
// |	char **value_string_out   the string out (pointer in place)
// |	)
// |
// | if argname is a string for a decimal integer, like "1" or "2", &c,
// | then we look for the Nth non-dash param. (typically file names)
// |
// | Does this mean we cant have parameters like --1=foo.mov?
// | yes, it does. Or at least, you cannot use nr_find_arg() to
// | find it.
// |



int nr_find_arg(int argc,char *argv[],char *argname,int *value_int_out,char **value_string_out)
	{
	int i;
	char an1[k_string_size];
	int an1_len;
	char *value_string = 0;
	int value_int = 0;
	int found_it = 0;
	int undashed_index = 0;

	sscanf(argname,"%d",&undashed_index);

	if(undashed_index != 0) // "1", "2", or like that
		{
		for(i = 1; i < argc; i++)
			{
			char *w = argv[i];

			if(w[0] != '-') // found an undashed arg
				{
				undashed_index--; // found the Nth one?
				if(undashed_index == 0)
					{
					found_it = 1;
					value_string = w;
					goto go_home;
					}
				}
			}
		goto go_home; // didnt find it.
		}

	strcpy(an1,"--");
	strcat(an1,argname);
	an1_len = strlen(an1);

	for(i = 0; i < argc; i++)
		{
		char *w = argv[i];


		if(strstr(w,an1) == w)
			{
			// |
			// | found argument. does it have =something?
			// |

			if(w[an1_len] == '=')
				value_string = w + an1_len + 1;
			else
				value_string = "1";

			found_it = 1;
			goto go_home;
			}
		}
	// didnt find it, fall through (with found_it unset)

go_home:
	if(found_it)
		{
		if(value_int_out)
			{
			// |
			// | convert to number
			// |

			if(value_string[0] == '0' && value_string[1] == 'x')
				sscanf(value_string,"%x",&value_int);
			else
				sscanf(value_string,"%d",&value_int);

			*value_int_out = value_int;
			}

		if(value_string_out)
			*value_string_out = value_string;
		}

	return found_it;
	}

char *r_find_arg_index(int argc, char *argv[],char *argname,int index)
	{
	int found_it;
	char *value;
	char *result = 0;

	// | no such param? return null
	found_it = nr_find_arg(argc,argv,argname,0,&value);
	if(!found_it)
		goto go_home;
	// | count commas...
	while(index-- > 0)
		{
		char *next_comma = strstr(value,",");

		if(next_comma == 0) // no more commas?
			goto go_home;
		else
			value = next_comma + 1;
		}

	// | empty string (maybe like --param=1,,3 index 1)? return null
	if(value[0] == 0 || value[0] == ',')
		goto go_home;

	result = value;

go_home:
	return result;
	}

char *nr_find_arg_string(int argc, char *argv[],char *argname,int index,char *dflt)
	{
	char *value;
	char *result = dflt;

	value = r_find_arg_index(argc,argv,argname,index);
	if(!value)
		goto go_home;
	
	result = value;

go_home:
	return result;
	}

int nr_find_arg_int(int argc, char *argv[],char *argname,int index,int dflt)
	{
	char *value;
	int result = dflt;

	value = r_find_arg_index(argc,argv,argname,index);
	if(!value)
		goto go_home;

	if(value[0] == '0' && value[1] == 'x')
		sscanf(value + 2,"%x",&result);
	else
		sscanf(value,"%d",&result);

go_home:
	return result;
	}

double nr_find_arg_double(int argc, char *argv[],char *argname,int index,double dflt)
	{
	char *value;
	double result = dflt;

	value = r_find_arg_index(argc,argv,argname,index);
	if(!value)
		goto go_home;

	sscanf(value,"%Le",&result);
go_home:
	return result;
	}

OSType nr_find_arg_ostype(int argc, char *argv[],char *argname,int index,OSType dflt)
	{
	char *value;
	OSType result = dflt;

	value = r_find_arg_index(argc,argv,argname,index);
	if(!value)
		goto go_home;

	result = c2o(value);
go_home:
	return result;
	}


// A Math Routine Or Two

int nr_map_int(int value,
		int sourceRangeLow,int sourceRangeHigh,
		int destinationRangeLow,int destinationRangeHigh)
	{
	int sourceRange = sourceRangeHigh - sourceRangeLow;
	int destinationRange = destinationRangeHigh - destinationRangeLow;
	
	value -= sourceRangeLow;
	value = (int)((double)value * destinationRange / sourceRange);
	value += destinationRangeLow;
	
	return value;
	}


void nr_enter_movies(void)
	{
	static ever_did_enter = 0;

	if(!ever_did_enter)
		{
		GrafPtr safetyPort;
		nr_printf(3,"EnterMovies();");
		EnterMovies();
		ever_did_enter = 1;

		// |
		// | Turns out QuickTime needs a port, any port, to work
		// | (though, without one, it may limp along sort of ok
		// | for a while), so we make one here.
		// | (Actually, everything works fine without doing this,
		// | but I heard you had to, so I'm leaving it in. dvb -- 2003)
		// |

		safetyPort = CreateNewPort();
		SetPort( safetyPort );
		}
	}

Movie nr_new_movie_from_file(char *filename)
	{
	Movie mo = 0;
	short frefnum = -1;
	short movie_resid = 0;

	mo = nr_new_movie_from_file_x(filename,&frefnum,&movie_resid);

	if(frefnum != -1)
		CloseMovieFile(frefnum);
	return mo;
	}

// +------------------
// | nr_new_movie_from_file_x returns to you the resrefnum and resid,
// | and you the caller must close the movie file, with CloseMovie(resrefnum).
// |

Movie nr_new_movie_from_file_x(char *filename,short *resrefnum_out,short *resid_out)
	{
	OSErr err;
	FSSpec fs;
	Movie mo = 0;
	short frefnum = -1;
	short movie_resid = 0;

	nr_enter_movies();
	err = NativePathNameToFSSpec(filename,&fs,0);
	if(err == fnfErr) err = 0; // mixed doc on whether you get fnfErr with this call!
	bailerr(err,"NativePathNameToFSSpec");

	err = OpenMovieFile(&fs,&frefnum,0);
    if(err)
        goto go_home;  // this is ok to fail -- return null.

	err = NewMovieFromFile(&mo,frefnum,&movie_resid,0,newMovieActive,0);
	bailerr(err,"NewMovieFromFile");

go_home:
	if(!mo)
		{
		frefnum = 0;
		movie_resid = 0;
		}

	*resrefnum_out = frefnum;
	*resid_out = movie_resid;
	return mo;
	}

ComponentInstance nr_open_component(OSType ctype,OSType csubtype, OSType cmfr)
	{
	int err;
	ComponentDescription cd = {0};
	ComponentDescription cd2 = {0};
	Component co = 0;
	ComponentInstance ci = 0;
	OSType osubtype;
	OSType omfr;

	cd.componentType = ctype;

	osubtype= csubtype;
	omfr = cmfr;

	nr_printf(3,"nr_open_component %s:%s (%08x:%08x)",
			o2c(osubtype),
			o2c(omfr),
			osubtype,
			omfr);
	// |
	// | march through components until subtype & mfr match
	// |

	co = 0;
	while (0 != (co = FindNextComponent(co,&cd)))
		{
		GetComponentInfo(co,&cd2,0,0,0);
		nr_printf(3,"FindNextComponent: 0x%08x, %s:%s (%08x:%08x)",
				co,
				o2c(cd2.componentSubType),
				o2c(cd2.componentManufacturer),
				cd2.componentSubType,
				cd2.componentManufacturer);

		if(cd2.componentSubType == osubtype
				&& cd2.componentManufacturer == omfr)
			{
			err = OpenAComponent(co,&ci);
			bailerr(err,nr_sprintf("OpenAComponent %d",err));
			goto go_home;
			}
		}

	bailerr(-1,"FindNextComponent");

go_home:
	return ci;
	}


// +----------------------------------------------
// | Misc Graphics
// |
// |
void GoGrey(int grey) // 0..255
{
	RGBColor co;
	co.red = co.green = co.blue = grey<<8;
	RGBForeColor(&co);
}
void GoBW()
{
	ForeColor(blackColor);
	BackColor(whiteColor);
}

// +----------------------------------------------
// | routines for pretty formatted printing
// |
// | So all our tools have a similar display look
// |

void nr_print_info(char *title, char *value)
	{
	nr_printf(1,"+ %26s : %s \n",title,value);
	}

void nr_print_cr(void)
	{
	nr_printf(1,"+");
	}

void nr_print_int(char *title,int v)
	{
	char s[k_string_size];

	sprintf(s,"%d",v);
	nr_print_info(title,s);
	}

void nr_print_int_of_int(char *title,int v1,int v2)
	{
	char s[k_string_size];

	sprintf(s,"%d of %d",v1,v2);
	nr_print_info(title,s);
	}

void nr_print_int2(char *title,int v1,int v2)
	{
	char s[k_string_size];

	sprintf(s,"%d / %d",v1,v2);
	nr_print_info(title,s);
	}

void nr_print_int3(char *title,int v1,int v2,int v3)
	{
	char s[k_string_size];

	sprintf(s,"%d / %d / %d",v1,v2,v3);
	nr_print_info(title,s);
	}

void nr_print_dims(char *title,int width,int height)
	{
	char s[k_string_size];

	sprintf(s,"%d x %d",width,height);
	nr_print_info(title,s);
	}

void nr_print_createmodtime(char *title,unsigned long t1,unsigned long t2)
	{
	char s[k_string_size];

	sprintf(s,"%lu / %lu",t1,t2);
	nr_print_info(title,s);
	}

char *r_d2s(double v)
	{
	char *s = malloc(k_string_size);

	if(v == (int)v)
		sprintf(s,"%d",(int)v);
	else
		sprintf(s,"%.2f",v);

	return s;
	}

// nr_sprintf is just like sprintf, but it allocates a string for you
// (And, as with the rest of this library and the tools who use it,
// it's never collected. Come on. This is 200x, and it is a command
// tool which runs briefly, then quits. Let the OS dispose my
// strings with the whole process. Hooray.)
char *nr_sprintf(char *format,...)
	{
	va_list v;
	char *s = malloc(k_string_size);

	va_start(v,format);
	vsprintf(s,format,v);
	return s;
	}



void nr_print_d_frame_rate(char *title,double v)
	{
	char s[k_string_size];

	if(v)
		sprintf(s,"%s",r_d2s(v));
	else
		sprintf(s,"movie");

	nr_print_info(title,s);
	}

void nr_print_d_data_rate(char *title,double v)
	{
	char s[k_string_size];

	if(v)
		sprintf(s,"%s",r_d2s(v));
	else
		sprintf(s,"whatever");

	nr_print_info(title,s);
	}

void nr_print_d_audio_format(char *title,double sample_rate,int sample_size,int channel_count)
	{
	char s[k_string_size];
	char cc[k_string_size];

	if(channel_count == 1)
		sprintf(cc,"mono");
	else if(channel_count == 2)
		sprintf(cc,"stereo");
	else
		sprintf(cc,"%d",channel_count);

	sprintf(s,"%s / %d / %s",r_d2s(sample_rate),sample_size,cc);
	nr_print_info(title,s);
	}

void nr_print_double(char *title,double v)
	{
	char s[k_string_size];

	sprintf(s,"%s",r_d2s(v));
	nr_print_info(title,s);
	}

void nr_print_d_time_range(char *title,double v1,double v2)
	{
	char s[k_string_size];

	if(v2 == 0)
		sprintf(s,"all");
	else if(v1 == 0)
		sprintf(s,"%f",v2);
	else
		sprintf(s,"%.4f to %.4f",v1,v2);

	nr_print_info(title,s);
	}
	
void nr_print_yesno(char *title,int v)
	{
	char s[k_string_size];

	sprintf(s,"%s",v ? "yes" : "no");
	nr_print_info(title,s);
	}

void nr_print_string(char *title,StringPtr v)
	{
	char s[k_string_size];

	sprintf(s,"%s",p2c(v));
	nr_print_info(title,s);
	}

void nr_print_ostype(char *title, OSType v)
	{
	char s[k_string_size];

	sprintf(s,"%s",o2c(v));
	nr_print_info(title,s);
	}

void nr_print_dimensions(char *title, Fixed width, Fixed height)
	{
	char s[k_string_size];

	sprintf(s,"(%d,%d)",width >> 16,height >> 16);
	nr_print_info(title,s);
	}

void nr_print_time(char *title,TimeValue v,TimeScale scale)
	{
	char s[k_string_size];
	double t;

	t = v;
	t = t / scale;

	sprintf(s,"%.3f (%d/%d)",t,v,(int)scale);
	nr_print_info(title,s);
	}

void nr_print_rate(char *title,Fixed r)
	{
	char s[k_string_size];
	int w;
	int f;

	w = r >> 16;
	f = (r & 0x0000ffff);

	f = f * 100000 / 65536;

	sprintf(s,"%d.%06d",w,f);
	nr_print_info(title,s);
	}

void nr_print_volume(char *title,short v)
	{
	char s[k_string_size];

	sprintf(s,"%d",v);
	nr_print_info(title,s);
	}

void nr_print_rect(char *title,Rect *r)
	{
	char s[k_string_size];

	sprintf(s,"(%d,%d,%d,%d)",r->left,r->top,r->right,r->bottom);
	nr_print_info(title,s);
	}

void nr_print_rgn(char *title,RgnHandle rgn)
	{
	char s[k_string_size];
	Rect rgn_bounds;

	if(!rgn)
		sprintf(s,"(null)");
	else if(rgn)
		{
		GetRegionBounds(rgn,&rgn_bounds);

		if(EmptyRgn(rgn))
			sprintf(s,"(empty)");
		else
			{
			if(!IsRegionRectangular(rgn))
				sprintf(s,"non");
			else
				s[0] = 0;
			sprintf(s + strlen(s),"rectangular (%d,%d,%d,%d)",
					rgn_bounds.left,
					rgn_bounds.top,
					rgn_bounds.right,
					rgn_bounds.bottom);
			}
		}

	nr_print_info(title,s);
	}

void nr_print_handler(char *title,OSType htype,OSType hmfr,StringPtr hname)
	{
	char s[k_string_size];

	sprintf(s,"%s/%s %s",o2c(htype),o2c(hmfr),p2c(hname));
	nr_print_info(title,s);
	}

// |
// | Handy atom container <--> file
// |

OSErr nr_file_to_atom_container(char *filename,QTAtomContainer *ac_out)
	{
	FSSpec fs;
	short fref = -1;
	long len;
	OSErr err = 0;
	QTAtomContainer ac = 0;

	err = NativePathNameToFSSpec(filename,&fs,0);
	bailerr(err,"NativePathNameToFSSpec");

	err = FSpOpenDF(&fs,fsRdPerm,&fref);
	bailerr(err,"FSpOpenDF");
	
	err = GetEOF(fref,&len);
	bailerr(err,"GetEOF");

	ac = (QTAtomContainer)NewHandleClear(len);
	err = MemError();
	bailerr(err,"NewHandleClear");

	err= FSRead(fref,&len,*(Handle)ac);
	bailerr(err,"FSRead");

go_home:
	if(fref != -1)
		err = FSClose(fref);

	if(err && ac)
		{
		QTDisposeAtomContainer(ac);
		ac = 0;
		}
	*ac_out = ac;

	return err;
	}

OSErr nr_atom_container_to_file(QTAtomContainer ac,char *filename)
	{
	FSSpec fs;
	short fref = -1;
	long len;
	OSErr err;

	err = NativePathNameToFSSpec(filename,&fs,0);
	if(!err)
		{
		// found it? we must delete (dee ee vee oh) we must delete
		err = FSpDelete(&fs);
		bailerr(err,"FSpDelete");
		}
	else if(err == fnfErr)
		err = 0;
	bailerr(err,"NativePathNameToFSSpec");

	err = FSpCreate(&fs,'dvbx','data',smSystemScript);
	bailerr(err,"FSpCreate");

	err = FSpOpenDF(&fs,fsRdWrPerm,&fref);
	bailerr(err,"FSpOpenDF");

	len = GetHandleSize((Handle)ac);

	err = FSWrite(fref,&len,*(Handle)ac);
	bailerr(err,"FSWrite");

go_home:
	if(fref != -1)
		err = FSClose(fref);

	return err;
	}


OSErr nr_find_deep_atom(QTAtomContainer ac,char *path,QTAtom *atom_out,int create_as_we_go)
	{
	QTAtom a = 0;
	QTAtom a_child;
	QTAtomType at;
	OSErr err = 0;

	nr_printf(3,"nr_find_deep_atom: \'%s\'%s",path,create_as_we_go ? ", create_as_we_go":"");
	if(strlen(path) % 4)
		{
		err = -1;
		bailerr(err,"nr_find_deep_atom_data: path must be 4n");
		}

	// |
	// | Find the requested atom
	// |

	while(strlen(path) >= 4)
		{
		memcpy(&at,path,4);
		path += 4;

		nr_printf(3,"nr_find_deep_atom: (\'%s\')",o2c(at));

		a_child = QTFindChildByIndex(ac,a,at,1,0);
		if(!a_child)
			{
			if(create_as_we_go)
				{
				err = QTInsertChild(ac,a,at,1,1,0,0,&a_child);
				bailerr(err,"QTInsertChild");
				}
			else
				{
				// not found! (but silent error, we expect this, often)
				err = -1;
				}
			}

		a = a_child;
		}

go_home:
	if(err)
		a = 0;
	else
		nr_printf(3,"nr_find_deep_atom: returning 0x%08x",a);
	*atom_out = a;

	return err;
	}

int nr_get_deep_atom_data(QTAtomContainer ac,char *path,int data_out_size,void *data_out)
	{
	OSErr err = 0;
	QTAtom a;
	long data_len;
	void *data_ptr;

	err = nr_find_deep_atom(ac,path,&a,0);
	if(err)
		goto go_home;

	err = QTGetAtomDataPtr(ac,a,&data_len,(Ptr *)&data_ptr);
	bailerr(err,"QTGetAtomDataPtr");

	if(data_len != data_out_size)
		{
		err = -1;
		goto go_home;
		}

	memcpy(data_out,data_ptr,data_out_size);

go_home:
	return err;
	}

// |
// | using a path composed of 4n characters,
// | descend the atom container and add the requested data
// |

OSErr nr_insert_deep_atom_data(QTAtomContainer ac,char *path,int data_size,void *data)
	{
	return nr_insert_deep_atom_data_id
			(
			ac,
			path,
			data_size,
			data,
			1
			);
	}

OSErr nr_delete_deep_atom(QTAtomContainer ac,char *path)
	{
	QTAtom a = 0;
	OSErr err;

	err = nr_find_deep_atom(ac,path,&a,0);
	if(!err && a)
		{
		err = QTRemoveAtom(ac,a);
		bailerr(err,"QTRemoveAtom");
		}
	err = 0;

go_home:
	return err;
	}

OSType nr_last_four_chars(char *filename)
    {
    char *last4 = filename + strlen(filename) - 4; // cheese!
    OSType result = c2o(last4);
    return result;
    }

OSErr nr_insert_deep_atom_data_id(QTAtomContainer ac,char *path,int data_size,void *data,QTAtomID id)
	{
	QTAtom a = 0;
	QTAtom a_child;
	QTAtomType at;
	OSErr err;
	char outer_path[k_string_size];

	// |
	// | First: delete the atom if it exists
	// |

	err = nr_delete_deep_atom(ac,path);
	err = 0;

	// |
	// | Next: descend to just above our new atom,
	// | creating the parents as we descend
	// |
	strcpy(outer_path,path);
	outer_path[strlen(path) - 4] = 0; // clip off last path segment

	err = nr_find_deep_atom(ac,outer_path,&a,1);
	bailerr(err,"nr_find_deep_atom");

	// |
	// | and pop in our new child
	// |
	memcpy(&at,path + strlen(path) - 4,4);
	QTInsertChild(ac,a,at,id,1,data_size,data,&a_child);
	bailerr(err,"QTInsertChild");

go_home:
	return err;
	}

OSErr nr_insert_deep_atom_byte(QTAtomContainer ac,char *path,char byte_of_data)
	{
	return nr_insert_deep_atom_data(ac,path,1,&byte_of_data);
	}

OSErr nr_insert_deep_atom_short(QTAtomContainer ac,char *path,short short_of_data)
	{
	return nr_insert_deep_atom_data(ac,path,2,&short_of_data);
	}

OSErr nr_insert_deep_atom_long(QTAtomContainer ac,char *path,long long_of_data)
	{
	return nr_insert_deep_atom_data(ac,path,4,&long_of_data);
	}

OSErr nr_file_exists(char *filename)
	{
	FSSpec fs;
	FInfo finfo;
	OSErr err = 0;

	err = NativePathNameToFSSpec(filename,&fs,0);
	if(err)
		goto go_home;

	err = FSpGetFInfo(&fs,&finfo);

go_home:
	// return noErr if file exists
	return err;
	}

void nr_print_version(char *filename)
	{
	char program_name[k_string_size];

	strcpy(program_name,filename);   // turn blah.c to blah
	program_name[strlen(program_name) - 2] = 0;

	printf("\n");
	printf(":\n");
	printf(":  %s version %s.%s-[%s] %s\n",
			program_name,
			k_version_major,
			k_version_minor,
			k_version_build_number,
			k_version_date);
	printf(":  built on %s\n",HOSTNAME);
	printf(":  (c) David Van Brink, poly@omino.com\n");
	printf(":\n");
	}



// --------------------------------------
// | Image Sequence Goodies

// Start with a filename, and populate sequence_stuff so we can
// generate related filenames by index number

#define m_isdigit(_char) (((_char) >= '0') && ((_char) <= '9'))

void nr_filename_to_sequence_stuff(char *filename,sequence_stuff *seqstuff_out)
{
	int firstDigitIndex = -1;
	int lastDigitIndex = -1;
	int len = strlen(filename);
	int i;
	
//	i = 0;
//	while(firstDigitIndex < 0 && i < len)
//	{
//		char k = filename[i];
//		if(m_isdigit(k))
//			firstDigitIndex = i;
//		i++;
//	}
//
//	while(lastDigitIndex < 0 && i < len)
//	{
//		char k = filename[i];
//		if(!m_isdigit(k))
//			lastDigitIndex = i;
//		i++;
//	}

    /***
     *  Walk backwards, assuming people put their frame numbers near
     *  the end of the file.
     *  Chris Perry Thu Sep 14 21:46:32 EDT 2006
     */
	i = len - 1;
    while (lastDigitIndex < 0 && i >= 0)
    {
        char k = filename[i];
        if(m_isdigit(k))
            lastDigitIndex = i+1;
        else
            i--;
    }
    while (firstDigitIndex < 0 && i >= 0)
    {
        char k = filename[i];
        if (!m_isdigit(k))
            firstDigitIndex = i + 1;
        else
            i--;
    }
	/* end of Chris Perry fix */



	
	seqstuff_out->beforeNumerals = csubstr(filename,0,firstDigitIndex);
	seqstuff_out->digitCount = lastDigitIndex - firstDigitIndex;
	
	seqstuff_out->indexMax = 1;
	for(i = 0; i < seqstuff_out->digitCount; i++)
		seqstuff_out->indexMax *= 10;
	
	seqstuff_out->afterNumerals = csubstr(filename,lastDigitIndex,len - lastDigitIndex);

    seqstuff_out->fmt = nr_sprintf("%%s%%0%dd%%s",seqstuff_out->digitCount);
}

// using sequence_stuff, get a filename by index.
char *nr_sequence_stuff_to_filename(sequence_stuff *seqstuff,int index)
{
	return nr_sprintf(seqstuff->fmt,seqstuff->beforeNumerals,index,seqstuff->afterNumerals);
}





// end of file
