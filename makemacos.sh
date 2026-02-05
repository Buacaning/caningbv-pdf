#!/bin/bash
set -x

# CBV: Enable ccache for faster rebuilds
if which ccache >/dev/null 2>&1; then
  export CC="ccache cc"
  export CXX="ccache c++"
  echo "ccache enabled: $(ccache --version | head -1)"
fi

#brew install sdl2
export G=`git rev-list HEAD --count`
echo $G
make -f Makefile.macos GIT_BUILD='${G}'  $1
MR=$?
if [ $MR -eq 0 ]; then
	mv build/release/mupdf-gl build/release/mupdf-macos
	./make-macos-bundle.sh
	cp Info.plist cbvpdf.app/Contents
#	/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f cbvpdf.app
fi
exit $MR
