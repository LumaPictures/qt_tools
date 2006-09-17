#!/usr/bin/perl

use strict;

sub usage
	{
	print <<EOP;

	usage: bump_version.pl <filename> <what_to_bump>

	reads in the file, and if it sees

		#define what_to_bump number

	changes it to

		#define <what_to_bump> <number + 1>

	also, always puts in a string for

		#define k_version_date 2003.03.07.20:34:02

EOP
	exit(0);
	}

sub date_time
	{
	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time());

	return sprintf("%04d.%02d.%02d-%02d:%02d:%02d",
		$year + 1900,
		$mon + 1,
		$mday,
		$hour,
		$min,
		$sec);
	}


sub main
	{
	my $version_file = shift;
	my $what_to_increment = shift;

	my $have_rcs = (-d "RCS");
	my $have_svn = (-d ".svn");

	usage() if(!$version_file or !$what_to_increment);

	print ".\n";
	print ".\n";

	system("co -l -q -f $version_file") if $have_rcs;

	open F,$version_file or die "could not read $version_file";
	
	my $line;
	my $file;
	while($line = <F>)
		{
		if($line =~ /^(.*?)[\n\r]*$/)
			{
			$line = $1;
			}

		if($line =~ /^(#define) ([^ ]+) (.+)$/)
			{
			my $def = $1;
			my $name = $2;
			my $value = $3;

			if($value =~ /^\"(.*)\"$/)
				{
				$value = $1;	# no quotes
				}

			if($name eq $what_to_increment)
				{
				if($have_rcs || $have_svn)
					{
					print ". bumping $name from $value to ",$value + 1,".\n";
					$value = $value + 1;
					}
				else
					{
					$value = "USER BUILD";
					print ". setting $name to $value.\n";
					}
				}

			if($name eq "k_version_date")
				{
				my $d = date_time();
				print ". setting $name from $value to $d\n";
				$value = $d;
				}

			$value = "\"" . $value . "\"";  # quote everything

			$line= "$def $name $value";
			}
		$file .= $line . "\n";
		}

	close F;
	open F,">$version_file" or die "could not write $version_file";
	print F $file;
	close F;

	system("echo bumping | ci -u -q -f $version_file") if $have_rcs;

	print ".\n";
	print ".\n";
	}

main(@ARGV);

#end of file
