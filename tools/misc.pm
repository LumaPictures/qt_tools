

# | routines for reading files and stuff


# ex: set tabstop=4:
# ex: set shiftwidth=4:
# ex: set expandtab:

use strict;
use FileHandle;
use POSIX;
use Fcntl ':flock';  # LOCK_* constants


$| = 1;



# -------------------------------------
# mp_new_opts(@ARGV)
#
# Given a list of arguments, return
# a hash where the keys and values
# are taken from those arguments of
# the form "--key=value". The hyphens
# disappear from the key name.
#
# A command line switch of "--key"
# is equivalent to "--key=_".
#
# a special key named _argc contains
# a count of non-dash-dash arguments,
# and they are in the hash as {0}, {1},
# and so on.
#
# |     in: array of arguments (such as @ARGV)
# |
# | return: reference to be accesed by mp_get_opt
# |
# | (no dispose routine needed)
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


# -------------------------------
# mp_get_opt(opts_ref, switch_name, default_value [, must_be_number])
#
# Look at a hash as returned by parseArgs, and
# give the value of the switch, or the default_value
# if it was not specified in the command line.
#
# |     in: opts_ref       -- reference from mp_new_opts
# |         switch_name    -- name of --option to check
# |         default_value  -- value to return if it wasnt in the args
# |         must_be_number -- optional boolean to force numeric interpretation
# |

sub mp_get_opt
  {
  my $opts_ref = shift;
  my $switch_name = shift;
  my $default_value = shift;
  my $must_be_number = shift;

  my $switch_value;

  $switch_value = $$opts_ref{$switch_name};
  $switch_value = $default_value if ($switch_value eq "");
  $switch_value *= 1 if ($must_be_number);

  return $switch_value;
  }



# +-------------------------
# | mp_read_file(filename[,error_ref])
# |
# | returns the complete file contents
# |
# |
# |     in: filename -- name (path) of file to read
# |
# | return: one long string with the whole file, or
# |         empty string if no such file or empty file.
# |
# |         (use Perl's -e if you really really care whether
# |         the file was there...)
# |

sub mp_read_file
  {
  my ($filename,$error_out) = (@_);
  my $bunch;
  my $file_contents;
  my $did;

  # (return error if ref provided)

  if($error_out)
    {
    $$error_out = (-f $filename) ? 0 : -1;
    }

  # (read the whole file, if present. else "")

  if(open(FILE,$filename))
    {
    binmode FILE;    # Bite me, Windows! --dvb
    while(read(FILE,$bunch,32000))
      {
      $file_contents .= $bunch;
      }
    close FILE;
    }

  return $file_contents;
  }

# -----------------------
# mp_write_file(filename,contents)
#
# creates new file and writes entire
# file contents. Return "ok" if so,
# or "" if not.
#
# |
# |     in: filename -- name of file to create (or replace)
# |         contents -- string of all the bytes to put into file
# |
# | return: 0 for success, <0 for error
# |

sub mp_write_file($$)
  {
  my ($filename,$contents) = (@_);

  my $did;

  #
  # If filename is "", print it to stdout
  # and that is all.
  #

  if($filename eq "")
    {
    print $contents;
    return "";
    }

  #
  # Delete existing file, if any.
  #

  unlink ($filename) if(-e $filename);

  $did = open(FILE,">$filename");
  if($did)
    {
    binmode FILE;    # Bite me, Windows! --dvb
    $did = print FILE $contents;
    close FILE;
    return $did ? 0 : -1;
    }

  return -1;
  }



sub mp_date_time
    {
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdet) = localtime(time);
    $mon++;
    $year += 1900;
    my $d = sprintf("%04d.%02d.%02d",$year,$mon,$mday);
    my $t = sprintf("%02d:%02d:%02d",$hour,$min,$sec);

    return "$d $t";
    }


return 1;


# end of file
