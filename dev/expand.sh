#!/bin/bash
#
# Expand markdown files (particularly includes)
#

YEAR=2020

lecture_dirname=lecture-notes # where the Markdown files are generated
if [[ ! -d $lecture_dirname ]]; then
	mkdir "$lecture_dirname"
fi

if [[ -x ./node_modules/markdown-include/bin/cli.js ]]; then
	cli=$PWD/node_modules/markdown-include/bin/cli.js
elif [[ -x ~/node_modules/markdown-include/bin/cli.js ]]; then
	cli=~/node_modules/markdown-include/bin/cli.js
else
	echo "Unknown location of cli.js for markdown-include"
	exit 1
fi

cd repo
for jsonfile in input/$YEAR/*.json; do
	$cli "$jsonfile"
done
