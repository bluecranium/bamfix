bamfix
======

This is a C program to strip "/1" and "/2" from BAM QNAME fields. Is was an exercise in using the samtools C library, and is useful for the TCGA BAMs where the /1 and /2 can cause programs like htseq-count to fail. It will strip the last 2 characters from the QNAME of each alignment and save a new BAM file. While I have verified the output using some small samples, please use at your own risk.

To Install:

				git clone https://github.com/bluecranium/bamfix.git
				cd bamfix
				git clone git://github.com/samtools/samtools.git
				cd samtools
				make
				cd ..
				make

