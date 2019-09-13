#!/usr/bin/perl
$netcat = "/usr/bin/nc";
$port = 4001;

print << "EOF";
Content-type: text/HTML

<html>
<head><title>
List of players, who are online now in the Shadow Realms"
</title></head>
<body bgcolor=#000000><font size=4>
<font color=#C0C0C0>List of players, who are online now in the Muddy Realms:

<pre>
EOF

open(LIST, "echo html | $netcat localhost $port |") || die "Cannot connect to server.";

#$max = <LIST>;
$found = 0;
while(<LIST>) {
	print;
	$found++;
}
printf "\n<font color=#C0C0C0>Players found: %d. Most so far today: %d.\n", $found, $max;

print << "EOF";
</pre>
</body>
</html>
EOF
