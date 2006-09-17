#!/usr/bin/perl

# |
# | AUTOMATED TESTS FOR QT_TOOLS
# | 2006 March
# |
# | These tests exercise qt_tools just a little bit.
# | They each run some command line and then look at the resulting files.
# | Often they use qt_info to peek inside a QuickTime movie or other
# | kind of file, and check the results that way.
# |
# | They aren't much, but for an afternoon's coding they
# | sure do improve my confidence about each build!
# |
# | david van brink / poly@omino.com
# |

use strict;


my $appsLoc = "./";

# Trivia: this is the first computer-graphic QuickTime movie ever made...
my $sweepMov = "./sweep.mov";


sub sys($;$)
    {
    my $cmdLine = shift;
    my $errIsOK = shift;

    $cmdLine = $appsLoc . $cmdLine;
    print "\n-------\nEXECUTING: $cmdLine\n-------\n";
    my $result = system($cmdLine);
    assertZero($result) unless $errIsOK;
    return $result;
    }

sub getFileSize($)
    {
    my $filename = shift;
    my @stats = stat($filename);
    my $result = $stats[7];
    return $result;
    }

# +-------------------
# | return reference to hash
# | of each --key=value
# |

sub mp_new_opts
  {
  my $arg;
  my $argVal;
  my $argc;
  my %hash;

  $argc = 0;


  while($arg = shift)
    {
    if($arg =~ /^--/)
      {
      if($arg =~ /^--(.*)\=(.*)$/)
        {
        $arg = $1;
        $argVal = $2;
        }
      elsif($arg=~ /^--(.*)$/)
        {
        $arg = $1;
        $argVal = "_";   # a true-but-zero value
        }

      $hash{$arg} = $argVal;
      }
    else
      {
      $hash{$argc++} = $arg;
      }
    }

  $hash{_argc} = $argc;

  return \%hash;
  }

# +---------------------
# | return hash of values for the qt_info
# | each track in a subhash
# | result{movie_name} = "foo.mov"
# | result{1}{track_dimensions} = "(10,10)"
# |

sub getQTInfo($)
{
	my %result;
	my $trackIndex = 0;
	my $fileName = shift;

	my $qtInfo = `$appsLoc/qt_info $fileName`;
	my @qtInfoLines = split(/[\n\r]+/s,$qtInfo);
    my $assnCount = 0;
	foreach my $line (@qtInfoLines)
	{
		if($line =~ /^\+ *(.*?) : (.*?) *$/)
		{
			my $key = $1;
			my $value = $2;
			$key =~ tr/ /_/;
			#print "$key = $value\n";
			$trackIndex++ if($key eq "track_index");
			if($trackIndex)
			{
				$result{$trackIndex}{$key} = $value;
                $assnCount++;
			}
			else
			{
				$result{$key} = $value;
                $assnCount++;
			}
		}
	}
    print "qt_info on $fileName got $assnCount values, dur=$result{movie_duration}\n";
	return \%result;
}

my $assertCount = 0;
sub assertFileExists($)
{
	my ($f) = (@_);
	open(F,$f) || die "\nERROR: missing file $f\n";
	close F;
	print("good: $f exists\n");
    $assertCount++;
}

sub assertTrue($$)
{
    my ($msg,$bool) = (@_);
    die $msg if !$bool;
    $assertCount++;
}

sub assertZero($)
{
	my ($x) = (@_);
	die "ERROR nonzero $x" if($x != 0);
    $assertCount++;
}

sub assertEquals($$$)
{
	my ($what,$want,$got) = (@_);
	die "ERROR $what: $want != $got" if $want != $got;
    print("good: $what == $got\n");
    $assertCount++;
}


sub assertEq($$$)
{
	my ($what,$want,$got) = (@_);
	die "ERROR $what: --$want-- ne --$got--" if $want ne $got;
    print("good: $what is $got\n");
    $assertCount++;
}

sub assertStarts($$$)
{
	my ($what,$want,$got) = (@_);
	my $len = length($want);
	my $gotStart = substr($got,0,$len);
	die "ERROR $what: $want doesn't begin $got" if $want ne $gotStart;
    print("good: $what -- $got begins with $want\n");
    $assertCount++;
}

sub testHaveTools()
{
    assertFileExists("$appsLoc/qt_export");
    assertFileExists("$appsLoc/qt_info");
    assertFileExists("$appsLoc/qt_thing");
    assertFileExists("$appsLoc/qt_atom");
    assertFileExists("$appsLoc/qt_proofsheet");
}


sub testQtInfo
{
    print "testQTInfo\n";
    my $qtInfo = getQTInfo($sweepMov);
    assertEquals("movie_duration",2,$$qtInfo{movie_duration});
    assertStarts("sound_format","twos",$$qtInfo{1}{sound_format});
    assertStarts("video_format","rle",$$qtInfo{2}{video_format});
    assertEquals("movie_track_count",2,$$qtInfo{movie_track_count});
}

sub test1
{
	my $dm = "testoutput/t1.mov"; # dest movie
	print ("test1");
	my $result = sys("qt_export $sweepMov --duration=1 $dm");
	assertFileExists($dm);
	my $qtInfo = getQTInfo($dm);
	assertEquals("movie_duration",1,$$qtInfo{movie_duration});
	assertStarts("video_format","SVQ3",$$qtInfo{2}{video_format});
	assertEq("movie_box","(0,0,256,256)",$$qtInfo{movie_box});
}

sub test2
{
	my $dm = "testoutput/t2.mov"; # dest movie
	print ("test2");
	my $result = sys("qt_export $sweepMov --duration=1 --video=jpeg $dm");
	assertZero($result);
	assertFileExists($dm);
	my $qtInfo = getQTInfo($dm);
	assertEquals("movie_duration",1,$$qtInfo{movie_duration});
	assertStarts("video_format","jpeg",$$qtInfo{2}{video_format});
}

sub test3
{
	my $dm = "testoutput/t3.jpg"; # dest pix
	print ("test3");
	my $result = sys("qt_proofsheet $sweepMov --duration=1 --video=jpeg $dm --framerate=4 --framesize=40 --framesperrow=4 --spacing=0 --title=0");
	assertZero($result);
	assertFileExists($dm);
	my $qtInfo = getQTInfo($dm);
	assertEq("movie_box","(0,0,160,40)",$$qtInfo{movie_box});
	assertStarts("video_format","jpeg",$$qtInfo{1}{video_format});
}

sub test4
{
	my $dm = "testoutput/t4.jpg"; # dest pix
	print ("test4");
	my $result = sys("qt_proofsheet $sweepMov --duration=1 --video=jpeg $dm --framerate=4 --framesize=40 --framesperrow=4 --spacing=10 --title=0");
	assertZero($result);
	assertFileExists($dm);
	my $qtInfo = getQTInfo($dm);
	assertEq("movie_box","(0,0,210,60)",$$qtInfo{movie_box});
	assertStarts("video_format","jpeg",$$qtInfo{1}{video_format});
}

sub test5
{
    # verify data rate...
    # doesnt have MUCh effect, but 1000 vs 5000 should
    # affect it some...
    my $dm1 = "testoutput/sdr1.mov"; # dest pix
    my $dm2 = "testoutput/sdr2.mov"; # dest pix
    print "test5\n";
    my $result = sys("qt_export --audio=0 $sweepMov $dm1 --datarate=1000");
    my $result = sys("qt_export --audio=0 $sweepMov $dm2 --datarate=5000");
    assertFileExists($dm1);
    assertFileExists($dm2);
    my $s1 = getFileSize($dm1);
    my $s2 = getFileSize($dm2);
    assertTrue("data rate looks wrong",$s1 < 2000000);
    assertTrue("data rate looks wrong",$s2 > 3000000);

    # while we're here, let us verify that audio doesnt exist...
    my $qtInfo = getQTInfo($dm1);
    assertEq("track_count",1,$$qtInfo{movie_track_count});
}

sub test6
{
    print "test6\n";
    # image sequence out and in
    #qt_export sweep.mov --exporter=grex --loadsettings=image_sequence.st testoutput/sweep_seq.jpg

    my $seqOutBaseName = "testoutput/sweep1_seq";
    my $seqOut = "$seqOutBaseName.jpg"; # will become 001 to 120
    my $rebuiltMov = "testoutput/rebuilt.mov";

    my $result = sys("qt_export sweep.mov --exporter=grex --loadsettings=image_sequence.st $seqOut");
    assertFileExists("${seqOutBaseName}001.jpg");
    assertFileExists("${seqOutBaseName}010.jpg");
    assertFileExists("${seqOutBaseName}100.jpg");
    assertFileExists("${seqOutBaseName}120.jpg");

    # and reimport the movie...
    # 10 fps down from 60 leads to a 12 second movie

    my $result = sys("qt_export --sequencerate=10 ${seqOutBaseName}002.jpg $rebuiltMov");
    assertFileExists($rebuiltMov);
    my $qtInfo = getQTInfo($rebuiltMov);
    assertEquals("movie_duration",12,$$qtInfo{movie_duration});
	assertEq("movie_box","(0,0,256,256)",$$qtInfo{movie_box});
}

sub main(@)
{
	my $opts = mp_new_opts(@_);
	my $file = $$opts{0};
    $appsLoc = $$opts{"apps"};
    my $dontdelete = $$opts{"keep"};
    $appsLoc = "../build/app" if(!$appsLoc);
    $appsLoc .= "/";

    print "{options --apps=<qt_tools> --keep (wont delete test results)}\n";
    print "\n\n\n";
    print "QT_TOOLS AUTOMATED TESTS\n";
    system(date);
    sys("qt_export --version",1);
    print "\n\n\n";

    print "apps in: $appsLoc\n";

	#print $file;
	#my $qtInfo = getQTInfo($file);
	#print $$qtInfo{movie_box};

    system("mkdir -p testoutput");
	system("rm -f testoutput/*");

	my $tests = $$opts{"testlist"};
	if($tests)
	{
		my @testList = split(/,/,$tests);
		foreach my $oneTest (@testList)
		{
			my $doString = "$oneTest()";
			print "(About to eval: $doString)\n";
			eval($doString);
		}
	}
	else
	{	
		testHaveTools();
		test6();
		testQtInfo();
		test1();
		test2();
		test3();
		test4();
		test5();
	}

    print "\n\n\nDone. If you see this, the tests passed.\n";
    print "$assertCount assertions correctly assertively asserted.\n\n";
    if(!$dontdelete)
        {
	    system("rm testoutput/*");
        print "(deleted testoutput/*)\n";
        }
}

main(@ARGV);

#end of file

