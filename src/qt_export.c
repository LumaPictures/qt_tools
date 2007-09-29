// file: qt_export.c
//
// started:
// 2003 March 4 dvb
//
// A command line utility for exporting quicktime movies
// into new formats
//

#include <stdio.h>
#include <CoreServices/CoreServices.h>
#include <QuickTime/QuickTime.h>

#include "qtc_utils.h"
#include "qtc_manpages.h"
#include "rationalize.h"

#define k_string_size 1024


#include "settings.c" // definitions of all the atom presets


// |
// | The order for the parameters stacked up is like so:
// |
// | 1. Some defaults built into this program
// | 2. The settings loaded from a file
// | 3. The command line parameters
// | 4. The results of the user dialog
// |
// | each of these is optional, except the first one
// |



void man(void)
	{
	nr_print_version(__FILE__);
	#ifdef print_qt_export_man
		print_qt_export_man
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

	U("qt_export is a command line tool");
	U("for converting QuickTime movies to")
	U("different formats.");
	Uu
	U("usage: qt_export [options] <source movie file> <exported movie file>")
	Uu
	U("options:")
	Ue("--video=<compressor[,frame rate,quality,bits per pixel]>")
	Ue("--audio=<compressor[,samples per second,bits per sample,channel count]>")
	Ue("--datarate=<kilobytes per second>")
	Ue("--keyframerate=<frames per key frame>")
	Ue("--duration=<[start time,]end time>")
	Ue("--replacefile")
	Ue("--loadsettings=<export settings file>");
	Ue("--dodialog");
	Ue("--savesettings=<export settings file>");
    Ue("--exporter=<exporter[,subtype]>");
    Ue("--sequencerate=<frames per second> treat source as image sequence");
	Uu
	U("also:")
	Ue("--video=0 to omit video")
	Ue("--audio=0 to omit audio")
	Uu
	U("also also:")
	Ue("\"qt_export --man | more\" for the full schpiel.")
	Uu
	U("by david van brink, poly@omino.com, subject line must begin with \"quicktime\"")
	Uu

	nr_printf(1,"");

	exit(0);
	}

// +-----------------------------
// |
// | settings from the --parameters
// |

typedef struct
	{
	// files (pointers to argv strings)

	char *source_movie_name;
	char *export_movie_name;

	// --loadsettings=, --dodialog, --savesettings=
    QTAtomContainer settings; // sometimes we preload this a bit.
	char *load_settings; // atom container as file
	int do_dialog;       // if so, do the QuickTime exporter dialog
	char *save_settings;
    int forced_duration; // some export guesses will FORCE the duration to be small, for single frame out.
    int forced_dialog;
    int dont_put_params_atop_atom;

	// --exporter=subtype,mfr (default MooV,appl)
	OSType exporter_subtype;
	OSType exporter_mfr;


	// --video=compressor,frame rate,quality,bits per pixel
	OSType video_compressor;
	double frame_rate;
	int quality;  // arbitrary range 0 to 100.
	int pixel_size; // 1,2,4,8,16,32

	// --audio=compressor,samplerate,samplesize,channels
	OSType audio_compressor;
	double sample_rate;
	int sample_size; // 8 or 16, generally hey
	int channel_count; // 1 or 2

	double data_rate; // k per second
	int key_frame_rate; // frames per key frame

	// --duration=[start_time,]end_time
	double start_time; // seconds
	double end_time; // seconds, 0 means whole time
    
    // --size=width[,height]
    long width;
    long height;

	int replace_file; // 1 means ok to delete existing file of same name
	int verbosity; // 0 silent, 1 normal, 2 verbose, 3 debuggery

	int exporter_audio; // 1 if were doing an audio-exporty-thing
	int exporter_video; // 1 if were doing a video-exporty-thing

    int b_supports_atoms;   // alas, apple 'MPEG'/0 mpeg 2 exporter does not
    int b_ignore_user_canceled; // special hack for 'MPEG'/0 mpeg2 exporter

    double sequence_rate;
	int ticks_per_frame;
	int ticks_per_second;
	} qte_parameter_settings;


// +---------------------
// | reverse engineered structures...

// | The "cdec" atom for QDM2 appears to be
// | sufficiently intertwined to be Not Worth reverse
// | engineering. You just have to do a --dodialog to
// | get repeatable settings for it.

// | Roxio's is really easy though, for MPG/Roxi
typedef struct
	{
	char format;       // 0: ntsc, 2: pal
	char shape;        // 1: fit, 2: crop, 3: fill
	char launch_toast; // 0: no, 1: yes
	char zero_1;       // must be 0
	char quality;      // 0: normal, 1: faster, 2: better
	char zeropad[11];  // must be 0
	} roxio_RtvS;

void r_print_parameter_settings(qte_parameter_settings *ss)
	{
	nr_print_cr();
	nr_print_ostype("exporter subtype",ss->exporter_subtype);
	nr_print_ostype("exporter mfr",ss->exporter_mfr);
	nr_print_cr();
	nr_print_d_time_range("duration",ss->start_time,ss->end_time);
    
    if(ss->width > 0)
    {
        nr_print_dims("size",ss->width,ss->height);
    }

	
	if(ss->video_compressor != '0   ' || ss->exporter_video)
		{
		nr_print_ostype("video compressor",
				ss->video_compressor != '0   ' ? ss->video_compressor : ss->exporter_subtype);
		nr_print_d_frame_rate("video frames per second",ss->frame_rate);
		nr_print_int("video quality",ss->quality);
		nr_print_int("video bits per pixel",ss->pixel_size);
		nr_print_int("video key frame rate",ss->key_frame_rate);
		}
	else
		nr_print_info("video","off");

	nr_print_cr();

	if(ss->audio_compressor != '0   ' || ss->exporter_audio)
		{
		nr_print_ostype("audio compressor",
				ss->audio_compressor != '0   ' ? ss->audio_compressor : ss->exporter_subtype);
		nr_print_d_audio_format("audio format",ss->sample_rate,ss->sample_size,ss->channel_count);
		nr_print_cr();
		nr_print_d_data_rate("data kilobytes per second",ss->data_rate);
		nr_print_cr();
		nr_print_yesno("replace existing file",ss->replace_file);
		}
	else
		nr_print_info("audio","off");
	nr_print_cr();
	}

int r_atom_container_to_parameter_settings(QTAtomContainer ac,qte_parameter_settings *ss)
	{
	SCSpatialSettings sps = {0};
	SCTemporalSettings sts = {0};
	SCDataRateSettings sdr = {0};
	int err = 0;
	char envi = 0;
	char enso = 0;
	UnsignedFixed uf_sample_rate;

	// |
	// | Video settings
	// |

	err = nr_get_deep_atom_data(ac,"videsptl",sizeof(sps),&sps);
	if(err == 0) // present?
		{
		ss->video_compressor = sps.codecType;
		ss->pixel_size = sps.depth;
		ss->quality = sps.spatialQuality * 100.0 / 1024.0;
		}

	err = nr_get_deep_atom_data(ac,"videtprl",sizeof(sts),&sts);
	if(err == 0) // present?
		{
		ss->frame_rate = f2d(sts.frameRate);
		ss->key_frame_rate = sts.keyFrameRate;
		}

	err = nr_get_deep_atom_data(ac,"videdrat",sizeof(sdr),&sdr);
	if(err == 0)
		{
		ss->data_rate = sdr.dataRate / 1024.0;
		}

	err = nr_get_deep_atom_data(ac,"envi",1,&envi);
	if(err || envi == 0)
		ss->video_compressor = '0   ';
	
	// |
	// | Audio Settings
	// |

	err = nr_get_deep_atom_data(ac,"sounssct",sizeof(OSType),&ss->audio_compressor);

	err = nr_get_deep_atom_data(ac,"sounssrt",sizeof(uf_sample_rate),&uf_sample_rate);
	if(err == 0)
		ss->sample_rate = f2d(uf_sample_rate);

	err = nr_get_deep_atom_data(ac,"sounssss",sizeof(ss->sample_size),&ss->sample_size);
	err = nr_get_deep_atom_data(ac,"sounsscc",sizeof(ss->channel_count),&ss->channel_count);

	err = nr_get_deep_atom_data(ac,"enso",1,&enso);
	if(err || enso == 0)
		ss->audio_compressor = '0   ';

	err = 0;

	return err;
	}

// +-----------------------------
// | Given an atom container, modify it
// | by any parameters in the parameter settings
// |
// | One subtlety: for the video compressor,
// | '0   ' means No Video! so delete the envi atom
// | NULL   means no opinion, leave those atoms alone
// | other  means that's the compressor we want.
// |
// | similarly for the audio compressor
// |
// | dvb07 -- this only works for movie-exporters...

int r_parameter_settings_atop_atom_container(qte_parameter_settings *ss,QTAtomContainer ac)
	{
	SCSpatialSettings sps = {0};
	SCTemporalSettings sts = {0};
	SCDataRateSettings sdr = {0};
	int err = 0;

	// | Video settings, if video

	if(ss->video_compressor == '0   ')
		{
		// | no video!
		err = nr_delete_deep_atom(ac,"envi");  // presence of the atom makes video...
		bailerr(err,"could not delete envi atom");
		}
	else if(ss->video_compressor != 0) // other than null value...
		{
		sps.codecType = ss->video_compressor;
		sps.depth = ss->pixel_size;
		sps.spatialQuality = ss->quality * 1024 / 100; // quality goes 0x0100 to 0x0400

		sts.temporalQuality = 0x0200;
		sts.frameRate = d2f(ss->frame_rate);
		sts.keyFrameRate = ss->key_frame_rate;

		sdr.dataRate = (int)(ss->data_rate * 1024); // was k/second
		sdr.frameDuration = 0xffff8000; // I saw it in the defaults once
		sdr.minSpatialQuality = 0x0100; // I saw it in the defaults once
		sdr.minTemporalQuality = 0x0100; // I saw it in the defaults once

		err = nr_insert_deep_atom_byte(ac,"envi",1); // enable video
		bailerr(err,"could not create settings atoms");
		err = nr_insert_deep_atom_data(ac,"videsptl",sizeof(sps),&sps);
		bailerr(err,"could not create settings atoms");
		err = nr_insert_deep_atom_data(ac,"videtprl",sizeof(sts),&sts);
		bailerr(err,"could not create settings atoms");
		err = nr_insert_deep_atom_data(ac,"videdrat",sizeof(sdr),&sdr);
		bailerr(err,"could not create settings atoms");

        // I saw this in quite a few movie setings files:
        // vide/wdth gets 16.16 width, 100 is 00 64 00 00
        // vide/hegt gets 16.16 height
        // isiz/iwdt gets 32 bit width, 100 is 00 00 00 64
        // isiz/ihgt gets 32 bit height
        if(ss->width > 0 && ss->height > 0)
        {
            nr_insert_deep_atom_long(ac,"videwdth",ss->width << 16);
            nr_insert_deep_atom_long(ac,"videhegt",ss->height << 16);
            nr_insert_deep_atom_long(ac,"isiziwdt",ss->width);
            nr_insert_deep_atom_long(ac,"isizihgt",ss->height);
        }
        
        
		long mfr = 0; // TRY IT! 2006.03.07
		err = nr_insert_deep_atom_data(ac,"videcmfr",sizeof(mfr),&mfr);


		}

	if(ss->audio_compressor == '0   ')
		{
		err = nr_delete_deep_atom(ac,"enso"); // disable audio (sound)
		bailerr(err,"could not delete enso atom");
		}
	else if(ss->audio_compressor != 0)
		{
		err = nr_insert_deep_atom_byte(ac,"enso",1); // enable audio (sound)
		bailerr(err,"could not create settings atoms");
		err = nr_insert_deep_atom_long(ac,"sounssct",ss->audio_compressor);
		bailerr(err,"could not create settings atoms");
		err = nr_insert_deep_atom_long(ac,"sounssrt",d2f(ss->sample_rate));
		bailerr(err,"could not create settings atoms");
		err = nr_insert_deep_atom_short(ac,"sounssss",ss->sample_size);
		bailerr(err,"could not create settings atoms");
		err = nr_insert_deep_atom_short(ac,"sounsscc",ss->channel_count);
		bailerr(err,"could not create settings atoms");
		}

go_home:
	nr_printf(3,"r_parameter_settings_atop_atom_container: returning %d bytes",
			ac ? GetHandleSize((Handle)ac) : 0);

	return err;
	}

// +-------------------------------
// | applying parameter settings... we do it all, here.
// | 


OSErr r_progress_proc
		(
		Movie theMovie,
		short message,
		short whatOperation,
		Fixed percentDone,
		long refcon
		)
	{
	qte_parameter_settings *ss = (qte_parameter_settings *)refcon;
	double percent = percentDone * 100.0 / 65536.0;
	static double recent_percent = -100;


	// |
	// | dont show more than uhhh a bunch progress notes
	// | also, avoid spewing a zillion 100% progresses at
	// | the end of Roxio's stupid mpeg exporter
	// | (which I really like for the most part -- dvb)
	// |

	double pdel = percent - recent_percent;
	if((pdel < 2) && (pdel > 0))
		goto go_home;
	recent_percent = percent;

	nr_printf(1,"# progress: (%03d:%03d) %6.2f%%       %s --> %s\n",
			(int)whatOperation,
			(int)message,
			percent,
			ss->source_movie_name,
			ss->export_movie_name
			);

	// could show output file size here, wouldnt that be kinda cool

go_home:
	return 0;
	}

/*
 * Assign a reasonable exporter type & subtype,
 * based on the output file extension.
 */

static int forceDoDialog(qte_parameter_settings *ss)
{
    ss->forced_dialog = 1;
}

static int makeMpg4Settings(qte_parameter_settings *ss)
{
    QTAtomContainer ac = (QTAtomContainer)NewHandleClear(sizeof(mpg4));
    memcpy(*(void **)ac,mpg4,sizeof(mpg4));
    ss->settings = ac;
    ss->dont_put_params_atop_atom = 1;
}

static int makeM4aSettings(qte_parameter_settings *ss)
{
    QTAtomContainer ac = (QTAtomContainer)NewHandleClear(sizeof(m4a));
    memcpy(*(void **)ac,m4a,sizeof(m4a));
    ss->settings = ac;
    ss->dont_put_params_atop_atom = 1;
}

static int makeAviSettings(qte_parameter_settings *ss)
{
    QTAtomContainer ac = (QTAtomContainer)NewHandleClear(sizeof(avi));
    memcpy(*(void **)ac,avi,sizeof(avi));
    ss->settings = ac;
    ss->dont_put_params_atop_atom = 1;
}

static int makeGrexAtomAndPokeSettings(OSType kind,qte_parameter_settings *ss)
{
    //    + sean[1]: (3 children)
    //    + .time[1]: (1 children)
    //    + ..fps [1]: 00 00 00 00                                     | ....
    //    + .ftyp[1]: 4a 50 45 47                                     | JPEG
    //    + ." "?[1]: 01                                              | .    
    QTAtomContainer js = 0;
    int err = QTNewAtomContainer(&js);
    obailerr(err,"Could not create atom container");
    nr_insert_deep_atom_data(js,"timefps ",4,"\0\0\0\0");
    nr_insert_deep_atom_long(js,"ftyp",kind);
    nr_insert_deep_atom_data(js,"\" \"?",1,"\0");
    ss->settings = js;
    ss->forced_duration = 1;
    ss->dont_put_params_atop_atom = 1;
go_home:
        return err;
}

static int makeJpegSettings(qte_parameter_settings *ss)
{
    return makeGrexAtomAndPokeSettings('JPEG',ss);
}

static int makePngSettings(qte_parameter_settings *ss)
{
    return makeGrexAtomAndPokeSettings('PNGf',ss);
}

static int makeTgaSettings(qte_parameter_settings *ss)
{
    QTAtomContainer ac = (QTAtomContainer)NewHandleClear(sizeof(tga));
    memcpy(*(void **)ac,tga,sizeof(tga));
    ss->settings = ac;
    ss->dont_put_params_atop_atom = 1;
    ss->forced_duration = 1;
}



#define EXTMAP(_ext,_subtype,_mfr) \
if(stringsEqual(ext,_ext)) \
{ \
    ss->exporter_subtype = _subtype; \
		ss->exporter_mfr = _mfr; \
    nr_printf(2,"# file ext is %s, guessing exporter %s:%s\n",_ext,o2c(ss->exporter_subtype),o2c(ss->exporter_mfr)); \
}

#define EXTMAP_AND_MORE(_ext,_subtype,_mfr,_doMore) \
if(stringsEqual(ext,_ext)) \
{ \
    ss->exporter_subtype = _subtype; \
		ss->exporter_mfr = _mfr; \
        _doMore(ss); \
    nr_printf(2,"# file ext is %s, guessing exporter %s:%s\n",_ext,o2c(ss->exporter_subtype),o2c(ss->exporter_mfr)); \
}

static void r_guess_exporter_subtype(qte_parameter_settings *ss)
{
	ss->exporter_subtype = 'MooV';
	ss->exporter_mfr = 'appl';

	char *ext = fileExt(ss->export_movie_name);

	EXTMAP("aif",'AIFF','soun');
	EXTMAP("aiff",'AIFF','soun');
	EXTMAP("dv",'dvc!','appl');
	EXTMAP("wav",'WAVE','soun');
	EXTMAP("mp2",'MPEG',0);
	EXTMAP_AND_MORE("mp4",'mpg4','appl',makeMpg4Settings);
	EXTMAP_AND_MORE("m4a",'mpg4','appl',makeM4aSettings); // just like mp4, but with audio only.
	EXTMAP("au",'ULAW','soun');
	EXTMAP_AND_MORE("avi",'VfW ','appl',makeAviSettings);
	EXTMAP("bmp",'BMPf','....');
    EXTMAP_AND_MORE("jpg",'grex','appl',makeJpegSettings);
    EXTMAP_AND_MORE("png",'grex','appl',makePngSettings);
    EXTMAP_AND_MORE("tga",'grex','appl',makeTgaSettings);
    

		/*
		 * The LAME mp3 encoder, 
		 * from Lynn Pye -- no longer
		 * available??
		 * pyehouse.com
		 */
	EXTMAP("mp3",'mp3 ','PYEh');

//	if(stringsEqual(ext,"aif") || stringsEqual(ext,"aiff"))
//	{
//		ss->exporter_subtype = 'AIFF';
//		ss->exporter_mfr = 'soun';
//	}
//	if(stringsEqual(ext,"mp3"))
//	{
//		ss->exporter_subtype = 'mp3';
//		ss->exporter_mfr = 'PYEh';
//	}
//	if(stringsEqual(ext,"dv"))
//	{
//		ss->exporter_subtype = 'dvc!';
//		ss->exporter_mfr = 'appl';
//	}
}


int r_args_to_settings(int argc,char **argv,qte_parameter_settings *ssOut)
{
	qte_parameter_settings ss = {0};
	int found_it = 0;
	int err = 0;

    ss.sequence_rate = -1;
    ss.b_supports_atoms = 1;
    ss.b_ignore_user_canceled = 0;
    ss.forced_duration = 0;
    ss.forced_dialog = 0;
	
  	// |
	// | needs two movie arguments to work... or none
	// | (for just settings-stuffs)
	// |

	found_it = nr_find_arg(argc,argv,"1",0,&ss.source_movie_name);
	found_it = nr_find_arg(argc,argv,"2",0,&ss.export_movie_name);

	// |
	// | If they did specify an export name, try to pick
	// | the right exporter based on the filename extension
	// |
	r_guess_exporter_subtype(&ss);

    {
        // if an exporter is spec'd on the command line,
        // undo that nasty forced-duration from maybe jpg or still image stuff.
        // or a duration
        double y = nr_find_arg_double(argc,argv,"duration",1,ss.end_time);
        OSType z = nr_find_arg_ostype(argc,argv,"exporter",0,0);
        char *z3 = 0;
        nr_find_arg(argc,argv,"loadsettings",0,&z3);
        
        if(z || y)
            ss.forced_duration = 0;

        if(z || z3)
            ss.forced_dialog = 0;
    }


	if(ss.source_movie_name && !ss.export_movie_name)
		{
		err = -1;
		obailerr(err,"If you provide a source file then you must provide an export file too");
		}

	// | set the output verbosity early

	ss.verbosity = nr_find_arg_int(argc,argv,"verbosity",0,1);
	g_verbosity = ss.verbosity; // global

	// |
	// | populate the parameter settings
	// |

	// | these first parameter settings come ONLY from command line
	// | (never from stored atoms)

	nr_find_arg(argc,argv,"loadsettings",0,&ss.load_settings);
	nr_find_arg(argc,argv,"savesettings",0,&ss.save_settings);
	nr_find_arg(argc,argv,"dodialog",&ss.do_dialog,0);
	ss.replace_file = nr_find_arg_int(argc,argv,"replacefile",0,0);
    ss.sequence_rate = nr_find_arg_double(argc,argv,"sequencerate",0,ss.sequence_rate);

    {
		unsigned long long ticks_per_frame; // aka numerator
		unsigned long long ticks_per_second; // aka denominator & time scale
		
        // inverted, turning fps into time...
		rationalize(ss.sequence_rate,1000000,1000000,&ticks_per_second,&ticks_per_frame);
		while(ticks_per_second < 60)
		{
			ticks_per_second *= 10;
			ticks_per_frame *= 10;
		}
		ss.ticks_per_frame = ticks_per_frame;
		ss.ticks_per_second = ticks_per_second;
	}

	// |
	// | !!! TODO (as they say): fill in an exporter type based on the
	// | filename extension of the export file. That way, qt_export blah.mp3 foo.aiff
	// | would just do what you meant. For example.
	// |

	ss.video_compressor = 'SVQ3';   // sorenson...
	ss.frame_rate = 0.0;
	ss.quality = 50;
	ss.pixel_size = 32;

	ss.audio_compressor = 'QDM2';
	ss.audio_compressor = 'twos';  // turns out that QDM2 is a bad choice for a default!
	ss.sample_rate = 44100;
	ss.sample_size = 16;
	ss.channel_count = 2;

	ss.exporter_subtype = nr_find_arg_ostype(argc,argv,"exporter",0,ss.exporter_subtype);

	// | User changing exporter? we set up some reasonable defaults here for mfr
	// | (if mfr is 'appl', its just a default, we might change it)
	if(ss.exporter_mfr == 'appl')
		switch(ss.exporter_subtype)
			{
			case 'AIFF':
			case 'sfil': // system 7 sound
			case 'snd ': // classic MacOS sound file
				ss.exporter_mfr = 'soun';
				ss.audio_compressor = 'twos';
				ss.video_compressor = 0;
				ss.exporter_audio = 1;
				break;

			case 'WAVE':
				ss.exporter_mfr = 'soun';
				ss.audio_compressor = 'raw ';
				ss.video_compressor = 0;
				ss.exporter_audio = 1;
				break;

			case 'MPG ':  // default mpeg1 exporter is roxio's
				ss.exporter_mfr = 'Roxi';
				ss.exporter_audio = 1;
				ss.exporter_video = 1;
				break;

			case 'MPEG': // mpeg 2
				ss.exporter_mfr = 0;  // strangely, mfr is 0
				ss.exporter_audio = 1;
				ss.exporter_video = 1;
				break;
			}

	ss.exporter_mfr = nr_find_arg_ostype(argc,argv,"exporter",1,ss.exporter_mfr);

	ss.data_rate = 100;    // kilobytes per second, default (video)
	ss.key_frame_rate = 60;

	ss.start_time = 0.0;
	ss.end_time = 0.0;

	ss.video_compressor = nr_find_arg_ostype(argc,argv,"video",0,ss.video_compressor);
	ss.frame_rate = nr_find_arg_double(argc,argv,"video",1, ss.frame_rate);
	ss.quality = nr_find_arg_double(argc,argv,"video",2,ss.quality);
	ss.pixel_size = nr_find_arg_int(argc,argv,"video",3,ss.pixel_size);

	ss.audio_compressor = nr_find_arg_ostype(argc,argv,"audio",0,ss.audio_compressor);
	ss.sample_rate = nr_find_arg_double(argc,argv,"audio",1,ss.sample_rate);
	ss.sample_size = nr_find_arg_int(argc,argv,"audio",2,ss.sample_size);
	ss.channel_count = nr_find_arg_int(argc,argv,"audio",3,ss.channel_count);

	// | going backwards, let some audio/video compressors override
	// | the exporter choice. 

	switch(ss.audio_compressor)
		{
		case 'aiff':
		case 'AIFF':
			ss.exporter_subtype = 'AIFF';
			ss.exporter_mfr = 'soun';
			ss.audio_compressor = 'twos';
		common_tail_1:
			ss.video_compressor = 0;
			nr_printf(3,"# (setting exporter to %s,%s, fmt to %s)\n",
					o2c(ss.exporter_subtype),
					o2c(ss.exporter_mfr),
					o2c(ss.audio_compressor));
			ss.exporter_audio = 1;
			break;

		case 'wav ':
		case 'WAV ':
		case 'wave':
		case 'WAVE':
			ss.exporter_subtype = 'WAVE';
			ss.exporter_mfr = 'soun';
			ss.audio_compressor = 'raw ';
			goto common_tail_1;
		}
	switch(ss.video_compressor)
		{
		case 'mpg2':
		case 'MPG2':
		case 'mpeg':
		case 'MPEG':
			ss.exporter_subtype = 'MPEG';
			ss.exporter_mfr = 0;
			ss.exporter_audio = 1;
			ss.exporter_video = 1;
			break;

		case 'mpg1':
		case 'MPG1':
		case 'mpg ':
		case 'MPG ':
			ss.exporter_subtype = 'MPG ';
			ss.exporter_mfr = 'Roxi';
			ss.exporter_audio = 1;
			ss.exporter_video = 1;
			break;
		}

	ss.data_rate = nr_find_arg_double(argc,argv,"datarate",0,ss.data_rate);
	ss.key_frame_rate = nr_find_arg_int(argc,argv,"keyframerate",0,ss.key_frame_rate);

	ss.start_time = nr_find_arg_double(argc,argv,"duration",0,ss.start_time);
	ss.end_time = nr_find_arg_double(argc,argv,"duration",1,ss.end_time);

	if(ss.end_time <= ss.start_time)
		{
		ss.end_time = ss.start_time;
		ss.start_time = 0;
		}
    
    if(ss.forced_duration > 0)
        ss.end_time = ss.start_time;

    
    ss.width = nr_find_arg_int(argc,argv,"size",0,ss.width);
    ss.height = nr_find_arg_int(argc,argv,"size",1,ss.height);

go_home:
	if(!err)
		*ssOut = ss;
	return err;
}


int doRoxiAtomicSetup(qte_parameter_settings *ss,ComponentInstance ci)
{
	// +------------------------------
	// | Special default roxio params
	int err = 0;
	if(ss->exporter_subtype == 'MPG '
			&& ss->exporter_mfr == 'Roxi')
		{
		QTAtomContainer roxi_ac;
		roxio_RtvS rtvs = {0};

		rtvs.format = 0; // ntsc
		rtvs.shape = 1; // fit
		rtvs.launch_toast = 0; // its already zero, but I feel so strongly
		rtvs.quality = 2; // better

		err = QTNewAtomContainer(&roxi_ac);
		obailerr(err,"Could not create atom container");

		// must be atom id 1000
		err = nr_insert_deep_atom_data_id(roxi_ac,"RtvS",sizeof(rtvs),&rtvs,1000);
		obailerr(err,"Could not insert RtvS atom for Roxio MPEG");

		err = MovieExportSetSettingsFromAtomContainer(ci,roxi_ac);
		obailerr(err,"Could not set settings for Roxio MPEG");

		QTDisposeAtomContainer(roxi_ac); // polite, not that it matters
		}
go_home:
	return err;
}


// mo is the source movie
int doMovieSequenceImport(ComponentInstance ci,Movie mo,qte_parameter_settings *ss)
{
	int err = 0;
	// analyze filename of source to discern the numbering pattern of the files...

	sequence_stuff seqstuff = {0};
	nr_filename_to_sequence_stuff(ss->source_movie_name,&seqstuff);
	if(seqstuff.digitCount < 1)
		bail(1,nr_sprintf("filename \"%d\" is unnumbered, cant import sequence",ss->source_movie_name));

	nr_printf(1,"# reading sequence frames, %d/%d per frame",ss->ticks_per_frame,ss->ticks_per_second);
//	Rect box;
//	GetMovieBox(mo,&box);
	long timeScale = GetMovieTimeScale(mo);  // we will use the source time scale. we have to?
	//printf("timeScale = %d\n",timeScale);
	//printf("src timeScale = %d, dst timeScale = %d/%d\n",timeScale,ss->ticks_per_frame,ss->ticks_per_second);

	Movie outMo = NewMovie(0);
	//SetMovieTimeScale(outMo,timeScale);
	nr_set_movie_time_scale(outMo,ss->ticks_per_second);

	int i;
	int frameCount = 0;
	
	// copy of parameter settings just to pass current frame filename
	// to the progress proc, while still getting the 2% throttling
	qte_parameter_settings ssCopy = *ss;
	for(i = 0; i < seqstuff.indexMax; i++)
	{
		char *sequenceFileName = nr_sequence_stuff_to_filename(&seqstuff,i);
        // we expect this to often fail, and return 0

		Movie aFrameMovie = nr_new_movie_from_file(sequenceFileName);
		if(aFrameMovie != 0)
		{
			nr_printf( 2, "Found %s\n", sequenceFileName); // Thanks Chris!
            nr_set_movie_time_scale(aFrameMovie,ss->ticks_per_second);
			long spot = GetMovieDuration(outMo);
			err = InsertMovieSegment(aFrameMovie,outMo,0,1,spot);
			obailerr(err,"InsertMovieSegment");
			
			DisposeMovie(aFrameMovie); // bye, frame.

			//err = ScaleMovieSegment(outMo,spot,1,timeScale / ss->sequence_rate);
			err = ScaleMovieSegment(outMo,spot,1,ss->ticks_per_frame);
			//err = ScaleMovieSegment(outMo,spot,1, ss->ticks_per_frame);
			obailerr(err,"ScaleMovieSegment");
			
			frameCount++;
			ssCopy.source_movie_name = nr_sprintf("found: %s (%d)",sequenceFileName,frameCount);
		}
		else
			ssCopy.source_movie_name = nr_sprintf("checking: %s (%d)",sequenceFileName,frameCount);
		

		r_progress_proc(0,1,1,i * 65536 / seqstuff.indexMax,(long)(&ssCopy));
	}
	nr_printf(1,"# found %d frames",frameCount);
	nr_printf(1,"# exporting movie");

	err = doMovieExport(ci,outMo,ss);

//	FSSpec fs;
//	err = NativePathNameToFSSpec(ss->export_movie_name,&fs,0);
//	if(err == fnfErr) err = 0; // mixed doc on whether you get fnfErr with this call!
//	obailerr(err,"NativePathNameToFSSpec");
//
//	err = ConvertMovieToFile(outMo,0,&fs,'MooV','TVOD',0,0,0,0);
//	obailerr(err,"ConvertMovieToFile");

go_home:
    return err;
}





// +---------------------------------------
// |
// |

int doMovieExport(ComponentInstance ci,Movie mo,qte_parameter_settings *ss)
{
	int err = 0;
	if(mo)
		{
		FSSpec fs;
		Boolean valid;
		TimeValue start_time;
		TimeValue duration;
		TimeScale movie_time_scale;

		// |
		// | figure offset & duration, with some funky
		// |

		movie_time_scale = GetMovieTimeScale(mo);
nr_printf(2,"# doMovieExport source movie_time_scale = %d\n",(int)movie_time_scale); //!!!
nr_printf(2,"# doMovieExport ss frame_rate = %f\n",ss->frame_rate); //!!!
		start_time = (int)(ss->start_time * movie_time_scale);
		duration = (int)((ss->end_time - ss->start_time) * movie_time_scale);

		if(duration <= 0)
			{
			start_time = 0;
			duration = GetMovieDuration(mo);
			}
        
        if(ss->forced_duration)
            duration = 1;

		// |
		// | Some abort conditions...
		// |

		if(ss->video_compressor == 'QDM2'
				&& !ss->do_dialog
				&& !ss->load_settings)
			{
			err = -1;
			obailerr(err,"QDM2 audio requires a settings file or setup dialog");
			}

		// |
		// | See about blasting the existing file
		// |

		err = NativePathNameToFSSpec(ss->export_movie_name,&fs,0);
		if(err == fnfErr) err = 0; // mixed doc on whether you get fnfErr with this call!
		//!!! put back obailerr(err,nr_sprintf("xx NativePathNameToFSSpec %d",err));

		err = nr_file_exists(ss->export_movie_name);
		if(err == 0) // file already exists!
			{
			if(ss->replace_file)
				{
				err = FSpDelete(&fs);
				obailerr(err,nr_sprintf("could not delete existing file %s",ss->export_movie_name));
				}
			else
				{
				err = -1;
				obailerr(err, nr_sprintf("file %s already exists, consider --replacefile",
						ss->export_movie_name));
				}
			}
		else if(err != fnfErr) // we hope for fnfErr
			{
			obailerr(err,nr_sprintf("file error %d",err));
			}

		err = MovieExportValidate(ci,mo,0,&valid);
		nr_printf(3,"movie validate: %d / %d\n",err,valid);
		if(!valid & !err)
			err = -1;
		obailerr(err,"invalid settings");

		// |
		// | Create the receiving file
		// |

		err = FSpCreate(&fs,'TVOD',0,smSystemScript);
		obailerr(err,"Could not create new file");
		
		// |
		// | And at last, export the new movie (or other
		// | file type)
		// |

		nr_printf(3,"%d ci=: 0x%08x\n",__LINE__,ci);
		err = MovieExportToFile(ci,&fs,mo,0,start_time,duration);
		obailerr(err,"MovieExportToFile failed");
		}
go_home:
	return err;
}



//int main(int argc,char *argv[])
main(int argc,char **argv)
	{
	int i;
	OSErr err = 0;
	ComponentInstance ci;
	int found_it = 0;
	qte_parameter_settings ss = {0};

	setbuf(stdout,0);

	if(argc == 1)
		usage();

	found_it = nr_find_arg(argc,argv,"help",0,0);
	if(found_it)
		usage();

	if(nr_find_arg(argc,argv,"man",0,0))
		man();
		
	if(nr_find_arg(argc,argv,"version",0,0))
		version();

	nr_printf(1,"");

	r_args_to_settings(argc,argv,&ss);

	// +---------------------------
	// | all done reading argc and argv. no more allowed.
	// |
	argc = 0;
	argv = 0;

	nr_enter_movies();
	
	// |
	// | Open up the movie (if any)
	// |

	Movie mo = 0;
	if(ss.source_movie_name)
		{
		mo = nr_new_movie_from_file(ss.source_movie_name);
		obailnil(mo,nr_sprintf("could not open movie %s",ss.source_movie_name));
		}

    // |
    // | fix up width & height selection, maybe
    // |
    if(ss.width > 0 && mo && ss.height == 0)
    {
        Rect box;
        GetMovieBox(mo,&box);
        OffsetRect(&box,-box.left,-box.top);
        double movieWidth = box.right;
        double movieHeight = box.bottom;
        if(movieWidth > 0 && movieHeight > 0)
            ss.height = movieHeight * ss.width / movieWidth;
    }
    


	ci = nr_open_component( 'spit',ss.exporter_subtype,ss.exporter_mfr);
	nr_printf(3,"ci is 0x%08x\n",ci);
    if(ci == 0)
    {
        // try not-appl also
        ci = nr_open_component( 'spit',ss.exporter_subtype,0);
    }
	obailnil(ci,
			nr_sprintf("could not open export component %s:%s (%08x:%08x)",
					o2c(ss.exporter_subtype),
					o2c(ss.exporter_mfr),
					ss.exporter_subtype,
					ss.exporter_mfr));


	// A special setup for roxio
	err = doRoxiAtomicSetup(&ss,ci);
	obailerr(err,"setup for Roxio mpg export");

	// |
	// | done with any special preloadery
	// +-------------------------------------

    // | various atomic things

    if(ss.b_supports_atoms)
        {

        // |
        // | loading from existing settings? do so then.
        // |

        if(ss.load_settings)
            {
            err = nr_file_to_atom_container(ss.load_settings,&ss.settings);
            obailerr(err,nr_sprintf("could not load settings file %s",ss.load_settings));

            err = r_atom_container_to_parameter_settings(ss.settings,&ss);
            obailerr(err,"could not extract settings from settings file atom");
            }
        else if(!ss.settings)
            {
            // |
            // | otherwise, get a starter-atom from the exporter component
            // |

            err = MovieExportGetSettingsAsAtomContainer(ci,&ss.settings);
            obailerr(err,"Initial MovieExportGetSettingsAsAtomContainer");
            }

        // |
        // | Convert settings to Atom, and possibly invoke the
        // | the export dialog
        // |

        // | IF you are using load_settings or do_dialog, 
        // | THEN we make no attempt to use the command line switches.
        // | We trust completely the dialog or stashed settings.
        // |

        if((ss.exporter_subtype == 'MooV') && (!ss.dont_put_params_atop_atom)) // we can disable the atop-ness 
            {
            // really, this is the exception -- being able to manipulate the
            // exporter settings atom. probably only vaguely feasible for -->.mov.
            err = r_parameter_settings_atop_atom_container(&ss,ss.settings);
            obailerr(err,"r_parameter_settings_atop_atom_container");
            }

        err = MovieExportSetSettingsFromAtomContainer(ci,ss.settings);
        obailerr(err,"MovieExportSetSettingsFromAtomContainer");
        } // b_supports_atoms

	if(ss.do_dialog || ss.forced_dialog)
		{
		Boolean canceled;

		nr_printf(1,"\n# (Settings dialog may be hidden by this terminal window...)\n\n");

            {
            // thank you sef!
            // sean writes:
            // Courtesy of Deric Horn and myself.
            // 2004.05.28
            ProcessSerialNumber psn;

            err = GetCurrentProcess(&psn);
            if (err == noErr)
                {
                (void)SetFrontProcess(&psn);
                }
            }

		TimeValue movie_duration = mo ? GetMovieDuration(mo) : 0;
		err = MovieExportDoUserDialog(ci,mo,nil,0,movie_duration,&canceled);
		bailerr(err,"MovieExportDoUserDialog");

		if(canceled)
			{
            if(ss.b_ignore_user_canceled)
                printf("# (ignoring MovieExportDoUserDialog canceled)\n");
            else
                {
                err = -1;
                obailerr(err,"user canceled");
			    }
			}
		}

	// | get the atom container from the export component, in case
	// | the exporter changed it trickily...

    if(ss.b_supports_atoms)
        {
        QTDisposeAtomContainer(ss.settings);
        err = MovieExportGetSettingsAsAtomContainer(ci,&ss.settings);
        obailerr(err,"MovieExportGetSettingsAsAtomContainer");

        // | one last conversion back to ss, for printing

        r_atom_container_to_parameter_settings(ss.settings,&ss);

        // | and maybe save those settings.

        if(ss.save_settings)
            {
            err = nr_atom_container_to_file(ss.settings,ss.save_settings);
            obailerr(err,nr_sprintf("could not save settings file %s",ss.save_settings));
            }

// ??        QTDisposeAtomContainer(ss.settings);
// ??        ss.settings = 0;
        } // b_supports_atoms

	r_print_parameter_settings(&ss);

	err = MovieExportSetProgressProc(ci,r_progress_proc,(long)&ss);
	//obailerr(err,"MovieExportSetProgressProc");
    if(err < 0)
        {
	    nr_printf(1,"# (MovieExportSetProgressProc: %d)",err);
        err = 0;
        }
	
	// |
	// | If we are actually exporting, do it now
	// |
	
    if(ss.sequence_rate >= 0)
        doMovieSequenceImport(ci,mo,&ss);
    else
	    doMovieExport(ci,mo,&ss);
	

go_home:
	if(err)
		fprintf(stderr,"### error is %d\n",err);

	if(ci)
		CloseComponent(ci);
	if(mo)
		DisposeMovie(mo);

	nr_printf(1,"");
	if(err)
		exit(1);

	exit(0);
	}

// end of file
