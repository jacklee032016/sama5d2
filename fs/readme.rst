Read me for File System package
################################
April 17th, 2019

Usage
============================
05.16, 2019

Keep only one tar.gz file in this directory;

* if ``minimal`` existed, delete it;
* ``loadTar.sh`` to uncompress tar file into minimal;
* Modify or update files in ``minimal`` as wish;
* ``mkJffs.sh`` to make new JFFS2 image which is updated onboard by tftp in u-boot;


Update logs
============================
04.17, 2019
---------------
32.4MB tar.gz; uncompressed directory: 105MB; packaged JFFS2 image: 38MB;


New version
============================
Based on tar.gz file system: uncompress it, then modify and package again:

Old Version
============================
Based on one JFFS2 image to modify and package again:

* load.sh: mount JFFS2 file system in Linux file system on PC;
* mkjffs.sh: package a new JFFS2 file system from a local directory;


