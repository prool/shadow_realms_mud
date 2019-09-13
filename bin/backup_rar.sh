#!/bin/sh
#	by Xor for SR in 2002
#	good idea is add this script in crontab ;)

DATE=`/bin/date "+%y.%m.%d"`
PATH=/home/mud/mud
TMPPATH=/home/mud/tmp
BACKUPPATH=/home/mud/backups

CP=/bin/cp
MV=/bin/mv
RM=/bin/rm
LS=/bin/ls
ECHO=/bin/echo
AWK=/bin/awk
MKDIR=/bin/mkdir
TAR=/bin/tar
GZIP=/bin/gzip
SORT=/bin/sort
TEST=/usr/bin/test
WC=/usr/bin/wc
HEAD=/usr/bin/head
UUFILE=$TMPPATH/bb_$$_mud.uu
RAR=/usr/local/bin/rar
TOMAIL=backup@mud.nnov.ru
MAIL=/bin/mail
BC=/usr/bin/bc
UUENCODE=/usr/bin/uuencode

if !($TEST -d $TMPPATH/mud)
	then $MKDIR $TMPPATH/mud
fi

$CP -R $PATH/player $TMPPATH/mud/
$CP -R $PATH/gods $TMPPATH/mud/
$CP -R $PATH/etc $TMPPATH/mud/
$CP -R $PATH/log $TMPPATH/mud/
$CP -R $PATH/notes $TMPPATH/mud/
cd $TMPPATH/mud

$RAR a -m5 -r sr-$DATE.rar * > /dev/null 2>&1
$MV -f sr-$DATE.rar $BACKUPPATH

$RM -rf $TMPPATH/mud/player

$RAR a -m5 -df -r -v500k sr-part-$DATE.rar * > /dev/null 2>&1
$MV -f sr-part-$DATE.r?? $BACKUPPATH

$RM -rf $TMPPATH/mud

$RM -rf $PATH/log/*.slog
$RM -rf $PATH/log/debug.*

cd $BACKUPPATH
max=`$LS -1 sr-part-$DATE.r?? | $WC -l | $AWK '{print $1}'`
maxm=`$ECHO $max-1|$BC`
count=0
filelist=`$LS sr-part-$DATE.rar;$LS -1 sr-part-$DATE.r?? | $SORT | $HEAD -$maxm`
for file in $filelist; do
	count=`$ECHO $count+1|$BC`
	$ECHO "Backup of SR" > $UUFILE
	$ECHO "Part $count (max $max)" >> $UUFILE
	if [ $count -eq $max ]; then
		$ECHO "Last part" >> $UUFILE
	fi
	$ECHO >> $UUFILE
	$UUENCODE $file $file >> $UUFILE
	$MAIL -s "Backup SR($count/$max)" $TOMAIL < $UUFILE
done

$RM -rf $UUFILE
$RM -rf sr-part-$DATE.r??

