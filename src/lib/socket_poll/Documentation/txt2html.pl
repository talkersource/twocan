#! /usr/bin/perl -w

use strict;

my $name = "Socket_poll documentation";

my $html_header = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n<html>\n<head>\n";
my $html_body = "  </head><body bgcolor=\"#FFFFFF\">";

my $html_footer = "\n</td></tr></table>\n</body></html>";

sub convert()
  {
    my $in_pre_tag = "";
    
    while (<IN>)
      {
	if (s!^Function: (.*)$!</td></tr></table><table width="80%"><tr><td bgcolor="#DDDDDD"><strong>Function: </strong> $1! ||
	    s!^Constant: (.*)$!</td></tr></table><table width="80%"><tr><td bgcolor="#DDDDDD"><strong>Constant: </strong> $1! ||
	    s!^ Explanation:\s*$!</td></tr><tr><td><strong>Explanation:</strong></td></tr><tr><td><p>! ||
	    s!^ Note:\s*$!</td></tr><tr><td><strong>Note:</strong></td></tr><tr><td><p>! ||
	    s!^Section:\s*(.*)$!</td></tr></table><table width="90%"><tr><td bgcolor="#DDDDFF"><h2>$1</h2>! ||
	    0)
	  {
	    # Do nothing...
	  }
	elsif (m!^ ([A-Z][a-z]+)(\[\d\]|\[ \.\.\. \])?: (.*)$!)
	  {
	    if (defined $2)
	      {
		if ($1 eq "Type")
		  {
		    $_ = "<br>$1<strong>$2</strong>: $3";
		  }
		else
		  {
		    $_ = "</td></tr><tr><td>$1<strong>$2</strong>: $3";
		  }
	      }
	    else
	      {
		if ($1 eq "Type" || $1 eq "Returns")
		  {
		    $_ = "<br>$1: $3";
		  }
		else
		  {
		    $_ = "</td></tr><tr><td>$1: $3";
		  }
	      }
	  }
	elsif (/^ \.\.\./)
	  {
	    if (/\.\.\.$/)
	      {
		$_ = "</pre>$_<br><pre>";
		$in_pre_tag = "</pre>";
	      }
	    else 
	      {
		$_ = "</pre>$_";
		$in_pre_tag = "";
	      }
	  }
	elsif (/\.\.\.$/)
	  {
	    $_ = "$_<br><pre>";
	    $in_pre_tag = "</pre>";
	  }
	elsif (!$in_pre_tag && /^  /)
	  {
	    $_ = "</p><p>$_";
	  }
	
	print OUT;
      }
  }

if (!open (IN, "<functions.txt"))
  {
    die "Open (read): $@";
  }

if (!open (OUT, ">functions.html"))
  {
    die "Open (write): $@";
  }

print OUT $html_header;
print OUT "<title>", "$name -- functions", "</title>", "\n";
print OUT $html_body;
print OUT "<table width=\"100%\"><tr><td bgcolor=\"#DDFFDD\">", 
  "<h1>", "$name -- functions", "</h1>", "\n";

convert();

print OUT $html_footer;

if (!open (IN, "<constants.txt"))
  {
    die "Open (read): $@";
  }

if (!open (OUT, ">constants.html"))
  {
    die "Open (write): $@";
  }

print OUT $html_header;
print OUT "<title>", "$name -- constants", "</title>", "\n";
print OUT $html_body;
print OUT "<table width=\"100%\"><tr><td bgcolor=\"#DDFFDD\">", 
  "<h1>", "$name -- constants", "</h1>", "\n";

convert();

print OUT $html_footer;

exit 0;
