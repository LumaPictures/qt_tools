#!/usr/bin/perl

use strict;
sub main
	{
	my $line;
	my $vmajor = "u";
	my $vminor = "u";
	my $vbld = "u";
	my $vdate = "u";

	my $spew;
	my $want_date = shift;

	while($line = <stdin>)
		{
		if($line =~ /^(#define) ([^ ]+) (.+)$/)
			{
			my $def = $1;
			my $name = $2;
			my $value = $3;

			if($value =~ /^\"(.*)\"$/)
				{
				$value = $1;	# no quotes
				}
			$vmajor = $value if($name eq "k_version_major");
			$vminor = $value if($name eq "k_version_minor");
			$vbld = $value if($name eq "k_version_build_number");
			$vdate = $value if($name eq "k_version_date");
			}
		}

	
	if($want_date == 1)
		{
		$spew = $vdate;
	    $spew =~ tr/\., /___/;
		}
    elsif($want_date == 2)
        {
		$spew = "${vmajor}.${vminor}";
        }
    elsif($want_date == 3)
        {
		$spew = "${vbld}";
        }
	else
		{
		$spew = "${vmajor}_${vminor}_$vbld";
		}

	print "$spew";
	}

main(@ARGV);

# end of file
