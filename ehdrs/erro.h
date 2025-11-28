/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- User (error) messages collected together.
 */

#define COPY_EMPTY	MESS(6000, "Empty copy buffer")
#define READ_BAD	MESS(6001, "Trouble with your how-to, see last line. Hit [interrupt] if you don't want this")
#define EDIT_TABS	MESS(6002, "Spaces and tabs mixed for indentation; check your program layout")
#define EXIT_HOLES	MESS(6003, "There are still holes left.  Please fill or delete these first.")
#define GOTO_BAD	MESS(6004, "I cannot [goto] that position")
#define GOTO_OUT	MESS(6005, "Sorry, I could not [goto] that position")
#define GOTO_REC	MESS(6006, "You can't use [goto] in recording mode")
#define INS_BAD		MESS(6007, "Cannot insert '%c'")
#define PLB_NOK		MESS(6008, "No keystrokes recorded")
#define REC_OK		MESS(6009, "Keystrokes recorded, use [play] to play back")
#define REDO_OLD	MESS(6010, "This redo brought you to an older version.  Use [undo] to cancel")
#define SUSP_BAD	MESS(6011, "Sorry, I failed to suspend ABC")
