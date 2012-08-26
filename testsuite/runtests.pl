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
	my $fileName = shift;

	my $qtInfoCmd = "$appsLoc/qt_info \"$fileName\"";
	my $result = snagParseableOutput($qtInfoCmd);
	my $assnCount = scalar(keys(%$result));
    print "qt_info on $fileName got $assnCount values, dur=$$result{movie_duration}\n";
	return $result;
}

sub snagOutput($)
{
	my $cmdLine = shift;
    print "snagOutput backticking: $cmdLine\n";
	my $out = `$cmdLine`;
    return $out;
}

sub snagParseableOutput($)
{
	my $cmdLine = shift;
    print "snagParseableOutput backticking: $cmdLine\n";
	my $parseableInfo = `$cmdLine`;
	my @infoLines = split(/[\n\r]+/, $parseableInfo);
    my $assnCount = 0;
	my $trackIndex = 0;
	my %result;
	foreach my $line (@infoLines)
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
    die "($assertCount) $msg" if !$bool;
    $assertCount++;
}

sub assertZero($)
{
	my ($x) = (@_);
	die "ERROR nonzero $x" if($x != 0);
    $assertCount++;
}

sub fail($)
{
    my $what = shift;
    die "ERROR $what";
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
    print "testQtInfo\n";
    my $qtInfo = getQTInfo($sweepMov);
    assertEquals("movie_duration",2,$$qtInfo{movie_duration});
    assertStarts("sound_format","twos",$$qtInfo{1}{sound_format});
    assertStarts("video_format","rle",$$qtInfo{2}{video_format});
    assertEquals("movie_track_count",2,$$qtInfo{movie_track_count});

    assertEquals("media_average_sample_rate",60,$$qtInfo{2}{media_average_sample_rate});
}

sub testWithSpace
{
    # just make sure some file with spaces can be found by a tool.
    print "test With Space\n";
	my $srcImgName = "clock2 with space.jpg";
	assertFileExists($srcImgName);
    my $qtInfo = getQTInfo($srcImgName);
	assertEq("movie_box","(0,0,200,188)",$$qtInfo{movie_box});
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
    assertTrue("data rate $s1 looks wrong",$s1 < 2000000);
    assertTrue("data rate $s1 looks wrong",$s2 > 3000000);

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

sub testOddSequenceRates()
{
	my $srcImgName = "clock1.jpg";
	
	foreach my $rate (500,501,550,597,1.1,29.97)
	{
	    my $movOutName = "testoutput/oddrate$rate.mov";
		my $result = sys("qt_export $srcImgName $movOutName --sequencerate=$rate");
		my $qtInfo = getQTInfo($movOutName);
        my $mediaDurationInfo = $$qtInfo{1}{media_duration};
        if($mediaDurationInfo =~ /^(.*) \((.*)\/(.*)\)$/)
        {
           my $dur = $1;
           my $num = $2;
           my $den = $3;

           my $reportedMediaDuration = "none";
           if($num != 0)
           {
             $reportedMediaDuration = $den / $num;
           }
           assertEquals("media frame fraction [$mediaDurationInfo]",$rate,$reportedMediaDuration);
        }
        else
        {
            assertEquals("no media duration",1,2);
        }
	}
}

sub testDirectoryWithNumbers()
{
    mkdir("testoutput/frames4u/");
    assertTrue("dir not made",-d "testoutput/frames4u/");

    my $cmd = "${appsLoc}qt_export $sweepMov testoutput/frames4u/sweep90frames.jpg --duration=0,1.5";
    print "cmd = $cmd\n";
	my $assns = snagParseableOutput("$cmd");

    my $cmd = "${appsLoc}qt_export testoutput/frames4u/sweep90frames02.jpg testoutput/sweepframes.mov --sequencerate=60";
    print "cmd = $cmd\n";
	my $assns = snagParseableOutput("$cmd");
    my $qtInfo = getQTInfo("testoutput/sweepframes.mov");
    # make sure we got all the frame -- even though there was a number in the directory
    my $duration = $$qtInfo{movie_duration};
    assertEquals("sequence with tricky numbers in file name (duration)",1.5,$duration);
}

sub testExporterSelecting()
{
# make sure we auto-select the exporter & stuff just from the file suffix
	foreach my $try (
			"aif,AIFF,soun",
			"aiff,AIFF,soun",
			"dv,dvc!,appl",
			"jpg,grex,appl,1",
			"png,grex,appl,1",
			"tga,grex,appl,1",
            # needs dvdsp installed"mp2,MPEG,....",
# mp3 is often not around, either... boo hoo. "mp3,mp3,PYEh",
			"wav,WAVE,soun",
			"mp4,mpg4,appl",
			"m4a,mpg4,appl",   # an audio-only mpeg-4 file
			"m4v,M4V,appl",   # ipod file...
            "avi,VfW,appl"
            )
	{
		my ($extension,$subtype,$mfr,$isStillFormat) = split(/,/,$try);
		my $resultFile = "testoutput/fooxport_$extension.$extension";
        my $duration = "--duration=0,.1";
        if($isStillFormat)
        {
            $duration = "";  # a duration on jpg would cause an image sequence. we want 1 file.
        }
        my $cmd = "${appsLoc}qt_export $sweepMov $duration $resultFile";
        print "cmd = $cmd\n";
		my $assns = snagParseableOutput("$cmd");
		assertEq("choosing exporter subtype for $extension",$subtype,$$assns{exporter_subtype});
		assertEq("choosing exporter mfr for $extension",$mfr,$$assns{exporter_mfr});
		assertFileExists($resultFile);
        $assns = getQTInfo($resultFile);
        assertEq("qtinfo reading movie",$resultFile,$$assns{movie_name});
	}
}

sub testM4aAudioOnly()
{
    my $resultFile = "testoutput/sweepmoveAudio.m4a";
    my $cmd = "${appsLoc}qt_export $sweepMov $resultFile";
    print "cmd = $cmd\n";
    my $assns = snagParseableOutput("$cmd");
    my $qtInfo = getQTInfo($resultFile);
	assertEq("audio has empty movie_box pls?","(0,0,0,0)",$$qtInfo{movie_box});
}

sub testManPages()
{
    print "checking man pages...";
    my $cmd = "${appsLoc}qt_export --man";
    print "cmd = $cmd\n";
    my $out = snagOutput($cmd);

    if($out =~ /.*No man .*/mi)
    {
        fail("No man page!");
    }
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
    system("date");
    sys("qt_export --version",1);
    print "\n\n\n";

    print "apps in: $appsLoc\n";

	#print $file;
	#my $qtInfo = getQTInfo($file);
	#print $$qtInfo{movie_box};

    system("mkdir -p testoutput");
	system("rm -rf testoutput/* testoutput/.Q*");

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
        testWithSpace();
		testQtInfo();
        testManPages();
        testM4aAudioOnly();
        testDirectoryWithNumbers();
		testExporterSelecting();
        testOddSequenceRates();
		test6();
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
	    system("rm -rf testoutput/*");
        print "(deleted testoutput/*)\n";
        }
}

main(@ARGV);

#end of file

