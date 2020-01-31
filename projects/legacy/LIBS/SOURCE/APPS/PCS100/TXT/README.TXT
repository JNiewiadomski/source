Read Me  --  April 27, 1990
---------------------------

The PCS100 v0.1ë2 software diskette includes the source resource file
named PCS100.GR which is used to create the German version of the PCS100
program.  To make translating text from English to German as trouble free
as possible, please make all modifications regarding text to this file.

NOTE: This file is TEXT ONLY.  If using a word processing program such as
Microsoft Word, please save the changes as a TEXT FILE.

The PCS100 diskette contains the following files:
	AUTOEXEC.BAT	Batch file which is automatically executed when
			computer boots up.  This file automates the
			procedure of invoking the PCS100 software, and is
			only needed on systems running the PCS100 software
			off of a floppy diskette.

	CONFIG.SYS	System configuration file.  This file is used to
			configure the system when the computer boots up.
			If the PCS100 software is being run off of a hard
			disk and the file CONFIG.SYS already exists, then
			this file should not be copied to the hard disk.
			Instead, the person installing the software should
			verify that the minimal configuration set in the
			supplied CONFIG.SYS file is equal to or less than
			the configuration on the hard disk.

	PCS100.EXE	The executable file of the PCS100 software.  Typing
			the command PCS100 will cause DOS to execute this
			file.  This file requires that the PCS100.RES file
			resides in the same location as the PCS100.EXE file.
			If the German language is selected, then the file
			PCS100.049 must also reside in the same location as
			the PCS100.EXE file.  At present, the two languages
			supported are English (US) and German (GR).

	PCS100.RES	The English version of the PCS100 resource file.
			This file must reside in the same location as the
			PCS100.EXE file.

	PCS100.049	The German version of the PCS100 resource file.
			If the user wishes to have the PCS100 software use
			the German language, then this file must reside in
			the same location as the PCS100.EXE file.

	PCS100.US	The English resource source file.  The PCS100
			software if data-driven.  This means that the actual
			data displayed is only loosly-linked to the PCS100
			software.  Changes can be made to this text file and
			compiled with the Vorne Symbolic Resource Compiler
			(SRC).  Once compiled, the PCS100 software will use
			the new text that has been supplied.  In this
			fashion, it is very easy to localize the PCS100
			software to any region that reads test from left to
			right and from top to bottom.
			
	PCS100.GR	The German resource source file.  See the description
			of the PCS100.US file.
