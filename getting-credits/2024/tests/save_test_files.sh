save_directory="$HOME/my_files"
echo "save_directory: $save_directory"
# Rest of your script...

# We want the config in the test root directory.
configvar=$(pwd)/$configvar

cd "$tmpdir" || exit 1

mkdir "$save_directory"
(($? != 0)) && echo "mkdir failed." && exit 1
$GNUTAR -c -f "$tarwithdir" "$emptydir"

printf "export tmpdir=$tmpdir\n\n" > "$configvar"
printf "export tarfile=$tarfile\n\n" >> "$configvar"

# Do not change aaa-file size, see below.
aaafile=aaa-file
typeset -i aaafileblocks=20
typeset -i aaafilepartialblocks=$(($aaafileblocks / 2))
typeset input="$aaafile	/dev/urandom	512	$aaafileblocks
file1.random	/dev/urandom	512	100
empty.data	/dev/zero	512	0
file.zero	/dev/zero	512	2
hello-world	/dev/urandom	1024	10
small-file	/dev/urandom	512	1
file2.zero	/dev/zero	512	0
file3.zero	/dev/zero	512	0
another-file2	/dev/urandom	512	50"

echo "Creating files:"
echo "$input" | while read -r fname source bs count; do
	echo "  $fname"
	dd if=$source of=$fname bs=$bs count=$count 2>/dev/null
	if (($? != 0)); then
		echo "ERROR: dd on '$fname'."
		exit 1
	fi
done

inputfiles=$(echo "$input" | awk '{ print $1 }' | LC_ALL=C sort)
printf "export inputfiles=\"$inputfiles\"\n\n" >> "$configvar"

echo "Creating a GNU tar archive '$tarfile'."
$GNUTAR -c -f "$tarfile" $inputfiles
(($? != 0)) && echo "$GNUTAR failed." && exit 1

# aaa-file has $aaafileblocks blocks, so put a header and half of aaa-file's
# block into the partial archive.
echo "Creating '$partial' truncated in the middle of an archived file."
dd if=$tarfile of=$partial count=$((aaafilepartialblocks + 1)) >/dev/null 2>&1
(($? != 0)) && echo "dd failed." && exit 1

echo "Creating a truncated file from $aaafile."
aaa1stblocks=$aaafile-1st-blocks
dd if=$aaafile of=$aaa1stblocks count=$aaafilepartialblocks
(($? != 0)) && echo "dd failed." && exit 1
printf "export aaafile=$aaafile\n\n" >> "$configvar"
printf "export aaa1stblocks=$aaa1stblocks\n\n" >> "$configvar"

# We assume that aaa-file (see above) is $aaafileblocks block long.
echo "Creating archives with missing ending zero block(s):"
printf "$onezeroblockmissing 1\n$twozeroblocksmissing 0\n" | \
    while read -r filename n; do
	dd if=$tarfile of=$filename count=$((aaafileblocks + 1)) >/dev/null 2>&1
	(($? != 0)) && echo "dd failed." && exit 1
	dd if=/dev/zero of=$filename seek=$((aaafileblocks + 1)) \
	    count=$n >/dev/null 2>&1
	(($? != 0)) && echo "dd failed with zero blocks." && exit 1
	echo "  $filename"
    done

exit 0

