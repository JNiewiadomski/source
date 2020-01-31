// resource version signiature
#define RES_SIG		"REZ v1.0 by Jack Niewiadomski"

// default resource source extension
#define RES_IN_EXT	".US"

// maximum lengths
#define RES_ERR_LEN	50		// error resource
#define RES_FORM_LEN	132		// form resource
#define RES_HELP_LEN	76		// help resource
#define RES_LIST_LEN	76		// list resource
#define RES_MENU_NAME	20		// menu name
#define RES_MENU_ITEM	50		// menu item
#define RES_MENU_META	7		// menu meta
#define RES_PROMPT_LEN	70		// prompt
#define RES_STAT_LEN	80		// status resource
#define RES_STR_LEN	80		// string resource
#define RES_VID_LEN	78		// video resource
#define RES_WIN_LEN	74		// window title

struct ResTable {
	long	type_id;		// resource type and id
	long	loc;			// location of resource in file
	void	*block;			// pointer to resource in memory
	short	preload		: 1;	// preload into memory
	short	purgeable	: 1;	// memory can be reused
	};

struct ResBar {
	Byte	num;			// number of colors in table
	Rect	loc;			// top, left, bottom, right
	short	id[1];			// dummy label to id table
	};

struct Attrib {
	Byte	color;			// color attribute
	Byte	mono;			// mono attribute
	};

struct ResColor {
	Byte	num;			// number of colors in table
	Attrib	attr[1];		// attribute pair table - dummy label
	};

struct ResCoord {
	Byte	num;			// number of coords in list
	Point	coord[1];		// dummy label to coordinate list
	};

struct ResElem {
	short	type;			// type of dialog element resource
	short	id;			// id of dialog element resource
	Point	loc;			// location of element placement
	short	help_id;		// id of help resource
	};

struct ResDialog {
	Byte	num;			// number of elements in table
	short	window_id;		// id of window resource
	short	status_id;		// id of status resource (NULL == none)
	Byte	status_idx;		// index into status resource
	ResElem	elem[1];		// table of elements
	};

struct ResError {
	Byte	num;			// number of error strings in list
	char	error[1];		// error strings list
	};

struct ResForm {
	Byte	num;			// number of form strings in list
	char	form[1];		// form strings list
	};

struct ResHelp {
	Word	num;			// number of help strings in list
	char	help[1];		// help strings list
	};

struct ResInternat {
	Byte	format;			// usa, europe, japan
	char	thousands;		// thousands separator
	char	decimal;		// decimal separator
	char	date;			// date separator
	char	time;			// time separator
	char	*datePrompt;		// pointers to data strings
	char	*timePrompt;
	char	*dateTemplate;
	char	*timeTemplate;
	char	*ltimeTemplate;
	char	data[1];		// date, time and long time template strings
	};

struct ResList {
	Byte	num;			// number of items in list
	Byte	len;			// length of each list string
	Byte	visible;		// number of visible items in list
	char	**list;			// pointer to list of strings
	char	data[1];		// string data
	};

struct ResMenu {
	Byte	num;			// number of items in menu
	Boolean	flash;			// menu name flash/noflash flag
	short	help_id;		// id of help resource (NULL == none)
	char	title[1];		// dummy label to menu title
	};				// menu item data follows menu title

struct ResMenuItem {
	short	popup_id;		// id of menu resource (NULL == none)
	short	help_id;		// id of help resource (NULL == none)
	char	item_meta[1];		// menu item and meta string dummy label
	};

struct ResPrompt {
	Byte	len;			// length of prompt
	Point	loc;			// local coordinate of prompt
	char	templ[1];		// dummy label to template string
	};				// followed by mask and default strings

struct ResStatus {
	Byte	num;			// number of status items in list
	char	status[1];		// status list
	};

struct ResString {
	Byte	num;			// number of strings in list
	char	string[1];		// strings list
	};

struct ResVersion {
	Byte	version;		// version number
	Byte	revision;		// revision number
	char	stage;			// stage
	Byte	release;		// release / bug fix
	char	*ver_str;		// version string pointer
	char	*country;		// country string pointer\
	char	*date;			// date string pointer
	char	data[1];		// version, country and date strings
	};

struct ResVideo {
	Byte	num;			// number of video lines
	Byte	len;			// length of each video line
	short	color_id;		// id of color resource
	Byte	color_idx;		// index into color resource
	char	video[1];		// video lines
	};

struct ResWindow {
	Rect	loc;			// top, left, bottom, right of window
	short	color_id;		// id of color resource
	Byte	color_idx;		// index into color resource
	Byte	frame;			// type of window frame;
	short	video_id;		// id of video resource (NULL == none)
	Point	video_loc;		// local vertical/horizontal of video placement
	char	title[1];		// window title
	};
