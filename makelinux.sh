#!/bin/bash
#
# Build linux binary on linux
set -x

if [ "$1" = "clean" ]; then
	make -f Makefile.general clean
	exit 0
fi

export G=`git rev-list HEAD --count`
echo $G
ZPR="cbvpdf-R${G}-linux.tar.gz"
make -j4 -f Makefile.general GIT_BUILD='${G}'  $1
MR=$?
if [ $MR -eq 0 ]; then
	mv build/release/mupdf-gl build/release/mupdf-linux
	if [ -d cbvpdf ]; then
		rm -rf cbvpdf
	fi
	mkdir cbvpdf
	cp build/release/mupdf-linux cbvpdf/cbvpdf
	tar zcvf ${ZPR} cbvpdf
fi
exit $MR
