; Valid separators include the following characters:
;   - space  (' ')
;   - tab    ('\t')
;   - return ('\n')
;   - comma  (',')
; Make sure that the same resource type is not redefined.
; Valid resource types include:
;   - ERROR
;   - FORM
;   - HELP
;   - MENU
;   - STRING
;   - VIDEO
;   - WINDOW
;   - END
; All resource definition files should end with the resource type END
;
; Labels that begin with an underscore ("_") are system dependant resources
; and must not be modified or removed.

INCLUDE "\\include\\tc\\seashell\\seashell"


; Error resource definition.
; Restrict the length of the error message to a maximum of 50 characters;
; otherwise the message will be truncated.

ERROR {
  tableWinNoMemERROR, "Window Manager out of memory.\n\a"
}


; Form resource definition.

FORM {
}

; Help resource definition.
; Make sure that the text does not exceed 76 characters.  Text that is
; surrounded by curly brackets ( '{' and '}' ) will be highlighted when
; displayed.

HELP {
  HelpItemAbout {
    "The TABLE software package was written in Borland International's Turbo C"
    "compiler v2.0."
    ""
    "The Greenleaf Comm Library v2.10 revision T3 by Greenleaf Software provided"
    "the routines necessary to establish high speed serial communication."
    ""
    "SeaShell by Vorne Industries Incorporated provided the menu driven"
    "windowing environment.  SeaShell is a proprietary library of routines"
    "written in Turbo C v2.0 and the Microsoft Macro Assembler v5.1."
  }

  HelpItemHelp {
    "{Help} is a context-sensitive on-line help facility that pops up to"
    "help the user when the help key {^H} is pressed."
    ""
    "A Help window contains text that explains some part of the program.  As"
    "users operate the program and move from feature to feature, the context"
    "of Help windows change to reflect the current window.  These changes are"
    "not seen; they occur within the software.  When a user presses the Help"
    "key, the appropriate Help message is displayed in a pop-up window."
    ""
    "Because the Help message is related to the current location in the"
    "program, the Help window is said to be context-sensitive.  Experienced"
    "users can ignore the Help features of a program; novices can press the"
    "Help key with every change in the program's condition and receive hints,"
    "reminders, or detailed instructions."
  }

  HelpItemQuit {
    "{Quit} terminates program execution.  All files are properly updated and"
    "closed, and the status of the machine is left in the state found.  The"
    "user may also press {^Q} to invoke the Quit command."
    ""
    "At times a serious error may occur that forces the program to terminate"
    "execution without the Quit command being used.  In such a case, an error"
    "message is displayed on the screen and the user is informed of all"
    "options available.  If the only option is to terminate program execution,"
    "the program attempts to exit as gracefully as possible by saving and"
    "closing as many files as it can and returning the system to a stable"
    "state."
  }
}


; Menu resource definition.
; Item attributes can be composed of the following:
;   - ""   : no special attributes
;   - "!"  : check item
;   - "("  : disable item
;   - ">"  : hierarchical menu
;   - "/x" : keyboard shortcut, substitute x with uppercase letter or number
;   - "=x" : hot key, substitute x with the active character

MENU {
  HelpMENU, "Help", FALSE, HelpMenuHelp {
    "About Table...", "=A",   "HelpItemAbout"
    "-",              "(",    ""
    "Help",           "=H/H", "HelpItemHelp"
    "-",              "(",    ""
    "Quit",           "=Q/Q", "HelpItemQuit"
  }

  SystemMENU, "System", FALSE, "" {
    "COM1",        "(=C>",  ""
    "Baud (9600)", "(=B>",  ""
    "-",           "(",    ""
    "Add",         "=A/A", ""
    "Modify",      "=M/M", ""
    "Delete",      "=D/D", ""
    "-",           "(",    ""
    "Binary",      "=B/1", ""
    "Decimal",     "=e/2", ""
    "Hex",         "=H/3", ""
    "-",           "(",    ""
    "Exit to DOS", "=x",   ""
  }

  BaudMENU, "Baud", FALSE, "" {
    "1200",  "", ""
    "9600",  "", ""
  }
}

; String resource definition.
; Restrict the length of the string to a maximum of 80 characters;
; otherwise the string will be truncated.

STRING {
  StrErrCreate, "Could not create file %s"
}

; Version Resource defenition.
; A new program has Version 1.0.  If there is a minor revision to the
; program, it is then labeled Version 1.1.  If there's a bug fix to
; Version 1.1, it's designated 1.1.1.  So, if you have a program that had a
; second bug fix to a third minor revision, it would be Version 1.3.2.
; If there's a major revision to the program, the first number is
; incremented.  If the program has gone through a major revision, four minor
; revisions, and six bug fixes, the current version is 2.4.6.
; There's also a development suffix, which is added to indicate how far along
; the product is.  There are four different stages: The earliest is '' for
; "development".  The next level is 'à' for "alpha".  Then 'á' for "beta".
; Finally, a '.' indicates the released version.  If you have a product
; labeled 1.31, it is the first development version of the third revision
; of the first release of the product.

VERSION {
	TABLE_Version,		; label to identify resource
	1,			; program version
	0,			; major revision
	,			; revision stage (development, alpha, beta, release)
	2,			; build number / bug fix in release version
	"U.S.A.",		; resource file language format
	".001",			; country code file extension
	"April 13, 1990"	; date of software release
	}

; Video resource definition.
; Valid attributes include:
;   BLACK,    BLUE,      GREEN,      CYAN,      RED,      MAGENTA,      BROWN,  LIGHTGRAY
;   DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, YELLOW, WHITE

VIDEO {

  AboutVIDEO, 0, 1, WHITE {
    "                   Vorne Industried Incorporated                    "
    "          5831 Northwest Highway, Chicago, Illinois  60631          "
    "                           (312) 775-9440                           "
    "                                                                    "
    "                                                by Jack Niewiadomski"
  }

  RegsVIDEO, 0, 0, CYAN {
    " Address  Hex     Decimal  Binary               Description                   "
    "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ"
  }

  AddVIDEO, 0, 1, WHITE {
    "Register Address:     "
  }

  DeleteVIDEO, 0, 1, WHITE {
    "Register Address:     "
  }

  BinaryVIDEO, 0, 1, WHITE {
    "Register      Value:                 "
  }

  DecimalVIDEO, 0, 1, WHITE {
    "Register      Value:      "
  }

  HexVIDEO, 0, 1, WHITE {
    "Register      Value:     "
  }
}


; Window Definition Resource
; Valid attributes include:
;   BLACK,    BLUE,      GREEN,      CYAN,      RED,      MAGENTA,      BROWN,  LIGHTGRAY
;   DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, YELLOW, WHITE

WINDOW {
  AboutWINDOW { 9, 4, 15, 75, MAGENTA, W1111, "", AboutVIDEO }
  RegsWINDOW { 1, 0, 17, 79, CYAN, W1111, " Registers ", RegsVIDEO }
  CommWINDOW { 18, 0, 20, 79, CYAN, W1111, " Communication ", "" }
  DataLinkWINDOW { 21, 0, 23, 79, CYAN, W1111, " Data Link ", "" }
  AddWINDOW { 5, 5, 7, 30, MAGENTA, W1111, " Add Register ", AddVIDEO }
  DeleteWINDOW { 5, 5, 7, 30, MAGENTA, W1111, " Delete Register ", DeleteVIDEO }
  BinaryWINDOW { 5, 5, 7, 45, MAGENTA, W1111, " Binary Value ", BinaryVIDEO }
  DecimalWINDOW { 5, 5, 7, 34, MAGENTA, W1111, " Decimal Value ", DecimalVIDEO }
  HexWINDOW { 5, 5, 7, 33, MAGENTA, W1111, " Hexadecimal Value ", HexVIDEO }
}


; No more resources defined.
; The resource definition file must end with END

END
