#!/usr/bin/perl


sub usage()
{
    print <<EOP;

     usage:  munge_and_move.pl srcfile dstfile a=this b=that

    replace --a-- with this and --b-- with that
    srcfile to dstfile


EOP
;
exit(0);
}

use strict;
use misc;

sub main(@)
{
    my $srcFile = shift;
    my $dstFile = shift;

    usage unless $dstFile;
    
    my %matchers;
    $matchers{date} = mp_date_time;
    while(my $r = shift)
    {
        if($r =~ /^(.*)=(.*)$/)
        {
            $matchers{$1} = $2;
        }
    }

    my $contents = mp_read_file($srcFile);
    foreach my $key (keys(%matchers))
    {
        my $value = $matchers{$key};
        $contents =~ s/--$key--/$value/sg;
    }

    mp_write_file($dstFile,$contents);
}

main(@ARGV);

# --- end ---
