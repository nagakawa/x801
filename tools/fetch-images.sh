#!/bin/bash

# sudo apt install jq

if [ -z "$1" ]; then
	echo "Please provide a page to scrape."
	exit 1
fi
if [ -z "$2" ]; then
	echo "Please provide an output directory."
	exit 1
fi

path=$2
mkdir -p $path
# Replace spaces with underscores
title=$( sed 's/ /_/g' <<< $1 )
useragent='User-Agent: fetch-images.sh (uruwi@protonmail.com)'

function getContent {
	curl -s "https://commons.wikimedia.org/w/api.php?action=query&format=json&prop=revisions&titles=$title&rvprop=content" \
		-H "$useragent"
}

function getFilenames {
	getContent | grep -o "File:[^|]*|" | sed 's/^File://g;s/|$//g' | grep -v ".og[gv]$"
}

function getFileInfo {
	curl -s "https://commons.wikimedia.org/w/api.php?action=query&format=json&prop=imageinfo%7Crevisions&meta=&titles=File:$1&iiprop=url&rvprop=content" \
		-H "$useragent"
}

function getFileLocation {
	info=$( getFileInfo $1 )
	content=$( (jq '.query.pages|.[]|.revisions|.[0]|.["*"]' \
		| sed 's/\\n/\n/g') <<< "$info" )
	licence=$( grep -Eo '\{{2}[A-Za-z0-9\-]+\}{2}' <<< $content )
	author=$( (grep -Eo '^|\s*Author\s*=.*$' \
		| sed -E 's/^|//g;s/\s*Author\s*=\s*//g;') <<< $content )
	(
		echo $1
		echo "Licensed under $licence. Author is $author."
	) >> "$path/credits.txt"
	(jq '.query.pages|.[]|.imageinfo|.[0]|.url' | sed 's/^"//g;s/"$//g') <<< $info
}

imageno=0
rm "$path/credits.txt" || true

function downloadFile {
	location=$( getFileLocation $1 )
	extension=${1##*.}
	fullname="$path"/SHITCAKES."$extension"
	curl -s $location -H "$useragent" > "$fullname"
	convert "$fullname" "$path/image_$imageno.png"
}

getFilenames | while read fname; do
	# Download each file
	truename=$( sed 's/ /_/g' <<< $fname )
	downloadFile "$truename"
	echo $fname
	let imageno=$imageno+1
done

rm "$path"/SHITCAKES.*