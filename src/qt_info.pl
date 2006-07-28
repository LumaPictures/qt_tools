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

qt_info 

=head1 SYNOPSIS

B<qt_info> [movie]

=head1 DESCRIPTION

B<qt_info> is a command line tool that sets or
displays various information about a QuickTime movie.
Most of this information is available -- in one form or another --
via QuickTime Player. B<qt_info> prints the information in a somewhat
script-friendly format, if that's your trip. It also shows what
I considered to be the relevant features all at once, with no tabbing
between the different QuickTime Player panels.

Furthermore, you can use options to set values for the
movie annotations.

=head1 OPTIONS

=over 4

   --album="<Album>"
   --artist="<Artist>"
   --author="<Author>"
   --chapter="<Chapter>"
   --comment="<Comment>"
   --composer="<Composer>"
   --copyright="<Copyright>"
   --creationdate="<Creation Date>"
   --description="<Description>"
   --director="<Director>"
   --disclaimer="<Disclaimer>"
   --encodedby="<Encoded By>"
   --fullname="<Full Name>"
   --genre="<Genre>"
   --hostcomputer="<Host Computer>"
   --information="<Information>"
   --keywords="<Keywords>"
   --make="<Make>"
   --model="<Model>"
   --originalartist="<Original Artist>"
   --originalformat="<Original Format>"
   --originalsource="<Original Source>"
   --performers="<Performers>"
   --producer="<Producer>"
   --product="<Product>"
   --software="<Software>"
   --specialplaybackrequirements="<Special Playback Requirements>"
   --track="<Track>"
   --warning="<Warning>"
   --writer="<Writer>"
   --urllink="<URL Link>"

=back

=head1 EXAMPLES

=over 4

=item qt_info test.mov

...generated the following display on my computer:

	+                 movie name : /downloads/short_films/krab/krab2_001k_per_sec.mov 
	+             movie duration : 11.283 (6770/600) 
	+                 movie rate : 0.000000 
	+       movie preferred rate : 1.000000 
	+               movie volume : 256 
	+     movie preferred volume : 256 
	+                  movie box : (0,0,240,320) 
	+       movie natural bounds : (0,0,240,320) 
	+     movie display clip rgn : (null) 
	+             movie clip rgn : (null) 
	+   movie display bounds rgn : rectangular (0,0,240,320) 
	+          movie track count : 1 
	+                      Album : From The Crab 
	+                     Author : the pipe and the crab 
	+
	+                track index : 1 
	+                   track id : 1 
	+      track create/mod time : 3131786230 / 3131786235 
	+              track enabled : yes 
	+   track usage/layer/alt id : 14 / 0 / 1 
	+               track volume : 0 
	+               track offset : 0.000 (0/600) 
	+             track duration : 11.283 (6770/600) 
	+           track dimensions : (320,240) 
	+             track clip rgn : (null) 
	+   track display bounds rgn : rectangular (0,0,240,320) 
	+     track movie bounds rgn : rectangular (0,0,240,320) 
	+           track bounds rgn : rectangular (0,0,240,320) 
	+             media duration : 11.285 (1015641/90000) 
	+             media language : 0 
	+              media quality : 0 
	+              media handler : vide/appl Apple Video Media Handler 
	+         media data handler : alis/appl Apple Alias Data Handler 
	+    media description count : 1 
	+         media sample count : 132 
	+    media sync sample count : 6 
	+ media preferred chunk size : 32768 
	+     --- sample description : 1 of 1 
	+               video format : SVQ3 Sorenson Video 3 Compressor 
	+   temporal/spatial quality : 512 / 512 
	+         width/height/depth : 320 / 240 / 32 
	+

=back

=head1 SEE ALSO

qt_export(1), qt_thing(1), qt_tools(1), qt_atom(1)

=head1 AUTHOR

David Van Brink, email poly@omino.com , subject line must begin with "quicktime".

This software is provided as-is, &c.

This software is B<FREEWARE>. There is no compensation expected. In
return, I'll try to fix bugs if you find them. Hope that's ok with you.

=cut

# end of file
