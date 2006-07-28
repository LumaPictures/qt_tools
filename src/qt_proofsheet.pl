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

qt_proofsheet

=head1 SYNOPSIS

B<qt_proofsheet> [options] source-movie proof-sheet-name

B<--framesize>=I<width>[,I<height>]

B<--framesperrow>=I<frames-per-row>

B<--duration>=I<start-time>,I<end-time> B<--replacefile>

B<--framerate>=I<frames-per-second>

B<--codec>=I<compressor-to-use>

B<--spacing>=I<space-between-frames>

Controls the layout of the proof sheet by adjusting the margins along
the edges and the spacing between adjacent frames.

B<--timecode>=I<timecode-font-size>[,I<timecode-position>]

B<--title>=I<title-font-size>

=head1 DESCRIPTION

B<qt_proofsheet> lets you convert a QuickTime movie to a big
picture. This can be a useful reference when editing large
video files, to visually locate times of interest.

Command line options let you select which portion of the
movie is drawn and how the timecode is displayed.


=head1 OPTIONS

By if no options are given then the whole movie will be presented
such that it fits roughly on a single 8.5" x 11" document.

=over 4

=item B<--framesize>=I<width>

=item B<--framesize>=I<width>,I<height>

Controls the size in pixels of each frame from the movie. If just one number is given then
the width is set to it, and the height will be proportionate. If two numbers
are given then the width and height are both controlled.

If a number is very small (less than two) it is presumed to be a multiplier. So,
for example, 0.5 would render each frame onto the proof sheet at 1/2 pixel size.

=item B<--framesperrow>=I<frames-per-row>

Determines the number of frames to place in each row of the proof sheet.

=item B<--duration>=I<start-time>,I<end-time> 

=item B<--duration>=I<end-time>

Specifies that only a portion of the source movie be used.

=item B<--replacefile>

Allows the destination file to be overwritten if it already exists.

=item B<--framerate>=I<frames-per-second>

Specifies the interval at which frames are taken from the source movie
for rendering. A value of 30 would use all frames (from a typical video
movie), and a value of 1 would use just one frame per second. A value of
0.0166666 would use just one frame per minute of the source.

=item B<--codec>=I<compressor-to-use>

If the proof sheet name ends with ".jpg" or ".tga" then the proof sheet will
be saved in JPEG or Targa format, respectively. To use a different format supply
a codec option with the four-character ID for the compression component to use.

=item B<--timecode>=I<timecode-font-size>[,I<timecode-position>]

To control size of the timecode imprint which appears on each frame, supply a I<timecode-font-size>.
A size of zero disables the timecode imprint.

The I<timecode-position> may be one of four values: 0 for lower left, 1 for lower right,
2 for upper right, 3 for upper left, and 4 for centered below the frame.

=item B<--title>=I<title-font-size>

Some useful information is printed at the top of the proof sheet; the I<title-font-size> controls
how large it appears. A value of 0 disables the title printing.

=item B<--help>

Show abbreviated help.

=item B<--man>

Show this man page, compiled right into the tool, to keep it tidy and self contained.

=back

=head1 ANIMATED TEXTURES FOR SECOND LIFE

Second Life is a virtual world comprised entirely of user-created content. It
turns out that qt_proofsheet is helpful for a certain obscure aspect of content
creation there. Do check out www.secondlife.com, and say Hi to me, Davan Camus,
if you visit.

"Second Life" supports animated textures in which multiple
frames of animation are tiled onto a single texture map. qt_proofsheet
can produce these tiled textures from a QuickTime movie source.

The main trick is to disable spacing, timecode, and titles. I find it
helpful to render my source movie (from After Effects) at 10 frames per
second. This makes the arithmetic easy. Here's a typical conversion:

qt_proofsheet myMovie.mov slTexture.tga --spacing=0 --title=0 --timecode=0 --framesize=64,64 --framesperrow=8 --duration=6.4

=head1 EXAMPLES

=over 4

=item qt_proofsheet sourcemovie.mov proofsheet.jpg

Use all default settings to show the full duration of sourcemovie.mov
as a single 8.5x11 proof sheet, with about 40 frames chosen evenly.

=back

=head1 SEE ALSO

qt_tools(1),
qt_export(1)

=head1 AUTHOR and LICENSE

David Van Brink, email poly@omino.com , subject line must begin with "quicktime".

This software is provided as-is, &c.

This software is B<FREEWARE>. There is no compensation expected. In
return, I'll try to fix bugs if you find them. Hope that's ok with you.

=cut

# end of file
