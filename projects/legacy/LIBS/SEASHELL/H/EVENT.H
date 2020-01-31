#ifndef _EVENT_H
#define _EVENT_H

// *****************
// * Event Manager *
// *****************

#define EKS_RSHIFT	0x0001		// keyboard toggles and shift keys
#define EKS_LSHIFT	0x0002
#define EKS_CTRL	0x0004
#define EKS_ALT		0x0008
#define EKS_SCROLL	0x0010
#define EKS_NUM		0x0020
#define EKS_CAPS	0x0040
#define EKS_INS		0x0080
#define EKS_RBUTTON	0x0100
#define EKS_LBUTTON	0x0200
#define EKS_EVERY	0x03FF

#define MSG_MOUSE_NONE	0x00
#define MSG_MOUSE_LEFT	0x01
#define MSG_MOUSE_RIGHT	0x02
#define MSG_MOUSE_BOTH	0x03

#define EVT_NULL	0x00
#define EVT_MOUSE_DOWN	0x01
#define EVT_MOUSE_UP	0x02
#define EVT_KEY		0x08
#define EVT_USER1	0x10
#define EVT_USER2	0x20
#define EVT_USER3	0x40
#define EVT_USER4	0x80
#define EVT_EVERY	0xFF

typedef void (*SysTaskFunc)(void);

class C_Saver {
	DWord	delay;
	short	vnum;
	Boolean	on;
	DWord	rate;
	TTimer	timer;
public:
	C_Saver(void) { SetDelay(180000L); rate=3000L; on=False; }
	~C_Saver(void) { Active(); }

	Boolean	Active(void);
	DWord	GetDelay(void) { return (delay); }
	void	Idle(void);
	void	On(void) { timer.Reset(0); }
	void	SetDelay(DWord new_delay) { delay=new_delay; timer.Reset(delay); }
	};

struct EventRecord {
	Word		what;		// event code
	Word		message;	// event message
	Point		where;		// mouse location
	Word		modifiers;	// modifier flags
	};

struct EventQueue {
	EventQueue	*link;		// next queue entry
	EventRecord	ev;		// event record
	};

class C_Event {
	static Boolean		once;		// one-time initialization

	static C_Mouse		mouse;
	static short		numEvents;
	static C_Saver		saver;
	static EventQueue	*theEvQueue;
	static Word		theMouseButton;
	static SysTaskFunc	theTask;	// background task for idle

	Word		theEvMask;
	EventRecord	theEvent;

	void	Null(void);
public:
	C_Event(void);

	void	ActivateSaver(void) { saver.On(); }
	Boolean	Avail(Word eventMask);
	void	Delay(DWord d);
	void	Flush(Word eventMask, Word stopMask);
	Boolean	GetNext(Word eventMask);
	DWord	GetSaverDelay(void) { return (saver.GetDelay()); }
	Word	Message(void) { return (theEvent.message); }
	Word	Modifiers(void) { return (theEvent.modifiers); }
	Boolean	Post(Word what, Word message);
	void	Register(void);
	void	SetMask(Word mask) { theEvMask = mask; }
	void	SetMessage(Word message) { theEvent.message = message; }
	void	SetSaverDelay(long delay) { saver.SetDelay(delay); }
	void	SetTask(SysTaskFunc task) { theTask = task; }
	void	WaitKey(void);
	Word	What(void) { return (theEvent.what); }
	Point	Where(void) { return (theEvent.where); }
	};

#endif
