#!/bin/bash

set -e

wget -N https://unicode.org/Public/12.1.0/ucd/CaseFolding.txt
wget -N https://unicode.org/Public/12.1.0/ucd/PropList.txt
wget -N https://unicode.org/Public/12.1.0/ucd/SpecialCasing.txt
wget -N https://unicode.org/Public/12.1.0/ucd/UnicodeData.txt
wget -N https://unicode.org/Public/12.1.0/ucd/extracted/DerivedGeneralCategory.txt
./generate.py
