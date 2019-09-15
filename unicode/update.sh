#!/bin/bash

set -e

wget -N https://unicode.org/Public/UCD/latest/ucd/CaseFolding.txt
wget -N https://unicode.org/Public/UCD/latest/ucd/PropList.txt
wget -N https://unicode.org/Public/UCD/latest/ucd/SpecialCasing.txt
wget -N https://unicode.org/Public/UCD/latest/ucd/UnicodeData.txt
wget -N https://unicode.org/Public/UCD/latest/ucd/extracted/DerivedGeneralCategory.txt
./generate.py
