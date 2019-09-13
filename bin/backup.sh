#!/bin/bash

DATE=`/bin/date "+%y.%m.%d"`
PATH=/home/mud/mud
TMPPATH=/home/mud/tmp
BACKUPPATH=/home/mud/backups
CP=/bin/cp
MV=/bin/mv
RM=/bin/rm
MKDIR=/bin/mkdir
TAR=/bin/tar
GZIP=/bin/gzip
TEST=/usr/bin/test
#UUFILE=$TMPPATH/bb_$$_mud.uu
RAR=/usr/bin/rar

if !($TEST -d $TMPPATH/mud)
	then $MKDIR $TMPPATH/mud
fi

$CP -R $PATH/player $TMPPATH/mud/
$CP -R $PATH/gods $TMPPATH/mud/
#$CP -R $PATH/clans $TMPPATH/mud/
$CP -R $PATH/log $TMPPATH/mud/
$CP -R $PATH/notes $TMPPATH/mud/
cd $TMPPATH/mud
$TAR -cf sr-$DATE.tar *
$GZIP -9 $TMPPATH/mud/sr-$DATE.tar
$MV $TMPPATH/mud/sr-$DATE.tar.gz $BACKUPPATH


$RM -rf $TMPPATH/mud
$RM -rf $PATH/log/*.slog
$RM -rf $PATH/debug.*

#
$RM -rf $UUFILE
/bin/echo "SR backup created: $DATE" > $UUFILE
/bin/echo >> $UUFILE
/bin/echo "File: $BACKUPPATH/log-$DATE.tar.gz"
#/bin/cat $BACKUPPATH/log-$DATE.tar.gz | /usr/bin/uuencode log-$DATE.tar.gz >> $UUFILE
#
/bin/cat $UUFILE | /bin/mail -s "Backup MUD" mud@mud.nnov.ru
$RM -rf $UUFILE
#$RM -rf $BACKUPPATH/log-$DATE.tar.gz

