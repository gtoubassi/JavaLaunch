#!/usr/bin/perl

use strict;

use FindBin;
use File::Basename;

my $installDir = "$FindBin::Bin";
my $scriptName = basename($0);
my $homeDir = `sh -c "cd ~ ; /bin/pwd"`;
chomp($homeDir);
my $appDataDir = "$homeDir/.$scriptName";

my $mainClass;
my $classpath;
my @vmArguments;
my @javaArguments;


sub substituteVariables
{
    my $line = shift;
    
    $line =~ s/%INSTALLDIR%/$installDir/;
    $line =~ s/%APPDATADIR%/$appDataDir/;
    $line =~ s/%APPNAME%/$scriptName/;
    
    return $line;
}

#ClassPath
#MainClass
#JavaArgument
#VMArgument

sub readConfig
{
    my $configFile = "$installDir/Resources/$scriptName.conf";
    
    open(FH, "< $configFile")
    	or die "Cannot open $configFile: $!\n";
    
    for my $line (<FH>) {
    	chomp $line;
	
	$line = substituteVariables($line);
	
	if ($line =~ m/\s*ClassPath\s+(\S+)/) {
	    if (!defined($classpath)) {
                $classpath=$1;
	    }
	    else {
                $classpath="$classpath:$1";
	    }
	}
	elsif ($line =~ m/\s*MainClass\s+(\S+)/) {
	    $mainClass = $1;
	}
	elsif ($line =~ m/\s*VMArgument\s+(\S+)/) {
	    push(@vmArguments, $1);
	}
	elsif ($line =~ m/\s*JavaArgument\s+(\S+)/) {
	    push(@javaArguments, $1);
	}
    }
    
    close(FH);
}

sub main
{
    readConfig();
    my $command = "java @vmArguments -classpath $classpath $mainClass @javaArguments";
    !system($command)
    	or die "Cannot execute $command: $!\n";
}

main();
