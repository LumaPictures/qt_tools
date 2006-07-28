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

qt_thing 

=head1 SYNOPSIS

B<qt_thing> [B<-type>=I<type>]

=head1 DESCRIPTION

B<qt_thing> is a command line tool that
displays various information about the components
installed in your system.

=head1 OPTIONS

=over 4

=item B<--type>=I<type>

You may specify the component type to show; if no type is specified, all
components will be listed. Usually there are several hundred.

=item B<--help>

Show abbreviated help.

=item B<--man>

Show this man page, compiled right into the tool, to keep it tidy and self contained.

=back

=head1 EXAMPLES

=over 4

=item qt_thing --type=clok

...generated the following display on my computer:

   +
   + There are 5 components of type 'clok' (0x636c6f6b)
   +
   +    1 @0x000101b3    clok:fwcy:appl     "Apple FireWire Cycle Clock"
   +    2 @0x00010093    clok:icm.:appl     "Apple ICM Clock"
   +    3 @0x000100f1    clok:micr:appl     "Apple Microsecond Clock"
   +    4 @0x00010056    clok:soun:appl     "Sound Clock"
   +    5 @0x000100f2    clok:tick:appl     "Apple Tick Clock"
   +

=back

=head1 SEE ALSO

qt_tools(1),
qt_atom(1), qt_export(1), qt_info(1)

=head1 AUTHOR and LICENSE

David Van Brink, email poly@omino.com , subject line must begin with "quicktime".

This software is provided as-is, &c.

This software is B<FREEWARE>. There is no compensation expected. In
return, I'll try to fix bugs if you find them. Hope that's ok with you.
=cut

# end of file

