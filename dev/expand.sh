#!/bin/bash
#
# Expand markdown files (particularly includes)
#

if [[ -x ./node_modules/markdown-include/bin/cli.js ]]; then
	cli=$PWD/node_modules/markdown-include/bin/cli.js
elif [[ -x ~/node_modules/markdown-include/bin/cli.js ]]; then
	cli=~/node_modules/markdown-include/bin/cli.js
else
	echo "Unknown location of cli.js for markdown-include"
	exit 1
fi

cd master

lecture_dirname=lecture-notes # where the Markdown files are generated
if [[ ! -d $lecture_dirname ]]; then
	mkdir "$lecture_dirname"
fi

year=$( ls -1 input | sort -n | tail -1 )
for jsonfile in input/$year/*.json; do
	$cli "$jsonfile"
done

# copy the generated files to temporary area
mkdir /tmp/notes
cp $lecture_dirname/*.md /tmp/notes
