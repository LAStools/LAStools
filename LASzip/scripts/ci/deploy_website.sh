#!/bin/bash

echo "deploying docs for $TRAVIS_BUILD_DIR/docs"

builddir=$1
destdir=$2

echo "builddir: " $builddir
echo "destdir: " $destdir

DATE=$(date +'%y.%m.%d %H:%M:%S')

git clone git@github.com:LASzip/laszip.github.io.git $destdir/laszipdocs
cd $destdir/laszipdocs
git checkout master


cd $builddir/html
cp -rf * $destdir/laszipdocs

#cd $builddir/latex/
#cp PDAL.pdf $destdir/laszipdocs

cd $destdir/laszipdocs
git config user.email "howard+pdal-docs@hobu.co"
git config user.name "PDAL Travis docsbot"

git add -A
git commit -m "update with results of commit https://github.com/LASzip/LASzip/commit/$TRAVIS_COMMIT for ${DATE}"
git push origin master

