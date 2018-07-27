#!/usr/bin/perl -w

use strict;


open I, "websource" or die;

my (@tag, $ltag, $etag) = ((), "", "");

print <<_HEADER;
<style>
h2 {
	background-color:lightblue;
	color:blue;
	padding-top:10;
	padding-left:10;
	padding-bottom:30;
}
body, p, ul, img, iframe {
	margin-left:50;
	margin-right:10;
	font-family:Verdana;
}
</style>
_HEADER

my $skipped = 0;

while (<I>) {
	chomp;
	/^:v\s+(\w+)/ and do {
		print <<_YOUTUBE;
<iframe width="560" height="315" src="https://www.youtube.com/embed/$1" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>
<br><br>
_YOUTUBE
		next;
	};
	/^:j\s+([\w\.]+)\s?([\w\.]*)/ and do {
		print "<br><img src=\"images/$1\" width=\"320\">";
		if (length $2) {
			print "<img src=\"images/$2\" width=\"320\">";
		}
		else {
			print "<br><br>\n";
		}
		print "<br><br>\n";
		next;
	};

	/^:(\w+)$/ and do {
		push @tag, $1;
		print "<$1>\n";
		$1 eq 'ul' and ($ltag, $etag) = ("li", "");
		next;
	};
	/^:(\w+)\s+(.+)$/ and do {
		print "<$1>$2</$1><br>\n";
		next;
	};
	/^:$/ and do {
		printf "</%s>\n", pop @tag;
		($ltag, $etag) = ("", "");
		next;
	};
	/^([\s\t])*$/ and do {
		$skipped or print "<br>\n";
		$skipped = 1;
		next;
	};
	$skipped = 0;
	length $_ < 20 && $#tag == 1 and do {
		print "<b>$_</b><br>\n";
		next;
	};
	s/l\((.+);([^\)]+)\)/<a href="$2">$1<\/a>/g;
	s/([bs])\(([^\)]+)\)/<$1>$2<\/$1>/g;
	if ($ltag) {
		$ltag and print "<$ltag>$_</$ltag>\n";
	}
	else {
		if (m/<b>/|| $#tag > 1) {
			print "$_\n";
		}
		else {
			print "<p>$_</p>\n";
		}
	}
}

close I;
