#!/usr/bin/perl
print "oh don't run this\n"; exit(1);

#
# This is a perl file only so we can use
# pod2man on it. What a hoot!
#
# It uses the source file name, though, so we
# give it this name.
#

=head1 NAME

qt_export 

=head1 SYNOPSIS

B<qt_export> [options] [source-movie new-movie]

B<--video>=I<compressor>,I<frame-rate>,I<quality>,I<bits-per-pixel>

B<--audio>=I<compressor>,I<sample-rate>,I<bits-per-sample>,I<channels>

B<--loadsettings>=I<export-settings>

B<--savesettings>=I<export-settings>

B<--exporter>=I<exporter-subtype>[,I<exporter-mfr>]

B<--sequencerate>=I<sequence-import-fps>

B<--dodialog> B<--datarate>=I<kilobytes-per-second> B<--keyframerate>=I<frames-per-key-frame>
B<--duration>=I<start-time>,I<end-time> B<--replacefile>

=head1 DESCRIPTION

B<qt_export> lets you convert (or "export") QuickTime movies to
new QuickTime movies. This is something that QuickTime Player can do if
you have licensed QuickTime Pro.
Why would you want to use this command line tool instead? Two reasons. First,
you can use shell scripting to control the parameters or file
choices for batch processing. (I am scared of AppleScript, you see.)
Second, because B<qt_export> is I<free>, and QuickTime Pro is not.

(Not to even mention that you can use "nice" to run a big encode job
without affecting your foreground work. Or run multiple encodes at
once -- exactly the way QuickTime Player doesn't -- to use both your
CPU's for those single-CPU encoders. Or even telnet to your
other Mac to start an encoding job going over there, too.)

Command line options let you change many (but not all) of the parameters 
QuickTime uses to export a movie. Additionally, you can
invoke QuickTime's built-in export dialog, and save those settings
for reuse.

=head1 OPTIONS

=over 4

=item B<--video>=0 to disable video export.

=item B<--video>=I<compressor>,I<frame-rate>,I<quality>,I<bits-per-pixel>

Comma-separated values are all optional; any omitted value will receive
a reasonable default.

I<compressor> is a four-character ID for the image
compression component to be used. (See Appendix A.)

Additionally, several types invoke exporters for specific nonmovie file types.
mpg invokes Roxio's MPEG-1 exporter (if present). mpg2 invokes Apple's
MPEG-2 exporter (if present).

I<frame-rate> is given in frames per second,
and may be a fractional number, such as 29.97 (for broadcast video) or 0.1 (for
a 10-second per frame slide show).

I<quality> ranges from 0 to 100.

I<bits-per-pixel>
is not used by most compressors.

=item B<--audio>=0 to disable audio export.

=item B<--audio>=I<compressor>,I<sample-rate>,I<bits-per-sample>,I<channels>

I<compressor> is a four-character ID for the audio
compression component to be used. (See Appendix B.)

Additionally, several types invoke exporters for specific nonmovie file types.
AIFF will invoke QuickTime's AIFF exporter. WAV will invoke QuickTime's WAV exporter.


I<sample-rate> is the number of samples per second, and may be a fractional value.

I<bits-per-sample> is ignored by most compressors; typically 8 or 16.

I<channels> is 1 for mono or 2 for stereo.

=item B<--datarate>=I<kilobytes-per-second>

QuickTime will try to limit the data rate of the
exported video track to this value.

=item B<--keyframerate>=I<frames-per-key-frame>

The video compressor will insert periodic sync frames (sometimes
called key frames) at this rate. (Not supported by all
compressors.)

=item B<--duration>=I<start-time>,I<end-time>

You may choose to export only a portion of the movie. These
times are specified in seconds. These may be decimal fractions.

=item B<--loadsettings>=I<export-settings>

If you have previously saved some export settings then you can
reuse them with this switch. These settings are appled I<before>
any command line switches which affect the export settings.

=item B<--savesettings>=I<export-settings>

You may save your export settings for reuse.

=item B<--exporter>=I<exporter-subtype>[,I<exporter-mfr>]

You can select a particular QuickTime exporter by type. Use
"qt_info --type=spit" to see a complete list of such exporters.
My favorite is --exporter=mpg4.

=item B<--sequencerate>=I<sequence-import-fps>

If the source name contains digits and a sequence rate is specified,
then B<qt_export> will import an image sequence. It will look for
any files which are part of the same sequence, and append them all
into a movie. It will then export the movie with the video settings
specified.

=item B<--dodialog>

This switch brings up the standard QuickTime movie export
dialog. The dialog is applied I<after> any loaded settings or
command line switches.

=item B<--help>

Show abbreviated help.

=item B<--man>

Show this man page, compiled right into the tool, to keep it tidy and self contained.

=back

=head1 COMPRESSING FOR DVD STUDIO PRO

This version of B<qt_export> only works with DVD Studio Pro 2.0 and later.
(Earlier versions of the MPEG2 exporter shipped with DVD Studio Pro did
not support dialog settings.)

To convert a single file, try:

       qt_export --video=mpg2 --dodialog sourcemovie.mov dvdmovie.mp2

To convert multiple files, first use --dodialog without a source or destination file
to save your desired settings, then use the saved settings as many times
as needed.

       qt_export --video=mpg2 --dodialog --savesettings=foo.st
       qt_export --video=mpg2 --loadsettings=foo.st sourcemovie_1.mov dvdmovie_1.mp2
       qt_export --video=mpg2 --loadsettings=foo.st sourcemovie_2.mov dvdmovie_2.mp2
       ... (more files)

=head1 EXAMPLES

=over 4

=item qt_export sourcemovie.mov exportmovie.mov

Use all default settings to recompress sourcemovie.mov to exportmovie.mov.
The default video compressor is Sorenson 3 (SVQ3) at 50% quality,
maintaining the source movie's frame rate, and one key frame for
every sixty frames. The default audio compressor is raw ('twos')
set to 44100 samples per second, stereo.

=item qt_export sourcemovie.mov video.dv

Convert a QuickTime movie to a digital video stream.

=item qt_export sourcemovie.dv video.mp4 --dodialog

Convert a digital video stream to an MPEG 4 file, letting you 
choose the exact settings in an export settings dialog.

=item qt_export sourcemovie.mov soundfile.aif

=item qt_export infile.wav outfile.mp3

=item qt_export infile.mp3 outfile.au

Audio formats can be freely interconverted. If the source is a movie, the audio will be extracted.

=item qt_export --sequencerate=15 sourcepicture_123.jpg exportmovie.mov

Look for files named sourcepicture_000.jpg, sourcepicture_001, ..., sourcepicture_999.jpg,
and append any which are found into a new movie named exportmovie.mov. Use the
default video compression settings.


=item qt_export --dodialog --savesettings=exportsettings.dat

This will invoke the QuickTime export dialog and save the settings to a file.
No movie will be exported.

=item qt_export --video=cvid download.mpg some_movie.mov

Convert an MPEG file to a Cinepak movie, keeping the same frame rate.
(As it turns out, QuickTime can not export the audio from an MPEG movie.)

=item qt_export --loadsettings=exportsettings.dat --video=,10 test.mov test_10.mov

Start with the settings in exportsettings.dat and override just the frame rate to 10
frames per second. Use these new settings to convert test.mov to test_10.mov.

=item qt_export --duration=2,3.5 test.mov test_out.mov

Recompress one and a half seconds starting two seconds in of test.mov into test_out.mov.
Use all default reasonable compressions settings.

=item qt_export mysong.mp3 --audio=aiff mysong.aiff

Convert an MP3 file to an AIFF file.

=item qt_export --exporter=BMPf,.... a_movie.mov a_bmp_file.bmp --duration=20.4,987

Convert the frame of a_movie.mov at time 20.4 seconds into a Windows-style bitmap
file. (The second part of the duration is ignored.)

=back

=head1 APPENDIX A: Video Compressors

On my Mac, I see the following video compressors. (qt_thing --type='imco'.)

	8BPS "Apple Planar RGB"
	SVQ1 "Sorenson Video Compressor"
	SVQ3 "Sorenson Video 3 Compressor"
	WRLE "Apple BMP"
	cvid "Apple Cinepak"
	dvc  "Apple DV/DVCPRO - NTSC"
	dvcp "Apple DV - PAL"
	dvpp "Apple DVCPRO - PAL"
	h261 "Apple H.261"
	h263 "Apple H.263"
	jpeg "Apple Photo - JPEG"
	mjp2 "JPEG 2000 Encoder"
	mjpa "Apple Motion JPEG A"
	mjpb "Apple Motion JPEG B"
	mp4v "Apple MPEG4 Compressor"
	png  "Apple PNG"
	raw  "Apple None"
	rle  "Apple Animation"
	rpza "Apple Video"
	smc  "Apple Graphics"
	tga  "Apple TGA"
	tiff "Apple TIFF"
	yuv2 "Apple Component Video - YUV422"

=head1 APPENDIX B: Audio Compressors

On my Mac, I see the following audio compressors. (qt_thing --type='scom'.)

	MAC3 "MACE 3:1"
	MAC6 "MACE 6:1"
	QDM2 "QDesign Music 2"
	Qclp "Qualcomm PureVoice"
	alaw "ALaw 2:1"
	fl32 "32-bit Floating Point"
	fl64 "64-bit Floating Point"
	ima4 "IMA 4:1"
	in24 "24-bit Integer"
	in32 "32-bit Integer"
	mp4a "MPEG-4 Audio"
	sowt "16-bit Little Endian"
	twos "16-bit Big Endian"
	ulaw "mu-Law 2:1"

=head1 APPENDIX C: QuickTime Exporters

On my Mac, I see the following exporters. (qt_thing --type='spit'.)

    3gpp:appl     "3G"
    AIFF:musi     "AIFF"
    AIFF:soun     "AIFF"
    BMPf:....     "BMP"
    FLC :appl     "FLC"
    MPEG:....     "MPEG2"
    MPG :Roxi     "Toast Video CD"
    Midi:musi     "Standard MIDI"
    MooV:appl     "QuickTime Movie"
    MooV:hint     "Hinted Movie"
    MooV:vrob     "Interframe Compressed VR Object Movie"
    MooV:vrsp     "Separate Single-Node Movies"
    MooV:vrwe     "Fast Start QuickTime VR Movie"
    PICT:....     "Picture"
    TEXT:text     "Text"
    TEXT:tx3g     "QuickTime TeXML"
    ULAW:soun     "?Law"
    VfW :appl     "AVI"
    WAVE:soun     "Wave"
    dvc!:appl     "DV Stream"
    embd:....     "QuickTime Media Link"
    grex:appl     "Image Sequence"
    mpg4:appl     "MPEG-4"
    sfil:soun     "System 7 Sound"
    snd :soun     "Sound"

=head1 SEE ALSO

qt_tools(1),
qt_atom(1), qt_info(1), qt_thing(1)

=head1 BUGS

For some codecs or exporter types, you MUST use the --dodialog option. Combined
with the --savesettings and --loadsettings options you can then set up a batch
process. But you have to use the --dodialog once to get the export configured
just right. Tragically, not all exporters properly export their configurability
in an API-accessible fashion.

=head1 AUTHOR and LICENSE

David Van Brink, email poly@omino.com , subject line must begin with "quicktime".

This software is provided as-is, &c.

This software is B<FREEWARE>. There is no compensation expected. In
return, I'll try to fix bugs if you find them. Hope that's ok with you.

=cut

# end of file
