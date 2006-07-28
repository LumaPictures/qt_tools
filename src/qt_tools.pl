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

qt_tools

=head1 DESCRIPTION

B<qt_tools> is a suite of command line tools for Mac OS X 10.4 and later
for working with QuickTime movies. Why command line tools?
Can't you do everything you need via QuickTime Player?

Well, if you have to ask, these tools are perhaps not
for you.

=over 4

=item B<qt_export> converts QuickTime movies between formats

=item B<qt_proofsheet> renders frames of a QuickTime move to a big picture

=item B<qt_info> displays information about a QuickTime movie

=item B<qt_thing> lists installed components

=item B<qt_atom> shows the contents of a qt_export settings file
   

=back

=head1 INSTALLATION

B<qt_tools> is provided as a simple folder of files. To install
it, you can run the script called "install.sh" from the Terminal.
(These are command line tools, after all!)

Type:

    sudo ./install.sh

from within the qt_tools disk image directory. You'll be prompted for root password.
The executables will be placed in /usr/local/bin and the manpages will be placed
in /usr/local/share/man.

Source code is also provided in the disk downlaod.

You can also run the tools directly off the qt_tools
download if you prefer.

=head1 OPTIONS

The following switches are supported by all tools in the B<qt_tools> suite.

=over 4

=item B<--help>

Display a brief summary of the purpose and use of the tool.

=item B<--version>

Display the version of the tool.

=item B<--man>

Display the manual page for the tool. You will probably want to 
use "| more" to see it all.

=back

=head1 SOURCE CODE

Source code is provided with these tools. To build:

  sh-2.05a$ cd <qt_tools_folder>/src/
  sh-2.05a$ make

To run the automated tests (not many, but better than nothing):

  sh-2.05a$ cd <qt_tools_folder>/src/testsuite
  sh-2.05a$ ./runtests.pl

=head1 SEE ALSO

qt_export(1), qt_info(1), qt_thing(1)

=head1 AUTHOR

David Van Brink, email poly@omino.com , subject line must begin with "quicktime".

This software is provided as-is, &c.

This software is B<FREEWARE>. There is no compensation expected. In
return, I'll try to fix bugs if you find them. Hope that's ok with you.

=head1 THANK YOU

The art of software engineering is carried on in large part via oral 
tradition. Sam Bushell and Sean Fagan helped me I<a lot> when I needed the
occasional bit of information. Also, the hive mind of forum and yam, as
ever, were indispensable. www.geek.org. And of course, pesky users have found
bugs... can't live with 'em, can't live without 'em. Bugs, I mean.

=cut

# end of file
