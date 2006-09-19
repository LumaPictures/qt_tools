// file: qt_info.c
//
// A command line utility to show you interesting things
// about a movie file
//

#include "qtc_utils.h"
#include "qtc_manpages.h"
#include <stdio.h>


#define k_max_sample_descriptions 100    // silly-high

// |
// | inbuilt data -- names of QuickTime annotations
// | for text display
// |

typedef struct
	{
	OSType k;              // | constant for movies toolbox
	char *n;               // | pretty user string
	char *switch_name;     // | crushed cmd-line switch name
	} s_annotation_entry;

s_annotation_entry d_annotations[] =
	{
	kUserDataTextAlbum                      ,             "Album",             "album",
	kUserDataTextArtist                     ,            "Artist",            "artist",
	kUserDataTextAuthor                     ,            "Author",            "author",
	kUserDataTextChapter                    ,           "Chapter",           "chapter",
	kUserDataTextComment                    ,           "Comment",           "comment",
	kUserDataTextComposer                   ,          "Composer",          "composer",
	kUserDataTextCopyright                  ,         "Copyright",         "copyright",
	kUserDataTextCreationDate               ,     "Creation Date",      "creationdate",
	kUserDataTextDescription                ,       "Description",       "description",
	kUserDataTextDirector                   ,          "Director",          "director",
	kUserDataTextDisclaimer                 ,        "Disclaimer",        "disclaimer",
	kUserDataTextEncodedBy                  ,        "Encoded By",         "encodedby",
	kUserDataTextFullName                   ,         "Full Name",          "fullname",
	kUserDataTextGenre                      ,             "Genre",             "genre",
	kUserDataTextHostComputer               ,     "Host Computer",      "hostcomputer",
	kUserDataTextInformation                ,       "Information",       "information",
	kUserDataTextKeywords                   ,          "Keywords",          "keywords",
	kUserDataTextMake                       ,              "Make",              "make",
	kUserDataTextModel                      ,             "Model",             "model",
	kUserDataTextOriginalArtist             ,   "Original Artist",    "originalartist",
	kUserDataTextOriginalFormat             ,   "Original Format",    "originalformat",
	kUserDataTextOriginalSource             ,   "Original Source",    "originalsource",
	kUserDataTextPerformers                 ,        "Performers",        "performers",
	kUserDataTextProducer                   ,          "Producer",          "producer",
	kUserDataTextProduct                    ,           "Product",           "product",
	kUserDataTextSoftware                   ,          "Software",          "software",
	kUserDataTextSpecialPlaybackRequirements,"Special Playback Requirements","specialplaybackrequirements",
	kUserDataTextTrack                      ,             "Track",             "track",
	kUserDataTextWarning                    ,           "Warning",           "warning",
	kUserDataTextWriter                     ,            "Writer",            "writer",
	kUserDataTextURLLink                    ,          "URL Link",           "urllink",
	0,0,0
	};

void man(void)
	{
	nr_print_version(__FILE__);
	#ifdef print_qt_info_man
		print_qt_info_man
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

	U("qt_info is a command line tool for");
	U("setting or showing information about a");
	U("a QuickTime movie.");
	Uu
	U("usage: qt_info [options] <movie file>");
	Uu
	U("Options are used to set properties. If");
	U("no options are given, information about");
	U("the movie is printed.");
	Uu

		{
		s_annotation_entry *w = d_annotations;
		while(w->k)
			{
			printf(":   --%s=\"<%s>\"\n",w->switch_name,w->n);
			w++;
			}
		}

	Uu
	U("\"qt_info --man | more\" will tell you the");
	U("same thing with a lot more words.");
	Uu

	nr_printf(1,"");
	exit(0);
	}


typedef struct
	{
	int track_count;

	TimeValue duration;
	TimeValue time_scale;
	Fixed rate;
	Fixed preferred_rate;

	short volume;
	short preferred_volume;

	Rect box;
	Rect natural_bounds;
	RgnHandle display_clip_rgn;
	RgnHandle clip_rgn;
	RgnHandle display_bounds_rgn;
	} s_movie_info;

int r_gather_movie_info(Movie mo, s_movie_info *info_out)
	{
	OSErr err = 0;

	info_out->track_count = GetMovieTrackCount(mo);

	info_out->duration = GetMovieDuration(mo);
	info_out->time_scale = GetMovieTimeScale(mo);
	info_out->rate = GetMovieRate(mo);
	info_out->preferred_rate = GetMoviePreferredRate(mo);

	info_out->volume = GetMovieVolume(mo);
	info_out->preferred_volume = GetMoviePreferredVolume(mo);

	GetMovieBox(mo,&info_out->box);
	GetMovieNaturalBoundsRect(mo,&info_out->natural_bounds);
	info_out->display_clip_rgn = GetMovieDisplayClipRgn(mo);
	info_out->clip_rgn = GetMovieClipRgn(mo);
	info_out->display_bounds_rgn = GetMovieDisplayBoundsRgn(mo);

	return err;
	}

int r_print_movie_info(s_movie_info *info)
	{
	nr_print_time("movie duration",info->duration,info->time_scale);
	nr_print_rate("movie rate",info->rate);
	nr_print_rate("movie preferred rate",info->preferred_rate);
	nr_print_volume("movie volume",info->volume);
	nr_print_volume("movie preferred volume",info->preferred_volume);
	nr_print_rect("movie box",&info->box);
	nr_print_rect("movie natural bounds",&info->natural_bounds);
	nr_print_rgn("movie display clip rgn",info->display_clip_rgn);
	nr_print_rgn("movie clip rgn",info->clip_rgn);
	nr_print_rgn("movie display bounds rgn",info->display_bounds_rgn);
	nr_print_int("movie track count",info->track_count);

	return 0;
	}

int r_print_movie_annotations(Movie mo)
	{
	int err = 0;
	s_annotation_entry *w = d_annotations;
	UserData  user_data = GetMovieUserData(mo);

	bailnil(user_data,"GetMovieUserData");

	while(w->k)
		{
		Handle h = NewHandle(0);
		err = GetUserDataText(user_data, h, w->k, 1, GetScriptManagerVariable(smRegionCode));
		if(!err)
			nr_print_info(w->n,h2c(h));

		DisposeHandle(h); // I never dispose handles
		w++;
		}
	err = 0;

go_home:
	return err;
	}

typedef struct
	{
	// |
	// | Track Info
	// |

	long index;
	long id;

	unsigned long creation_time;
	unsigned long modification_time;
	int enabled;
	long usage;
	short layer;
	long alternate_id;

	short volume;

	TimeValue offset; // start time
	TimeValue duration;

	Fixed width;
	Fixed height;

	RgnHandle clip_rgn;
	RgnHandle display_bounds_rgn;
	RgnHandle movie_bounds_rgn;
	RgnHandle bounds_rgn;

	// |
	// | Media Info
	// |

	int has_media;
	TimeScale media_time_scale;
	TimeValue media_duration;
	short media_language;
	short media_quality;
	OSType media_type;
	Str255 media_creator_name;
	OSType media_creator_manufacturer;
	OSType media_data_type;
	Str255 media_data_creator_name;
	OSType media_data_creator_manufacturer;

    Str255 media_file_name;
	long media_sample_description_count;
	long media_sample_count;
	long media_sync_sample_count;
	long media_preferred_chunk_size;

	SampleDescriptionHandle sample_description_h[k_max_sample_descriptions];
	} s_track_info;

int r_gather_track_info(Movie mo, int index, s_track_info *info_out)
	{
	Track tr;
	Media me;
	OSErr err = 0;
	int i;

	memset(info_out,0,sizeof(s_track_info));

	tr = GetMovieIndTrack(mo,index);
	bailnil(tr,"GetMovieIndTrack");
	me = GetTrackMedia(tr);

	info_out->index = index;
	info_out->id = GetTrackID(tr);

	info_out->creation_time =  GetTrackCreationTime(tr);
	info_out->modification_time = GetTrackModificationTime(tr);
	info_out->enabled = GetTrackEnabled(tr);
	info_out->usage = GetTrackUsage(tr);
	info_out->layer = GetTrackLayer(tr);

		{
		Track alt_tr = GetTrackAlternate(tr);
		info_out->alternate_id = alt_tr ? GetTrackID(alt_tr) : 0;
		}

	info_out->volume = GetTrackVolume(tr);
	info_out->offset = GetTrackOffset(tr);
	info_out->duration = GetTrackDuration(tr);

	GetTrackDimensions(tr,&info_out->width,&info_out->height);

	info_out->clip_rgn = GetTrackClipRgn(tr);
	info_out->display_bounds_rgn = GetTrackDisplayBoundsRgn(tr);
	info_out->movie_bounds_rgn = GetTrackMovieBoundsRgn(tr);
	info_out->bounds_rgn = GetTrackBoundsRgn(tr);
	
	if(me)
		{
        DataHandler dh;
		info_out->has_media = 1;
		info_out->media_time_scale = GetMediaTimeScale(me);
		info_out->media_duration = GetMediaDuration(me);
		info_out->media_language = GetMediaLanguage(me);
		info_out->media_quality = GetMediaQuality(me);
		
		GetMediaHandlerDescription(me,
				&info_out->media_type,
				info_out->media_creator_name,
				&info_out->media_creator_manufacturer);

		GetMediaDataHandlerDescription(me,1,
				&info_out->media_data_type,
				info_out->media_data_creator_name,
				&info_out->media_data_creator_manufacturer);

        dh = GetMediaDataHandler(me,1);

        info_out->media_file_name[0] = 0;
        if(dh)
            {
            err = DataHGetFileName(dh,info_out->media_file_name);
            bailerr(err,"DataHGetFileName");
            }

		info_out->media_sample_description_count = GetMediaSampleDescriptionCount(me);
		info_out->media_sample_count = GetMediaSampleCount(me);
		info_out->media_sync_sample_count = GetMediaSyncSampleCount(me);
		err = GetMediaPreferredChunkSize(me,&info_out->media_preferred_chunk_size );
		bailerr(err,"GetMediaPreferredChunkSize");

		for(i = 0; i < info_out->media_sample_description_count; i++)
			{
			info_out->sample_description_h[i] = (SampleDescriptionHandle) NewHandle(0);

			bailnil(info_out->sample_description_h[i] ,"NewHandle");
			GetMediaSampleDescription(me,i + 1,info_out->sample_description_h[i]);
			}
		}

go_home:
	return err;
	}

char *r_get_component_name(OSType ct,OSType cst,OSType cmfr)
	{
	ComponentDescription cd = {0};
	Component co;
	Handle component_name_h = NewHandle(0);

	cd.componentType = ct;
	cd.componentSubType = cst;
	cd.componentManufacturer = cmfr;

	co = FindNextComponent(0,&cd);
	if(!co)
		return "-";

	GetComponentInfo(co,&cd,component_name_h,0,0);
	if(!component_name_h)
		return "-";
	return p2c((StringPtr)*component_name_h);
	}

int r_print_sample_description(s_track_info *tinfo,int sample_description_index)
	{
	SampleDescriptionPtr sd_p = *tinfo->sample_description_h[sample_description_index];

	nr_print_int_of_int("--- sample description",
			sample_description_index + 1,
			tinfo->media_sample_description_count);

	// |
	// | can only really print audio or video sample descriptions
	// |
	if(tinfo->media_type == 'soun')
		{
		SoundDescription *sd = (SoundDescription *) sd_p;
		nr_print_info("sound format",
				nr_sprintf("%s %s",
						o2c(sd->dataFormat),
						r_get_component_name('scom',sd->dataFormat,sd->vendor)));

		nr_print_int2("bits/channels",sd->sampleSize,sd->numChannels);
		nr_print_int("sampling rate",sd->sampleRate >> 16);
		}
	else if(tinfo->media_type == 'vide')
		{
		ImageDescription *vd = (ImageDescription *) sd_p;
		nr_print_info("video format",
				nr_sprintf("%s %s",
						o2c(vd->cType),
						r_get_component_name('imco',vd->cType,vd->vendor)));
		nr_print_int2("temporal/spatial quality",
				vd->temporalQuality,
				vd->spatialQuality);
		nr_print_int3("width/height/depth",vd->width,vd->height,vd->depth);
		}
	else
		{
		nr_print_info("format",nr_sprintf("unknown, %d bytes",sd_p->descSize));
		}
	}


int r_print_track_info(s_track_info *tinfo,s_movie_info *movie_info)
	{
	char s[k_string_size];
	int i;

	nr_print_cr();
	nr_print_int("track index",tinfo->index);
	nr_print_int("track id",tinfo->id);
	nr_print_createmodtime("track create/mod time",tinfo->creation_time,tinfo->modification_time);
	nr_print_yesno("track enabled",tinfo->enabled);
	nr_print_int3("track usage/layer/alt id",tinfo->usage,tinfo->layer,tinfo->alternate_id);

	nr_print_volume("track volume",tinfo->volume);

	nr_print_time("track offset",tinfo->offset,movie_info->time_scale);
	nr_print_time("track duration",tinfo->duration,movie_info->time_scale);

	nr_print_dimensions("track dimensions",tinfo->width,tinfo->height);

	nr_print_rgn("track clip rgn",tinfo->clip_rgn);
	nr_print_rgn("track display bounds rgn",tinfo->display_bounds_rgn);
	nr_print_rgn("track movie bounds rgn",tinfo->movie_bounds_rgn);
	nr_print_rgn("track bounds rgn",tinfo->bounds_rgn);

	if(tinfo->has_media)
		{
		nr_print_time("media duration",tinfo->media_duration,tinfo->media_time_scale);
		nr_print_int("media language",tinfo->media_language);
		nr_print_int("media quality",tinfo->media_quality);
		nr_print_handler("media handler",
				tinfo->media_type,
				tinfo->media_creator_manufacturer,
				tinfo->media_creator_name);
		nr_print_handler("media data handler",
				tinfo->media_data_type,
				tinfo->media_data_creator_manufacturer,
				tinfo->media_data_creator_name);
		nr_print_int("media description count",tinfo->media_sample_description_count);
		nr_print_int("media sample count",tinfo->media_sample_count);
		nr_print_int("media sync sample count",tinfo->media_sync_sample_count);
		nr_print_int("media preferred chunk size",tinfo->media_preferred_chunk_size);

        nr_print_info("media file name",p2c(tinfo->media_file_name));

		for(i = 0; i < tinfo->media_sample_description_count; i++)
			r_print_sample_description(tinfo,i);
		}



	}

int r_show_movie_info(char *filename)
	{
	OSErr err;
	s_movie_info movie_info;
	Movie mo = nr_new_movie_from_file(filename);
	int i;

	bailnil(mo,"nr_new_movie_from_file");

	nr_print_info("movie name",filename);
	err = r_gather_movie_info(mo,&movie_info);
	bailerr("r_gather_movie_info",err);

	err = r_print_movie_info(&movie_info);
	bailerr("r_print_movie_info",err);
	err = r_print_movie_annotations(mo);
	bailerr("r_print_movie_annotations",err);

	for(i = 1; i <= movie_info.track_count; i++)
		{
		s_track_info track_info;

		err = r_gather_track_info(mo,i,&track_info);
		bailerr(err,"r_gather_track_info");

		err = r_print_track_info(&track_info,&movie_info);
		bailerr(err,"r_print_track_info");
		}
	nr_print_cr();
	
go_home:
	if(mo)
		DisposeMovie(mo);
	else
		err = -1;

	return err;
	}



// +-----------------------------------------
// | r_handle_property_changes(argc,argv)
// |
// | If any property-changing options are specified,
// | then do them. open up the movie on demand; if no 
// | options are found, then dont open the movie, nor
// | of course save it at the end.
// |

int r_open_movie_if_necessary(char *filename,Movie *m_inout,short *resrefnum_inout, short *resid_inout)
	{
	int err;

	if(!*m_inout)
		{
		*m_inout = nr_new_movie_from_file_x(filename,resrefnum_inout,resid_inout);
		bailnil(*m_inout,nr_sprintf("could not open movie %s",filename));
		}
go_home:
	return err;
	}

int r_handle_property_changes(char *filename,int argc,char *argv[])
	{
	s_annotation_entry *w = d_annotations;
	int foundit;
	char *annotation_value;
	Movie mo = 0;
	short resrefnum = 0;
	short resid = 0;
	short err = 0;

	// |
	// | march through all the switches for annotations
	// |
	while(w->k)
		{
		foundit = nr_find_arg(argc,argv,w->switch_name,0,&annotation_value);
		if(foundit)
			{
			UserData user_data;

			err = r_open_movie_if_necessary(filename,&mo,&resrefnum,&resid);
			bailerr(err,"could not open movie");

			user_data = GetMovieUserData(mo);
			err = AddUserDataText
					(
					user_data,
					c2h(annotation_value),
					w->k,
					1,
					GetScriptManagerVariable(smRegionCode)
					);
			bailerr(err,"AddUserDataText");
			}

		w++;
		}

	// |
	// | got here, no errors: write out the movie if necessary
	// |

	if(mo)
		{
		err = UpdateMovieResource(mo,resrefnum,resid,0);
		bailerr(err,"UpdateMovieResource");
		}

go_home:
	if(mo)
		CloseMovieFile(resrefnum);
	return err;
	}

int main(int argc,char *argv[])
	{
	int foundit;
	char *filename;
	int err = 0;

	setbuf(stdout,0);

	if(nr_find_arg(argc,argv,"man",0,0))
		man();

	if(nr_find_arg(argc,argv,"version",0,0))
		version();

	g_verbosity = nr_find_arg_int(argc,argv,"verbosity",0,1);

	foundit = nr_find_arg(argc,argv,"1",0,&filename);
	if(!foundit)
		usage();

	foundit = nr_find_arg(argc,argv,"help",0,0);
	if(foundit)
		usage();

	err = r_handle_property_changes(filename,argc,argv);
	obailerr(err,nr_sprintf("could not set properties on %s",filename));

	r_show_movie_info(filename);

go_home:
	fflush(stdout);
	return err;
	}

// end of file
