// file: qtc_utils.h
//
// started 2003.03.02
// various helpful routines for command line QuickTime utilities
//


// |
// | Inclusions for all
// |

#include <QuickTime/QuickTime.h>

// |
// | Macros and Konstants
// |

#define k_string_size 1024

// | Convenient for error handling and assault-printing

#define bail(_verbosity,_msg) \
		{ \
        nr_printf(3,"### (%s: %d)\n",__FILE__,__LINE__); \
		nr_printf(_verbosity,"### error: %s\n",_msg); \
		if(!err) err = -1; \
		goto go_home; \
		}

// low level bail macros

#define bailerr(_x,_msg) if((_x) < 0) bail(2,nr_sprintf("%s %d" , (_msg) , (_x) ))
#define bailnil(_x,_msg) if((_x) == 0) bail(2,_msg)

// outermost bail macros

#define obailerr(_x,_msg) if((_x) < 0) bail(1,nr_sprintf("%s %d" , (_msg) , (_x) ))
#define obailnil(_x,_msg) if((_x) == 0) bail(1,_msg)

// | Convenient for --help usage

#define U(x)  printf(":  %s\n",x);
#define Ue(x) printf(":       %s\n",x);
#define Uu printf(":\n");

// |
// | Globals
// |

extern int g_verbosity;

// |
// | Routines
// |

char *p2c(StringPtr p);
char *h2c(Handle h);
Handle c2h(char *c);
StringPtr c2p(char *c);
char *o2c(OSType o);
OSType c2o(char *c);
Fixed d2f(double d);
double f2d(Fixed f);

char *csubstr(char *x,int first,int len);
int lastIndexOf(char *x,char y);
char *fileExt(char *filename);
int stringsEqual(char *s1,char *s2);


int nr_find_arg(int argc,char *argv[],char *argname,int *value_int_out,char **value_string_out);

// | variations on find arg which return values or default value
// | also, the index parameter (0..n) looks within comma-separated lists

char *nr_find_arg_string(int argc, char *argv[],char *argname,int index,char *dflt);
int nr_find_arg_int(int argc, char *argv[],char *argname,int index,int dflt);
double nr_find_arg_double(int argc, char *argv[],char *argname,int index,double dflt);
OSType nr_find_arg_ostype(int argc, char *argv[],char *argname,int index,OSType dflt);

int nr_map_int(int value,
		int sourceRangeLow,int sourceRangeHigh,
		int destinationRangeLow,int destinationRangeHigh);

void nr_enter_movies(void);
Movie nr_new_movie_from_file(char *filename);
Movie nr_new_movie_from_file_x(char *filename,short *resrefnum_out,short *resid_out);
void nr_set_movie_time_scale(Movie mo, TimeScale timeScale);
OSErr nr_flatten_movie_to_file(Movie mo,char *filename);
ComponentInstance nr_open_component(OSType ctype,OSType csubtype, OSType cmfr);

// | pretty printing for these utilities

void nr_print_info(char *title, char *value);
void nr_print_cr(void);
void nr_print_int(char *title,int v);
void nr_print_int2(char *title,int v1,int v2);
void nr_print_int_of_int(char *title,int v1,int v2);
void nr_print_int3(char *title,int v1,int v2,int v3);
void nr_print_double(char *title,double v);
void nr_print_createmodtime(char *title,unsigned long t1,unsigned long t2);
void nr_print_d_frame_rate(char *title,double v);
void nr_print_d_time_range(char *title,double v1,double v2);
void nr_print_d_audio_format(char *title,double sample_rate,int sample_size,int channel_count);
void nr_print_yesno(char *title,int v);
void nr_print_string(char *title,StringPtr v);
void nr_print_ostype(char *title, OSType v);
void nr_print_dimensions(char *title, Fixed width, Fixed height);
void nr_print_time(char *title,TimeValue v,TimeScale scale);
void nr_print_rate(char *title,Fixed r);
void nr_print_volume(char *title,short v);
void nr_print_rect(char *title,Rect *r);
void nr_print_rgn(char *title,RgnHandle rgn);
void nr_print_handler(char *title,OSType htype,OSType hmfr,StringPtr hname);

char *nr_sprintf(char *format,...);

// | QuickTime Atoms Made Easy

OSErr nr_file_to_atom_container(char *filename,QTAtomContainer *ac_out);
OSErr nr_atom_container_to_file(QTAtomContainer ac,char *filename);

OSErr nr_find_deep_atom(QTAtomContainer ac,char *path,QTAtom *atom_out,int create_as_we_go);
OSErr nr_get_deep_atom_data(QTAtomContainer ac,char *path,int data_out_size,void *data_out);
OSErr nr_get_deep_atom_short(QTAtomContainer ac,char *path,short *data_out);
OSErr nr_get_deep_atom_long(QTAtomContainer ac,char *path,long *data_out);
OSErr nr_get_deep_atom_ostype(QTAtomContainer ac,char *path,OSType *data_out);


OSErr nr_delete_deep_atom(QTAtomContainer ac,char *path);
OSErr nr_insert_deep_atom_data(QTAtomContainer ac,char *path,int data_size,void *data);
OSErr nr_insert_deep_atom_byte(QTAtomContainer ac,char *path,char byte_of_data);
OSErr nr_insert_deep_atom_short(QTAtomContainer ac,char *path,short short_of_data);
OSErr nr_insert_deep_atom_long(QTAtomContainer ac,char *path,long long_of_data);
OSErr nr_insert_deep_atom_ostype(QTAtomContainer ac,char *path,OSType long_of_data);

// full struct fetch/gett
OSErr nr_get_deep_atom_SCSpatialSettings(QTAtomContainer ac,char *path,SCSpatialSettings *sps);
OSErr nr_insert_deep_atom_SCSpatialSettings(QTAtomContainer ac,char *path,SCSpatialSettings *sps);

OSErr nr_get_deep_atom_SCTemporalSettings(QTAtomContainer ac,char *path,SCTemporalSettings *sts);
OSErr nr_insert_deep_atom_SCTemporalSettings(QTAtomContainer ac,char *path,SCTemporalSettings *sts);

OSErr nr_get_deep_atom_SCDataRateSettings(QTAtomContainer ac,char *path,SCDataRateSettings *sds);
OSErr nr_insert_deep_atom_SCDataRateSettings(QTAtomContainer ac,char *path,SCDataRateSettings *sds);

OSErr nr_insert_deep_atom_data_id
		(QTAtomContainer ac,char *path,int data_size,void *data,QTAtomID id);

// | More Misc

OSErr nr_file_exists(char *filename);
void nr_printf(int verbosity_level,char *format,...);
void nr_print_version(char *filename);


// | Stuff for image sequence filename manipulation

typedef struct
{
	char *beforeNumerals;
	int digitCount;
	int indexMax;
	char *afterNumerals;
	char *fmt;
} sequence_stuff;

void nr_filename_to_sequence_stuff(char *filename,sequence_stuff *seqstuff_out);
char *nr_sequence_stuff_to_filename(sequence_stuff *seqstuff,int index);

// | the meat of qt_atom...
int r_show_atom_contents(QTAtomContainer ac);


// end of file
