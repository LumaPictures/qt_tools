#!/usr/bin/perl

use strict;

sub usage
	{
	print <<EOP;

	usage: text2printf.pl <before_each_line> <after_each_line>

	Produces one line per input line, quoted for C

EOP
	exit(1);
	}


sub main
	{
	my $before_each_line = shift;
	my $after_each_line = shift;

	my $line;

	my $fixups =
		[
			{ in => "\"", out => "\\\"", between => "__dquote__" },
			{ in => "\'", out => "\\\'", between => "__squote__" },
			{ in => "\\\\", out => "\\\\", between => "__bslash__" },
			{ in => "%", out => "%%", between => "__percent__" }
		];	

	while($line = <stdin>)
		{
		# kill end of line
		if($line =~ /^(.*?)[\n\r]*$/)
			{
			$line = $1;
			}

		my $fixup;

		foreach $fixup (@$fixups)
			{
			$line =~ s/$$fixup{in}/$$fixup{between}/g
			}

		foreach $fixup (@$fixups)
			{
			$line =~ s/$$fixup{between}/$$fixup{out}/g
			}

		print "$before_each_line \"$line\\n\" $after_each_line\n";
		}
	}

main(@ARGV);

#end of file
