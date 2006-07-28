// file: qt_proofsheet.c
//
// started:
// 2005 September 22 dvb
// 2006 March 11 dvb lots of featurefulness now, labels & spacing...
//
// A command line utility for rendering a
// QuickTime movie into a "proof sheet" mesh
//

#include <stdio.h>
#include <CoreServices/CoreServices.h>
#include <QuickTime/QuickTime.h>

#include "qtc_utils.h"
#include "qtc_manpages.h"

#define k_string_size 1024

// +-----------------------------
// |
// | settings from the --parameters
// |

typedef struct
	{
	// files (pointers to argv strings)
	char *source_movie_name;
    char *output_sheet_name;
	
	int timeCodeSize; // point size for time code printing
	int timeCodeCorner; //LL,LR,UR,UL,Lctr
	int titleSize; // point size for text at the top...

	int frameWidth;
    int framesPerRow;
	int spacing; // spacing between frame, and at edge

	double startTime;
	double endTime;
	double duration;
    int replace_file; // 1: ok to delete

	// computed values
	int frameHeight;
	int frameCount;
	int rowCount;
    int sheetWidth;
    int sheetHeight;
	
	CodecType codecType; 
	
	double framesPerSecond; // how often to sample
    } s_parameter_settings;

Rect bigRect = {-30000,-30000,30000,30000};

// man and version, templatic
void man(void)
	{
	nr_print_version(__FILE__);
	#ifdef print_qt_proofsheet_man
		print_qt_proofsheet_man
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

	U("qt_proofsheet is a command line tool")
	U("for rendering a QuickTime movie as a")
    U("single grid of frames.");
	Uu
	U("usage: qt_proofsheet [options] <source movie file> <exported picture file>")
	Uu
	U("(picture.jpg or picture.tga will automatically choose codec)")
	Uu
	U("options:")
    Ue("--framesize=<pixels wide[,pixels tall]> or small multipliers like 1.0 or 0.5")
    Ue("--framesperrow=<thumbnails per row>")
	Ue("--duration=<[start time,]end time>")
	Ue("--framerate=<frames[,per seconds]>")
	Ue("--codec=<codec id>")
	Ue("--spacing=<space between frames>")
	Ue("--timecode=<timecode imprint font size[,timecode position 0-4]> or 0")
	Ue("--title=<title font size>")
	Uu
	U("also:")
	Ue("\"qt_proofsheet --man | more\" for the full schpiel.")
	Uu
	U("by david van brink, poly@omino.com, subject line must begin with \"quicktime\"")
	Uu

	nr_printf(1,"");

	exit(0);
	}

char *timeToTimeCode(double t)
{
	int tt = (int)t;
	int thirtieths = (t - tt) * 30;
	
	int s = tt % 60;
	tt /= 60;
	int m = tt % 60;
	tt /= 60;
	int h = tt;
	
	char *c;
	c = nr_sprintf("%02d:%02d.%02d",m,s,thirtieths);
	return c;
}

#define paintByte(_srcColor,_dstPixel,_opacity,_byteMask) \
	_dstPixel = (_dstPixel & ~_byteMask) \
			| (_byteMask & (long)( ((_srcColor & _byteMask) * _opacity) \
				+ ((_dstPixel & _byteMask) * (1-_opacity))) )

void fadeRect(Rect *r,int doOuterFrame)
{
	GrafPtr g;
	GetPort(&g);
	PixMapHandle pmH = GetPortPixMap(g);
	char *pix = GetPixBaseAddr(pmH);
	int rowBytes = GetPixRowBytes(pmH);
	
	double opacity = .7;
	long color = 0xffFFFFFF; // white. highbyte is alpha, must be ff
	
	int x,y;
	// great, great. Now lets whack some pixels
	for(y = r->top; y < r->bottom; y++)
	{
		char *slab = pix + y * rowBytes;
		long *slabL = (long *)slab;
		for(x = r->left; x < r->right; x++)
		{
			long p = slabL[x];
			paintByte(color,p,opacity,0x000000ff);
			paintByte(color,p,opacity,0x0000ff00);
			paintByte(color,p,opacity,0x00ff0000);
			paintByte(color,p,opacity,0xff000000);
			slabL[x] = p;
		}
		
	}
	if(doOuterFrame)
	{
		// outside frame... aligns with edges...
		Rect rr = *r;
		InsetRect(&rr,-1,-1);
		FrameRect(&rr);
	}
}


void r_print_parameter_settings(s_parameter_settings *ss)
	{
	nr_print_cr();
    nr_print_dims("frame size",ss->frameWidth,ss->frameHeight);
    nr_print_int("spacing",ss->spacing);
    nr_print_int("frame count",ss->frameCount);
    nr_print_int("frames/row",ss->framesPerRow);
    nr_print_dims("sheet size",ss->sheetWidth,ss->sheetHeight);
	nr_print_ostype("codec",ss->codecType);
    char *s;
    if(ss->framesPerSecond == (int)ss->framesPerSecond)
        s = nr_sprintf("%d",(int)ss->framesPerSecond);
    else
        s = nr_sprintf("%.4f",ss->framesPerSecond);
    nr_print_info("frames per second",s);

	nr_print_d_time_range("duration",ss->startTime,ss->endTime);

	nr_print_cr();
    }

main(int argc,char **argv)
    {
	int i;
	Movie mo = 0;
	OSErr err = 0;
	int found_it = 0;
	s_parameter_settings ss = {0};

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


  	// |
	// | needs two movie arguments to work...
	// |

	found_it = nr_find_arg(argc,argv,"1",0,&ss.source_movie_name);
	found_it = nr_find_arg(argc,argv,"2",0,&ss.output_sheet_name);
	
	if(!ss.source_movie_name || !ss.output_sheet_name)
		{
		err = -1;
		obailerr(err,"You must provide a source movie and a destination picture.");
		}
	
	
	ss.replace_file = nr_find_arg_int(argc,argv,"replacefile",0,0);


	nr_enter_movies();
	
	// |
	// | Open up the movie
	// |

	if(ss.source_movie_name)
		{
		mo = nr_new_movie_from_file(ss.source_movie_name);
		obailnil(mo,nr_sprintf("could not open movie %s",ss.source_movie_name));
		}

    // |
    // | got movie, make sure we can replace dest
    // |

	FSSpec fs;
	err = NativePathNameToFSSpec(ss.output_sheet_name,&fs,0);
	if(err == fnfErr) err = 0; // mixed doc on whether you get fnfErr with this call!
	obailerr(err,"NativePathNameToFSSpec");

	err = nr_file_exists(ss.output_sheet_name);
	if(err == 0) // file already exists!
		{
		if(ss.replace_file)
			{
			err = FSpDelete(&fs);
			obailerr(err,nr_sprintf("could not delete existing file %s",ss.output_sheet_name));
			}
		else
			{
			err = -1;
			obailerr(err, nr_sprintf("file %s already exists, consider --replacefile",
					ss.output_sheet_name));
			}
		}
	else if(err != fnfErr) // we hope for fnfErr
		{
		obailerr(err,nr_sprintf("file error %d",err));
		}
	err = 0;


	// Get the movie basic info, so we can populate
	// the default sheet settings...
	TimeValue movie_duration = GetMovieDuration(mo);
    TimeScale movie_time_scale = GetMovieTimeScale(mo);
    double duration = (double)movie_duration / (double)movie_time_scale;  // in seconds
	Rect movieBox;
	GetMovieBox(mo,&movieBox);
	int movieWidth = movieBox.right - movieBox.left;
	int movieHeight = movieBox.bottom - movieBox.top;

	#define SHEET_WIDTH (144 * 8) // 144 dpi-ish
	#define SHEET_HEIGHT ((int)(144 * 9.5))


	// Read duration choices first, to influence default layout...
    ss.endTime = 0;
	ss.startTime = 0;
	ss.startTime = nr_find_arg_double(argc,argv,"duration",0,ss.startTime);
	ss.endTime = nr_find_arg_double(argc,argv,"duration",1,ss.endTime);
	if(ss.endTime == 0)
		{
		ss.endTime = ss.startTime;
		ss.startTime = 0;
		}

	// if omitted, match movie duration
	if(ss.startTime == 0 && ss.endTime == 0)
		ss.endTime = duration;
	if(ss.endTime > duration)
		ss.endTime = duration;

    ss.duration = ss.endTime - ss.startTime;

	// Try to fill the sheet exactly
	ss.frameWidth = movieWidth / 4; //movieWidth; // default...
	
	// frameSize = 100,.9 -- width 100, height .9 of whats implied
	// frameSize = 40,40 -- 40x40
	// frameSize = .5 -- half the movie size
	// frameSize = .5,1.0 --narrow and tall...

	double frameWidthD = nr_find_arg_double(argc,argv,"framesize",0,ss.frameWidth);
	int frameWidthIsMul = 0;
	if(frameWidthD < 2.0) // small number: treat as multiplier
	{
		ss.frameWidth = movieWidth * frameWidthD;
		frameWidthIsMul = 1;
	}
	else
		ss.frameWidth = frameWidthD;
	ss.frameHeight = movieHeight * ss.frameWidth / movieWidth;
	double frameHeightD = nr_find_arg_double(argc,argv,"framesize",1,ss.frameHeight);
	if(frameHeightD < 2.0)
	{
		if(frameWidthIsMul)
			ss.frameHeight = movieHeight * frameHeightD;
		else
			ss.frameHeight = ss.frameHeight * frameHeightD;
	}
	else
		ss.frameHeight = frameHeightD;
	
	ss.spacing = nr_find_arg_int(argc,argv,"spacing",0,ss.frameWidth / 10);
	ss.framesPerRow = (SHEET_WIDTH - ss.spacing) / (ss.frameWidth + ss.spacing);
	int frameRows = (SHEET_HEIGHT - ss.spacing) / (ss.frameHeight + ss.spacing);
	int frameCount = ss.framesPerRow * frameRows;
	ss.framesPerSecond = frameCount / duration;

	ss.framesPerRow = nr_find_arg_int(argc,argv,"framesperrow",0, ss.framesPerRow);
	ss.framesPerSecond = nr_find_arg_double(argc,argv,"framerate",0,ss.framesPerSecond);
	
	// computed values
	ss.frameCount = ss.duration * ss.framesPerSecond;
	ss.rowCount = (ss.frameCount + ss.framesPerRow - 1) / ss.framesPerRow;

    ss.codecType = kJPEGCodecType;
    OSType last4Chars = nr_last_four_chars(ss.output_sheet_name);
    if(last4Chars == '.tga')
        ss.codecType = kTargaCodecType;
	
	ss.codecType = nr_find_arg_ostype(argc,argv,"codec",0,ss.codecType);

	{
		ComponentInstance ci = nr_open_component('imco',ss.codecType,0);
//		obailnil(ci,nr_sprintf("No such codec \"%s\"",o2c(ss.codecType)));
	}
	
	ss.timeCodeSize = ss.spacing * 3 / 4;
	if(ss.timeCodeSize < 9)
		ss.timeCodeSize = 9;
	ss.timeCodeCorner = 1;
	ss.timeCodeSize = nr_find_arg_int(argc,argv,"timecode",0,ss.timeCodeSize);
	ss.timeCodeCorner = nr_find_arg_int(argc,argv,"timecode",1,ss.timeCodeCorner);

	ss.titleSize = 15;
	ss.titleSize = nr_find_arg_int(argc,argv,"title",0,ss.titleSize);
	
	// how much room to leave at the top, for title, label...
	int titleVerticalArea = 3 * ss.titleSize;

	ss.sheetWidth = ss.spacing + (ss.spacing + ss.frameWidth) * ss.framesPerRow;
	ss.sheetHeight = ss.spacing + (ss.spacing + ss.frameHeight) * ss.rowCount + titleVerticalArea;
	

    //------------------------------------------------------------------------
    // No More Parameter Fetching!
    argv = 0; argc = 0;
    //------------------------------------------------------------------------

    r_print_parameter_settings(&ss);
	

    {
        GWorldPtr gw;
        Rect r;

        r.top = r.left = 0;
        r.right = ss.sheetWidth;
        r.bottom = ss.sheetHeight;

        OSErr err = QTNewGWorld(
            &gw,
            32, //k32RGBAPixelFormat,
            &r,
            0,
            0,
            0);
		
		LockPixels(GetGWorldPixMap(gw));
        SetGWorld(gw,0);
		
		EraseRect(&bigRect);
		
		// and do each frame
		// This is the drawing portion of our show!
		int i;
		for(i = 0; i < ss.frameCount; i++)
		{
			printf(".");
			int row = i / ss.framesPerRow;
			int column = i % ss.framesPerRow;
			Rect r;
//			r.left = nr_map_int(column,0,ss.framesPerRow,0,ss.sheetWidth);
//			r.right = nr_map_int(column + 1,0,ss.framesPerRow,0,ss.sheetWidth);
//			r.top = nr_map_int(row,0,ss.rowCount,0,ss.sheetHeight);
//			r.bottom = nr_map_int(row + 1,0,ss.rowCount,0,ss.sheetHeight);
			
			r.left = ss.spacing + column * (ss.frameWidth + ss.spacing);
			r.right = r.left + ss.frameWidth;
			r.top = ss.spacing + row * (ss.frameHeight + ss.spacing) + titleVerticalArea;
			r.bottom = r.top + ss.frameHeight;

			// attractive shadow...
			if(ss.spacing > 8)
			{
				int x = ss.spacing / 4;
				Rect r2 = r;
				OffsetRect(&r2,x,x);
				GoGrey(200);
				PaintRect(&r2);
//				int xMax = x;
//				while(x > 0)
//				{
//					Rect r2 = r;
//					OffsetRect(&r2,x,x);
//					GoGrey(200 + 55 * x / xMax);
//					PaintRect(&r2);
//					
//					x--;
//				}
				GoBW();
			}

			
			double movieTimeF = ss.startTime + (double)i / (double)ss.framesPerSecond;
			long movieTime = movieTimeF * movie_time_scale;
			PicHandle p = GetMoviePict(mo,movieTime);
			EraseRect(&r);
			FrameRect(&r);
			DrawPicture(p,&r);
			KillPicture(p);
			
			if(ss.timeCodeSize > 0)
			{
				int margin = 2;
				int size = ss.timeCodeSize;
				
				MoveTo(r.left + margin,r.bottom - margin);
				TextSize(size);
				short fontNum;
				GetFNum(c2p("Monaco"),&fontNum);
				TextFont(fontNum);
				StringPtr s = c2p(nr_sprintf("t%4.4f",movieTimeF));
				s = c2p(timeToTimeCode(movieTimeF));
				Point p;
				GetPen(&p);
				
				Rect sR;
				sR.left = 0;
				sR.right = sR.left + StringWidth(s);
				sR.top = 0;
				sR.bottom = sR.top + size - 1;
				InsetRect(&sR,-2 * margin,-margin);
				switch(ss.timeCodeCorner)
				{
					case 0: // LL
						OffsetRect(&sR,r.left - sR.left,r.bottom - sR.bottom);
						break;
					case 1: // LR
						OffsetRect(&sR,r.right - sR.right,r.bottom - sR.bottom);
						break;
					case 2: // UR
						OffsetRect(&sR,r.right - sR.right,r.top - sR.top);
						break;
					case 3: // UL
						OffsetRect(&sR,r.left - sR.left,r.top - sR.top);
						break;
					case 4: // lower center
						{
						int xr = (r.right + r.left)/2;
						int xsr = (sR.right + sR.left)/2;
						OffsetRect(&sR,xr - xsr,r.bottom - sR.top + 1);
						}
				}
				
				MoveTo(sR.left + margin * 2,sR.bottom - margin - 1);

				if(ss.timeCodeCorner != 4) // dont erase if "lower center" pos
					fadeRect(&sR,ss.spacing > 0);
				DrawString(s);
			}
			
			if(ss.spacing)
			{
				InsetRect(&r,-1,-1);
				FrameRect(&r);
			}
		}
		printf("\n");
		
		//--
		// render some titles on the sheet
		if(ss.titleSize)
		{
			TextSize(ss.titleSize);
			int y = ss.titleSize * 5 / 3;
			MoveTo(20,y); y += ss.titleSize;
			DrawString(c2p(nr_sprintf("Source: %s",ss.source_movie_name)));
			MoveTo(20,y); y += ss.titleSize;
			DrawString(c2p(nr_sprintf("Time: %s to %s",timeToTimeCode(ss.startTime),timeToTimeCode(ss.endTime))));
		}
		//--

        PixMapHandle pmh = GetGWorldPixMap(gw);
        ImageDescriptionHandle idh = (ImageDescriptionHandle)NewHandle(0);

		Ptr compressed_image_data = 0;
			{
			CodecQ quality = codecHighQuality;
			long compressionMaxDataSize = 0;
			err = GetMaxCompressionSize (pmh,&r,0,quality,ss.codecType,0,&compressionMaxDataSize);

			//nr_printf(1,"x4 datasize = %ld\n",compressionMaxDataSize);

			compressed_image_data = NewPtrClear(compressionMaxDataSize);

			err = CompressImage (
				pmh,
				&r,
				codecHighQuality,
				ss.codecType,
				idh,
				compressed_image_data);
			}

        FILE *f = fopen(ss.output_sheet_name,"w+b");
        long len = (**idh).dataSize;
        fwrite(compressed_image_data,1,len,f);
        fclose(f);
    }
    

go_home:
	if(err)
		fprintf(stderr,"### error is %d\n",err);

	if(mo)
		DisposeMovie(mo);

	nr_printf(1,"");
	if(err)
		exit(1);

	exit(0);
	}

// end of file
