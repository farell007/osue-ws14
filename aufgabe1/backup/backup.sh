#!/bin/sh

# Backup of a folder
# The files to backup
backup_files="./src"
# The destination folder of the backup
dest="./backup"
# The Timestamp to make the backup unique^
day=$(date +%s)

archive_file="backup-$day.tgz"

echo "Backing up all files to $dest/$archive_file"
date
echo

tar czvf $dest/$archive_file $backup_files

echo 
echo "Backup finished"
date

ls -lh $dest
