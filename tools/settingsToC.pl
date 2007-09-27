#!/usr/bin/perl
use strict;

my $filename = shift;
my $varname = shift;

my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
       $atime,$mtime,$ctime,$blksize,$blocks)
                  = stat($filename);

print "size = $size\n";

open F, $filename;
my @bytes;
for(my $i = 0; $i < $size; $i++)
{
    my $byte;
    read F,$byte,1;
    #print $byte;
    $bytes[$i] = $byte;
}

print "unsigned char $varname\[$size] = { // from $filename\n";
my $step = 10;
for(my $base = 0; $base < $size; $base += $step)
{
    printf("    /* %5d */ ",$base);
    my $last = $base + $step;
    if($last > $size) { $last = $size; }
    for(my $addr = $base; $addr < $last; $addr++)
    {
        my $byte = $bytes[$addr];
        my $byteVal = unpack("C",$byte);
        printf "0x%02x",$byteVal;
        print "," if($addr < $size - 1);
    }
    print "  // ";
    for(my $addr = $base; $addr < $last; $addr++)
    {
        my $byte = $bytes[$addr];
        my $byteVal = unpack("C",$byte);
        $byte = "." if($byteVal < 0x20 || $byteVal > 0x7e);
        print $byte;
    }
    print "\n";
}
print "    };\n";
