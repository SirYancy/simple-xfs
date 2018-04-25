#!bin/bash

rm -rf ./alice
rm -rf ./bob
rm -rf ./carol

cp -r ./reset/alice ./alice
cp -r ./reset/bob/. ./alice

mkdir ./bob
mkdir ./carol
