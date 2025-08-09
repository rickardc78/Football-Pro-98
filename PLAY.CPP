/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
  PARTICULAR PURPOSE.

  Copyright (C) 1993-1996  Sierra On-Line.  All Rights Reserved.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
// PLAY.CPP
// Routines for D_PLAY class.


#include "all.h"
#include "vm_globs.h"
#include "resource.h"
#include "drawwin.h"
#include "winmain.h"
INCLUDE_ASSERT

#define	X_TITLE 300
#define X_MINI	350
#define X_FULL	10


// Module variables--------------------------------------------------

static char numPos[17];


// File variables----------------------------------------------------

char far *putBuf = NULL;
short D_PLAY::PrnMinY = 0;
short D_PLAY::PrnMaxY = 0;


// Function declarations---------------------------------------------

void DrawPlayBox (int, int, char *, char *);


// D_PLAY public member functions------------------------------------


// ******************************************************************
// ******************************************************************

D_PLAY::D_PLAY()
{
	short player;
	CurrPlayer = PlayerMoving = -1;
	PlayerCount = EditedFlag = 0;
	DirType = 0;
	SpecialFlag = PlayCategory = UserCategory = 0;
	PlayFile = new char[MAXPATH + 1];
	PlayName = new char[13];
	CatStr = new char[30];
	PreSnapPlayer = AfterSnapPlayer = KickingPlayer = -1;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		PlayerPtr[player] = NULL;
		PlayerDepth[player].Pos = NULL_POS;
		PlayerDepth[player].Rank = 0;
		PlayerLogic[player][0] = NULL;
		PlayerLogic[player][1] = NULL;
		SavedLogic[player][0] = NULL;
		SavedLogic[player][1] = NULL;
	}
}

D_PLAY::D_PLAY(const D_PLAY *play)
{
	PlayerCount = play->PlayerCount;
	CurrPlayer = play->CurrPlayer;
	EditedFlag = play->EditedFlag;
	PlayerMoving = play->PlayerMoving;
	DirType = play->DirType;
	SpecialFlag = play->SpecialFlag;
	PlayCategory = play->PlayCategory;
	UserCategory = play->UserCategory;
	PlayFile = play->PlayFile;
	PlayName = play->PlayName;

	int i, j;

	for (i=0; i<NUM_PLAY_PLAYERS; i++)
		PlayerPtr[i] = play->PlayerPtr[i];

	for (i=0; i<NUM_PLAY_PLAYERS; i++)
		PlayerDepth[i] = play->PlayerDepth[i];

	for (i=0; i<NUM_PLAY_PLAYERS; i++)
		for (j=0; j<2; j++)
			PlayerLogic[i][j] = play->PlayerLogic[i][j];

	PreSnapPlayer = play->PreSnapPlayer;
	AfterSnapPlayer = play->AfterSnapPlayer;
	KickingPlayer = play->KickingPlayer;
	CatStr = play->CatStr;

	for (i=0; i<NUM_PLAY_PLAYERS; i++)
		for (j=0; j<2; j++)
			SavedLogic[i][j] = play->SavedLogic[i][j];
}

// ******************************************************************
// ******************************************************************

D_PLAY::~D_PLAY()
{
	short player;
	Erase();

	if (PlayFile)
		delete []PlayFile;

	if (PlayName)
		delete []PlayName;

	if (CatStr)
		delete []CatStr;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		if (PlayerLogic[player][0])
			delete PlayerLogic[player][0];

		if (PlayerLogic[player][1])
			delete PlayerLogic[player][1];

		if (SavedLogic[player][0])
			delete SavedLogic[player][0];

		if (SavedLogic[player][1])
			delete SavedLogic[player][1];
	}
}

void D_PLAY::Initialize(BOOL bOffPlay)
{
	if (bOffPlay)
	{
		strcpy(PlayName, "Off_Wiz");
		PlayCategory = UserCategory = 0xff;
	}
	else
	{
		strcpy(PlayName, "Def_Wiz");
		PlayCategory = UserCategory = 0xfe;
	}

	getcwd(PlayFile, MAX_PATH);
	strcat(PlayFile, "\\Custom\\");
	strcat(PlayFile, PlayName);
	PlayerMoving = -1;
}

// ******************************************************************
// ******************************************************************

void far *D_PLAY::operator new(size_t Size)
{
	return (void far *)vm_hmalloc(Size, MEMF_CLEAR);
}


// ******************************************************************
// ******************************************************************

void D_PLAY::operator delete(void far *Ptr)
{
	vm_hfree((char huge *)Ptr);
}


// ******************************************************************
// ******************************************************************
// bScale is TRUE if the play needs to be scaled (used for the field, not the mini-plays)

short D_PLAY::Load(InputFileHandler &InFile, short size, BOOL bScale)
{
	short player, playPart, numLogic;
	short command, comArgs, logicBoxNum, type, x, y;
	short fileOff[NUM_PLAY_PLAYERS];
	char *PlayBuffer, *BuffPtr;
	D_COMMAND *com;
	D_LOGIC_BOX *lb;
	PreSnapPlayer = AfterSnapPlayer = KickingPlayer = -1;

	if (PlayerCount)
		Erase();

	PlayBuffer = new char[size];

	if (PlayBuffer == NULL)
		return -1;

	InFile.Read(PlayBuffer, size);
	BuffPtr = PlayBuffer;
	memcpy(fileOff, BuffPtr, sizeof(fileOff));
	PlayCategory = *(BuffPtr + 22);
	SpecialFlag = *(BuffPtr + 23);

	// This is put in to set the user category.
	if (fileOff[0] == 24) // If this is an old stock play that I made...
		UserCategory = PlayCategory;  // See OldLoad...
	else
		UserCategory = *(BuffPtr + 24);

	for (player=0; player<NUM_PLAY_PLAYERS; player++)
	{
		BuffPtr = PlayBuffer + fileOff[player];
		PlayerDepth[player].Rank = *(short *) BuffPtr;
		BuffPtr += 2;
		PlayerDepth[player].Pos = (D_POSITION) *(short *) BuffPtr;
		BuffPtr += 2;

		for (playPart=PRE_SNAP; playPart<END_OF_PLAY; playPart++)
		{
			numLogic = *(short *) BuffPtr;
			BuffPtr += 2;

			for (logicBoxNum=0; logicBoxNum<numLogic; logicBoxNum++)
			{
				x = *(short *) BuffPtr;
				BuffPtr += 2;
				y = *(short *) BuffPtr;
				BuffPtr += 2;
				FieldToEdit(&x, &y);		// This scales the play to the larger field size of FB97

				if (!logicBoxNum)
					lb = PlayerLogic[player][playPart] = new D_LOGIC_BOX(logicBoxNum, player, x, y);
				else
					lb = lb->nextLogicBox =	new D_LOGIC_BOX(logicBoxNum, player, x, y);

				lb->comList.comCnt = *(short *) BuffPtr;
				BuffPtr += 2;

				for (command=0; command<lb->comList.comCnt;	command++)
				{
					com = &lb->comList.com[command];
					type = *(short *) BuffPtr;
					BuffPtr += 2;
					com->type = type & 0xff;
					comArgs = (type >> 8) & 0xf;

					if (comArgs == 1)
					{
						com->data1 = *(short *) BuffPtr;
						BuffPtr += 2;
					}
					else if (comArgs == 2)
					{
						x = *(short *) BuffPtr;
						BuffPtr += 2;
						y = *(short *) BuffPtr;
						BuffPtr += 2;

						if (com->type != COM::PASS_DEFENSE)
							FieldToEdit(&x, &y);		// This scales the play to the larger field size of FB97

						com->data1 = x;
						com->data2 = y;
					}
				}
			}
		}
	}

	delete [] PlayBuffer;

	for (player=0; player<NUM_PLAY_PLAYERS; player++)
	{
		if (PlayerDepth[player].Rank & 0x0100)
			PreSnapPlayer = player;

		if (PlayerDepth[player].Rank & 0x0200)
			AfterSnapPlayer = player;

		if (PlayerDepth[player].Rank & 0x0400)
			KickingPlayer = player;

		PlayerDepth[player].Rank &= 0x00ff;
	}

	PlayerCount = 11;
	DirType = !(PlayCategory & 1);

	if (SpecialFlag)
		DirType += 2;

	return ((PlayCategory & 1) + 1);
}


// Load a Wizard formation

short D_PLAY::LoadFormation(InputFileHandler &InFile, short size)
{
	short player;
	short x, y;
	short fileOff[NUM_PLAY_PLAYERS];
	char *PlayBuffer, *BuffPtr;
	D_LOGIC_BOX *lb;
	PreSnapPlayer = AfterSnapPlayer = KickingPlayer = -1;

	if (PlayerCount)
		Erase();

	PlayBuffer = new char[size];

	if (PlayBuffer == NULL)
		return -1;

	InFile.Read(PlayBuffer, size);
	BuffPtr = PlayBuffer;
	memcpy(fileOff, BuffPtr, sizeof(fileOff));
#if 0
	PlayCategory = *(BuffPtr + 22);
	SpecialFlag = *(BuffPtr + 23);

	// This is put in to set the user category.
	if (fileOff[0] == 24) // If this is an old stock play that I made...
		UserCategory = PlayCategory;  // See OldLoad...
	else
		UserCategory = *(BuffPtr + 24);
#endif

	for (player=0; player<NUM_PLAY_PLAYERS; player++)
	{
		BuffPtr = PlayBuffer + fileOff[player];
		PlayerDepth[player].Rank = *(short *) BuffPtr;
		BuffPtr += 2;
		PlayerDepth[player].Pos = (D_POSITION) *(short *) BuffPtr; 
		BuffPtr += 2;

		// Only pre-snap
		// Only one logic box
		x = *(short *) BuffPtr;
		BuffPtr += 2;
		y = *(short *) BuffPtr;
		BuffPtr += 2;
		FieldToEdit(&x, &y);			// This scales the play to the larger field size of FB97

		lb = PlayerLogic[player][PRE_SNAP] = new D_LOGIC_BOX(0, player, x, y);

		// fill in MIDDLE_OF_PLAY as well
		PlayerNum = player;					// PlayerMoved needs it
		PlayerLogic[player][MIDDLE_OF_PLAY] = new D_LOGIC_BOX(0, player, 0, 0);		// Initialize after snap logic
		PlayerMoved(player, -1, -1);				// Fills in MIDDLE_OF_PLAY numbers
	}

	delete []PlayBuffer;

	for (player=0; player<NUM_PLAY_PLAYERS; player++)
	{
		if (PlayerDepth[player].Rank & 0x0100)
			PreSnapPlayer = player;

		if (PlayerDepth[player].Rank & 0x0200)
			AfterSnapPlayer = player;

		if (PlayerDepth[player].Rank & 0x0400)
			KickingPlayer = player;

		PlayerDepth[player].Rank &= 0x00ff;
	}

	PlayerCount = 11;
	return ((PlayCategory & 1) + 1);
}


// Loads a Wizard pre snap logic play

short D_PLAY::LoadPreSnapLogic(InputFileHandler &InFile, short size)
{
	short player, numLogic;
	short command, comArgs, logicBoxNum, type, x, y;
	short fileOff[NUM_PLAY_PLAYERS];
	char *PlayBuffer, *BuffPtr;
	D_COMMAND *com;
	D_LOGIC_BOX *lb;
	PreSnapPlayer = AfterSnapPlayer = KickingPlayer = -1;

	if (PlayerCount)
		Erase();

	PlayBuffer = new char[size];

	if (PlayBuffer == NULL)
		return -1;

	InFile.Read(PlayBuffer, size);
	BuffPtr = PlayBuffer;
	memcpy(fileOff, BuffPtr, sizeof(fileOff));

#if 0		// UserCategory and PlayCategory set in the Initialize member function
	PlayCategory = *(BuffPtr + 22);
	SpecialFlag = *(BuffPtr + 23);

	// This is put in to set the user category.
	if (fileOff[0] == 24) // If this is an old stock play that I made...
		UserCategory = PlayCategory;  // See OldLoad...
	else
		UserCategory = *(BuffPtr + 24);
#endif

	for (player=0; player<NUM_PLAY_PLAYERS; player++)
	{
		BuffPtr = PlayBuffer + fileOff[player];
		PlayerDepth[player].Rank = *(short *) BuffPtr;
		BuffPtr += 2;
		PlayerDepth[player].Pos = (D_POSITION) *(short *) BuffPtr;
		BuffPtr += 2;

		numLogic = *(short *) BuffPtr;
		BuffPtr += 2;

		for (logicBoxNum=0; logicBoxNum<numLogic; logicBoxNum++)
		{
			x = *(short *) BuffPtr;
			BuffPtr += 2;
			y = *(short *) BuffPtr;
			BuffPtr += 2;
			FieldToEdit(&x, &y);				// This scales the play to the larger field size of FB97

			if (!logicBoxNum)
				lb = PlayerLogic[player][PRE_SNAP] = new D_LOGIC_BOX(logicBoxNum, player, x, y);
			else
				lb = lb->nextLogicBox =	new D_LOGIC_BOX(logicBoxNum, player, x, y);

			lb->comList.comCnt = *(short *) BuffPtr;
			BuffPtr += 2;

			for (command=0; command<lb->comList.comCnt;	command++)
			{
				com = &lb->comList.com[command];
				type = *(short *) BuffPtr;
				BuffPtr += 2;
				com->type = type & 0xff;
				comArgs = (type >> 8) & 0xf;

				if (comArgs == 1)
				{
					com->data1 = *(short *) BuffPtr;
					BuffPtr += 2;
				}
				else if (comArgs == 2)
				{
					x = *(short *) BuffPtr;
					BuffPtr += 2;
					y = *(short *) BuffPtr;
					BuffPtr += 2;

					if (com->type != COM::PASS_DEFENSE)
						FieldToEdit(&x, &y);		// This scales the play to the larger field size of FB97

					com->data1 = x;
					com->data2 = y;
				}
			}
		}
		PlayerNum = player;
		PlayerLogic[player][MIDDLE_OF_PLAY] = new D_LOGIC_BOX(0, player, 0, 0);		// Initialize after snap logic
		PlayerMoved(player, -1, -1);				// Fills in MIDDLE_OF_PLAY numbers
	}

	delete []PlayBuffer;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		if (PlayerDepth[player].Rank & 0x0100)
			PreSnapPlayer = player;

		if (PlayerDepth[player].Rank & 0x0200)
			AfterSnapPlayer = player;

		if (PlayerDepth[player].Rank & 0x0400)
			KickingPlayer = player;

		PlayerDepth[player].Rank &= 0x00ff;
	}

	PlayerCount = 11;
	DirType = !(PlayCategory & 1);

	if (SpecialFlag)
		DirType += 2;

	return ((PlayCategory & 1) + 1);
}


// ******************************************************************
// ******************************************************************

short D_PLAY::OldLoad(InputFileHandler &InFile, short size)
{
	short player, playPart, numLogic;
	short command, comArgs, logicBoxNum, type, x, y;
	short fileOff[NUM_PLAY_PLAYERS];
	char *PlayBuffer, *BuffPtr;
	D_COMMAND *com;
	D_LOGIC_BOX *lb;
	PreSnapPlayer = AfterSnapPlayer = KickingPlayer = -1;

	if (PlayerCount)
		Erase();

	PlayBuffer = new char[size];

	if (PlayBuffer == NULL)
		return -1;

	memset(numPos, 0, sizeof(numPos));
	InFile.Read(PlayBuffer, size);
	BuffPtr = PlayBuffer;
	memcpy(fileOff, BuffPtr, sizeof(short) * NUM_PLAY_PLAYERS);
	UserCategory = PlayCategory = *(BuffPtr + 22);  // Normally old plays use this
	SpecialFlag = *(BuffPtr + 23);

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		BuffPtr = PlayBuffer + fileOff[player];
		BuffPtr += 2;
		PlayerDepth[player].Pos = (D_POSITION)*(short *)BuffPtr;
		BuffPtr += 2;
		PlayerDepth[player].Rank = IDToRank(PlayerDepth[player].Pos);

		for (playPart = PRE_SNAP; playPart < END_OF_PLAY; playPart++)
		{
			numLogic = *(short *)BuffPtr;
			BuffPtr += 2;

			for (logicBoxNum = 0; logicBoxNum < numLogic; logicBoxNum++)
			{
				x = *(short *)BuffPtr;
				BuffPtr += 2;
				y = *(short *)BuffPtr;
				BuffPtr += 2;
				FieldToEdit(&x, &y);				// This scales the play to the larger field size of FB97

				if (!logicBoxNum)
					lb = PlayerLogic[player][playPart] = new D_LOGIC_BOX(logicBoxNum, player, x, y);
				else
					lb = lb->nextLogicBox = new D_LOGIC_BOX(logicBoxNum, player, x, y);

				lb->comList.comCnt = *(short *)BuffPtr;
				BuffPtr += 2;

				for (command = 0; command < lb->comList.comCnt; command++)
				{
					com = &lb->comList.com[command];
					type = *(short *)BuffPtr;
					BuffPtr += 2;
					com->type = type & 0xff;
					comArgs = (type >> 8) & 0xf;

					if (comArgs == 1)
					{
						com->data1 = *(short *)BuffPtr;
						BuffPtr += 2;
					}
					else if (comArgs == 2)
					{
						x = *(short *)BuffPtr;
						BuffPtr += 2;
						y = *(short *)BuffPtr;
						BuffPtr += 2;
						FieldToEdit(&x, &y);		// This scales the play to the larger field size of FB97
						com->data1 = x;
						com->data2 = y;
					}
				}
			}
		}
	}

	delete []PlayBuffer;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		if (PlayerDepth[player].Pos == OFF_CENTER)
		{
			PreSnapPlayer = player;
		}
		else if (PlayerDepth[player].Pos == QUARTERBACK)
		{
			AfterSnapPlayer = player;
		}
		else if ((PlayerDepth[player].Pos == KICKER) ||
				(PlayerDepth[player].Pos == PUNTER))
		{
			if ((AfterSnapPlayer == -1) &&
					(SpecialFlag != D_PLAYCAT::KICKOFF) &&
					(SpecialFlag != D_PLAYCAT::ONSIDE_KICK) &&
					(SpecialFlag != D_PLAYCAT::SQUIB_KICK) &&
					(SpecialFlag != D_PLAYCAT::FREE_KICK))
				AfterSnapPlayer = player;

			KickingPlayer = player;
		}
	}

	PlayerCount = 11;
	DirType = !(PlayCategory & 1);

	if (SpecialFlag)
		DirType += 2;

	return ((PlayCategory & 1) + 1);
}


// ******************************************************************
// ******************************************************************

short D_PLAY::Save(OutputFileHandler &OutFile)
{
	short player, playPart, numLogic, x, y, size;
	short command, comArgs, type;
	D_COMMAND *com;
	short fileOff[NUM_PLAY_PLAYERS];
	D_LOGIC_BOX *lb, *tlb;
	long tablePos;
	tablePos = OutFile.GetPosition();
	OutFile.MarkSpot();
	OutFile.Write(fileOff, sizeof(short) * NUM_PLAY_PLAYERS);
	OutFile.Write(&PlayCategory, sizeof(char));
	OutFile.Write(&SpecialFlag, sizeof(char));
	OutFile.Write(&UserCategory, sizeof(char));

	for (player=0; player<NUM_PLAY_PLAYERS; player++)
	{
		fileOff[player] = (short) (OutFile.GetPosition() - tablePos);

		if (player == PreSnapPlayer)
			PlayerDepth[player].Rank |= 0x0100;

		if (player == AfterSnapPlayer)
			PlayerDepth[player].Rank |= 0x0200;

		if (player == KickingPlayer)
			PlayerDepth[player].Rank |= 0x0400;

		OutFile.Write(&PlayerDepth[player].Rank, sizeof(short));
  		PlayerDepth[player].Rank &= 0x00ff;
		OutFile.Write(&PlayerDepth[player].Pos, sizeof(short));

		for (playPart=PRE_SNAP; playPart<END_OF_PLAY; playPart++)
		{
			lb = tlb = PlayerLogic[player][playPart];
			numLogic = 1;

			while (tlb->nextLogicBox)
			{
				tlb = tlb->nextLogicBox;
				numLogic++;
			}

			OutFile.Write(&numLogic, sizeof(short));

			do
			{
				x = lb->x; y = lb->y;

				EditToField(&x, &y);				// This scales the field data to prepare for writing
				OutFile.Write(&x, sizeof(short));
				OutFile.Write(&y, sizeof(short));
				OutFile.Write(&lb->comList.comCnt, sizeof(short));

				for (command=0; command<lb->comList.comCnt; command++)
				{
					com = &lb->comList.com[command];
					type = com->type;
					comArgs = ComSize(type & 0xff);

					if (((type & 0xff) == COM::PASS_DEFENSE) &&	(com->data1 == 2))
						type |= 2 << 8;
					else
						type |= comArgs << 8;

					OutFile.Write(&type, sizeof(short));

					if (comArgs == 2)
					{
						x = com->data1; y = com->data2;
						EditToField(&x, &y);				// This scales the field data to prepare for writing
						OutFile.Write(&x, sizeof(short));
						OutFile.Write(&y, sizeof(short));
					}
					else if (comArgs == 1)
					{
						OutFile.Write(&com->data1, sizeof(short));

						if (((type & 0xff) == COM::PASS_DEFENSE) &&	(com->data1 == 2))
							OutFile.Write(&com->data2, sizeof(short));
					}
				}

				lb = lb->nextLogicBox;
			}
			while (lb);
		}
	}

	OutFile.GotoSpot();
	OutFile.Write(fileOff, sizeof(short) * NUM_PLAY_PLAYERS);
	OutFile.DeleteSpot();
	size = (short) (OutFile.GetPosition() - tablePos);
	EditedFlag = FALSE;
	return size;
}


// For Sierra only

short D_PLAY::SaveFormation(OutputFileHandler &OutFile)
{
	short player, x, y, size;
	short fileOff[NUM_PLAY_PLAYERS];
	D_LOGIC_BOX *lb, *tlb;
	long tablePos;
	tablePos = OutFile.GetPosition();
	OutFile.MarkSpot();
	OutFile.Write(fileOff, sizeof(short) * NUM_PLAY_PLAYERS);
	OutFile.Write(&PlayCategory, sizeof(char));
	OutFile.Write(&SpecialFlag, sizeof(char));
	OutFile.Write(&UserCategory, sizeof(char));

	for (player=0; player<NUM_PLAY_PLAYERS; player++)
	{
		fileOff[player] = (short) (OutFile.GetPosition() - tablePos);

		if (player == PreSnapPlayer)
			PlayerDepth[player].Rank |= 0x0100;

		if (player == AfterSnapPlayer)
			PlayerDepth[player].Rank |= 0x0200;

		if (player == KickingPlayer)
			PlayerDepth[player].Rank |= 0x0400;

		OutFile.Write(&PlayerDepth[player].Rank, sizeof(short));
  		PlayerDepth[player].Rank &= 0x00ff;
		OutFile.Write(&PlayerDepth[player].Pos, sizeof(short));

		lb = tlb = PlayerLogic[player][PRE_SNAP];
		x = lb->x; y = lb->y;

		EditToField(&x, &y);				// This scales the field data to prepare for writing
		OutFile.Write(&x, sizeof(short));
		OutFile.Write(&y, sizeof(short));
	}

	OutFile.GotoSpot();
	OutFile.Write(fileOff, sizeof(short) * NUM_PLAY_PLAYERS);
	OutFile.DeleteSpot();
	size = (short) (OutFile.GetPosition() - tablePos);
	return size;
}


// For Sierra only

short D_PLAY::SavePreSnapLogic(OutputFileHandler &OutFile)
{
	short player, numLogic, x, y, size;
	short command, comArgs, type;
	D_COMMAND *com;
	short fileOff[NUM_PLAY_PLAYERS];
	D_LOGIC_BOX *lb, *tlb;
	long tablePos;
	tablePos = OutFile.GetPosition();
	OutFile.MarkSpot();
	OutFile.Write(fileOff, sizeof(short) * NUM_PLAY_PLAYERS);
	OutFile.Write(&PlayCategory, sizeof(char));
	OutFile.Write(&SpecialFlag, sizeof(char));
	OutFile.Write(&UserCategory, sizeof(char));

	for (player=0; player<NUM_PLAY_PLAYERS; player++)
	{
		fileOff[player] = (short) (OutFile.GetPosition() - tablePos);

		if (player == PreSnapPlayer)
			PlayerDepth[player].Rank |= 0x0100;

		if (player == AfterSnapPlayer)
			PlayerDepth[player].Rank |= 0x0200;

		if (player == KickingPlayer)
			PlayerDepth[player].Rank |= 0x0400;

		OutFile.Write(&PlayerDepth[player].Rank, sizeof(short));
  		PlayerDepth[player].Rank &= 0x00ff;
		OutFile.Write(&PlayerDepth[player].Pos, sizeof(short));

		lb = tlb = PlayerLogic[player][PRE_SNAP];
		numLogic = 1;

		while (tlb->nextLogicBox)
		{
			tlb = tlb->nextLogicBox;
			numLogic++;
		}

		OutFile.Write(&numLogic, sizeof(short));

		do
		{
			x = lb->x; y = lb->y;

			EditToField(&x, &y);				// This scales the field data to prepare for writing
			OutFile.Write(&x, sizeof(short));
			OutFile.Write(&y, sizeof(short));
			OutFile.Write(&lb->comList.comCnt, sizeof(short));

			for (command=0; command<lb->comList.comCnt; command++)
			{
				com = &lb->comList.com[command];
				type = com->type;
				comArgs = ComSize(type & 0xff);

				if (((type & 0xff) == COM::PASS_DEFENSE) &&	(com->data1 == 2))
					type |= 2 << 8;
				else
					type |= comArgs << 8;

				OutFile.Write(&type, sizeof(short));

				if (comArgs == 2)
				{
					x = com->data1; y = com->data2;
					EditToField(&x, &y);				// This scales the field data to prepare for writing
					OutFile.Write(&x, sizeof(short));
					OutFile.Write(&y, sizeof(short));
				}
				else if (comArgs == 1)
				{
					OutFile.Write(&com->data1, sizeof(short));

					if (((type & 0xff) == COM::PASS_DEFENSE) &&	(com->data1 == 2))
						OutFile.Write(&com->data2, sizeof(short));
				}
			}

			lb = lb->nextLogicBox;
		}
		while (lb);
	}

	OutFile.GotoSpot();
	OutFile.Write(fileOff, sizeof(short) * NUM_PLAY_PLAYERS);
	OutFile.DeleteSpot();
	size = (short) (OutFile.GetPosition() - tablePos);
	EditedFlag = FALSE;
	return size;
}

// ******************************************************************
// ******************************************************************

// JDW - 96-08-15 - More rewriting to get more than one play
//					on page for plan-printing

void D_PLAY::Print(short playNum, short posInPlan)
{
	short		partSave = PlayPart;
	short		i, pSave = PlayerNum, iLine = 1;
	short		command, textCol, textLine, prevTextline;
	char		PlayTitle[81];
	D_COMMAND	*com;
	D_LOGIC_BOX *lb;
	TEXTMETRIC	sTM;
	long		xTitle;

 	PrnMinY = 455;
	PrnMaxY = 735;

	/* Set scaling limits for diagram */
	if (Printer->Diag == PRN_FULL)
	{
		PlayPart = PRE_SNAP;

		while (PlayPart <= MIDDLE_OF_PLAY)
		{
			for (i=0; i<PlayerCount; i++)
			{
				lb = PlayerLogic[i][PlayPart];

				while (lb)
				{
					CheckPrnLimit(lb->y);

					for (command=0; command<lb->comList.comCnt; command++)
					{
						com = &lb->comList.com[command];

						switch (com->type)
						{
							case COM::BLOCK_MOVE_TO:
							case COM::MOVE_TO:
							case COM::PASS:
								CheckPrnLimit(com->data2);
								break;

							case COM::PASS_DEFENSE:
								if (com->data1 == 1)
								{
									CheckPrnLimit((com + 1)->data2);
									CheckPrnLimit((com + 2)->data2);
									command += 2;
								}
								break;
						}
					}

					lb = lb->nextLogicBox;
				}
			}

			PlayPart++;
		}
	}

	/* Set the drawing window */
	if (playNum == -1)
		pgSetViewPort(0, 0, 1999, 3999);
	else
		GetPlayers();

	/* Make play title string */
	PlayTitle[0] = 0;

	if (posInPlan != -1)
	{
		Gameplan->PlayNumToRowColStr(posInPlan, PlayTitle);
		strcat(PlayTitle, " - ");
	}

	int	cPlay = playNum;	// JDW - 96-08-16 - We need to save the old version for plan-printing

	playNum++;
	strcat(PlayTitle, PlayName);

	if ((Printer->Diag == IDC_RADIO_PRINT_MINI)  &&  (Printer->Text == IDC_RADIO_PRINT_NONE2))
		Printer->LoadAltFont("Arial", 70, 0, FONT_NORMAL, FALSE);
	else
		Printer->LoadAltFont("Arial", 150, 0, FONT_BOLD, FALSE);
	Printer->SetAltFont();

	GetTextMetrics(Printer->GetPrinterDC(), &sTM);

	// JDW - 96-08-19 - Just print play list
	if ((Printer->Diag == IDC_RADIO_PRINT_NONE)  &&  (Printer->Text == IDC_RADIO_PRINT_NONE2)  &&  (Printer->Printout == IDC_RADIO_PRINT_PLAN))
	{
		DrawPlayBox(Printer->hMarginList + (((playNum - 1) % 4) * Printer->hPlayList),
			Printer->vMarginList + (((playNum - 1) / 4) * Printer->vPlayList),
			PlayTitle, GetCatStr());
		return;
	}

	// JDW - 96-08-15 - Print more than one play per page
	if (playNum > 1)
	{
		if ((Printer->Diag == IDC_RADIO_PRINT_MINI)  &&  (Printer->Text == IDC_RADIO_PRINT_NONE2))
		{
		}
		else if (Printer->Diag == IDC_RADIO_PRINT_MINI)
			Printer->SetCurLine((Printer->vPrinterRes>>1)/Printer->GetLineHeight());
	}
	else 
		Printer->SetCurLine(0);

	if ((Printer->Diag == IDC_RADIO_PRINT_FULL_SIZE) || !(Printer->Text == IDC_RADIO_PRINT_NONE2) || !playNum)
	{
		if (PlayerCount)
		{
			strcat(PlayTitle, " - ");
			strcat(PlayTitle, GetCatStr());
		}
		xTitle = sTM.tmAveCharWidth * strlen((PCHAR) PlayTitle);
		TextOut( Printer->GetPrinterDC(), (Printer->hPrinterRes - xTitle)/2, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) PlayTitle, strlen((PCHAR) PlayTitle));
		Printer->SetCurLine(Printer->GetCurLine() + iLine);
	}
	else if ((Printer->Diag == IDC_RADIO_PRINT_MINI)  &&  (Printer->Text == IDC_RADIO_PRINT_NONE2)  &&  (Printer->Printout == IDC_RADIO_PRINT_PLAN))
	{
		xTitle = sTM.tmAveCharWidth * strlen((PCHAR) PlayTitle);
		TextOut( Printer->GetPrinterDC(), Printer->hMarginMini + ((cPlay % 4) * Printer->hPlayMini) + ((Printer->hPlayMini - xTitle)/2), Printer->vMarginMini + ((cPlay >> 2) * (Printer->vPlayMini + Printer->vTextMarginMini)) - Printer->vTextOffsetMini, (PCHAR) PlayTitle, strlen((PCHAR) PlayTitle));
	}
	else if (Printer->Diag == IDC_RADIO_PRINT_MINI)
	{
		xTitle = sTM.tmAveCharWidth * strlen((PCHAR) PlayTitle);
		TextOut( Printer->GetPrinterDC(), (Printer->hPrinterRes - xTitle)/2, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) PlayTitle, strlen((PCHAR) PlayTitle));
		Printer->SetCurLine(Printer->GetCurLine() + iLine);
	}
	else
	{
		xTitle = sTM.tmAveCharWidth * strlen((PCHAR) PlayTitle);
		TextOut( Printer->GetPrinterDC(), (Printer->hPrinterRes - xTitle)/2, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) PlayTitle, strlen((PCHAR) PlayTitle));
		Printer->SetCurLine(Printer->GetCurLine() + iLine);

		if (PlayerCount || SpecialFlag)
		{
			TextOut( Printer->GetPrinterDC(), (Printer->hPrinterRes - xTitle)/2, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) GetCatStr(), strlen((PCHAR) GetCatStr()));
			Printer->SetCurLine(Printer->GetCurLine() + iLine);
		}
		else
		{
			TextOut( Printer->GetPrinterDC(), (Printer->hPrinterRes - xTitle)/2, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) "--------", strlen((PCHAR) "--------"));
			Printer->SetCurLine(Printer->GetCurLine() + iLine);
		}
	}
	Printer->SetStandardFont();

	switch (Printer->Diag)
	{
		case IDC_RADIO_PRINT_MINI:
			RECT rctMiniPlay;
			rctMiniPlay.left = rctMiniPlay.top = 0;
			rctMiniPlay.right = ShapeTable2[0].width;
			rctMiniPlay.bottom = ShapeTable2[0].height;

			// Prepare the offscreen buffer
			FillRect(hdcOsbBuffer, &FieldViewRect, GetStockObject(WHITE_BRUSH));
			ClearMiniPlay(FALSE);
			DrawMiniPlayMem(&rctMiniPlay, FALSE);

			// Print it out
			if ((Printer->Text == IDC_RADIO_PRINT_NONE2) && (Printer->Printout == IDC_RADIO_PRINT_PLAN))
				BitBlt(Printer->GetPrinterDC(),
					Printer->hMarginMini + ((cPlay % 4) * Printer->hPlayMini),
					Printer->vMarginMini + ((cPlay >> 2) * (Printer->vPlayMini + Printer->vTextMarginMini)),
					Printer->hPlayMini, Printer->vPlayMini, hdcOsbBuffer, 0, 0, SRCCOPY);
			else
				StretchBlt(Printer->GetPrinterDC(), 0, (Printer->GetCurLine() * Printer->GetLineHeight()), Printer->hPlayMini, Printer->vPlayMini, 
					hdcOsbBuffer, 0, 0, rctMiniPlay.right, rctMiniPlay.bottom, SRCCOPY);
			break;

		case IDC_RADIO_PRINT_FULL_SIZE:
			DrawFieldBackGround(FALSE);
			Draw();
			BitBlt(Printer->GetPrinterDC(), (Printer->hPrinterRes - Printer->hPlayFull)/2, 
				(Printer->GetCurLine() * Printer->GetLineHeight()), Printer->hPlayFull, Printer->vPlayFull, 
				hdcOsbBuffer, 0, 0, SRCCOPY);
			break;
	}

	/* Print the logic text for each player */
	textLine = 150;
	textCol = 0;

	if (Printer->Diag == IDC_RADIO_PRINT_FULL_SIZE)
	{
		Printer->SetCurLine(((Printer->GetCurLine() * Printer->GetLineHeight()) + Printer->vPlayFull + 10 + Printer->GetLineHeight())/Printer->GetLineHeight());
		textCol = (Printer->hPrinterRes - Printer->hPlayFull)/2;
	}
	else if (Printer->Diag == IDC_RADIO_PRINT_MINI)
		textCol = Printer->hPrinterRes/3;
	else
		textCol = Printer->hPrinterRes/10;

	Printer->LoadAltFont("Times New Roman", 80, 0, 0, FALSE);
	Printer->SetAltFont();

	if ((Printer->Text != IDC_RADIO_PRINT_NONE2) && !SpecialFlag)
	{
		pgSetTextScaling(3, 2, 3, 2);
		pgSetTextJustify(pgLEFT, pgCENTER);
		prevTextline = Printer->GetCurLine();

		for (i=0; i<PlayerCount; i++)
		{
			if (prevTextline == Printer->GetCurLine())
				Printer->SetCurLine(Printer->GetCurLine() + iLine);

			prevTextline = Printer->GetCurLine();
			TextOut( Printer->GetPrinterDC(), textCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) PlayerPtr[i]->GetPosStr(), strlen((PCHAR) PlayerPtr[i]->GetPosStr()));

			if ((i == PreSnapPlayer) && (this == DefPlay))
			{
				TextOut( Printer->GetPrinterDC(), textCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) "Preferred Player", strlen((PCHAR) "Preferred Player"));
				Printer->SetCurLine(Printer->GetCurLine() + iLine);
			}

			PlayPart = (Printer->Text == IDC_RADIO_PRINT_AFTER_SNAP_ONLY) ? MIDDLE_OF_PLAY : PRE_SNAP;

			while (PlayPart <= MIDDLE_OF_PLAY)
			{
				lb = PlayerLogic[i][PlayPart];

				while (lb)
				{
					textLine = lb->PrintCommandText(textCol, textLine, this);
					lb = lb->nextLogicBox;
				}

				PlayPart += (Printer->Text == IDC_RADIO_PRINT_BEFORE_SNAP) ? 2 : 1;
			}
		}
	}

	Printer->SetStandardFont();

	PlayPart = partSave;
	PlayerSelect(pSave);
}


// ******************************************************************
// ******************************************************************

void D_PLAY::PrintPlayer()
{
	short x = PlayerX, y = PlayerY;
	EditToPrint(&x, &y);

	if (Printer->Diag == PRN_FULL)
		pgDrawTextXY(x, y, ::PlayerPtr->GetPosStr());
	else
		pgRectangle(x - 5, y - 5, x + 5, y + 5, pgOFILL);
}


// ******************************************************************
// ******************************************************************

void D_PLAY::EditToPrint(short *x, short *y)
{
	long tempY = *y;
	*x *= 3;
	*x += 75;
	tempY -= PrnMinY;
	tempY *= (Printer->Diag == PRN_FULL) ? 1800 : 1760;
	tempY /= PrnMaxY - PrnMinY;
	tempY += (Printer->Diag == PRN_FULL) ? 50 : 20;

	if (Printer->Diag == PRN_MINI)
	{
		*x >>= 2;
		tempY >>= 2;
	}

	*y = (short)tempY;
}


// ******************************************************************
// ******************************************************************

void D_PLAY::Flip()
{
	short player, playPart, command, x;
	D_LOGIC_BOX *lbPre, *lbMid, *lb;
	D_COMMAND *com;

	if (PlayerCount)
	{
		EditedFlag = 1;
		PlayerMoving = -1;

		for (player = 0; player < PlayerCount; player++)
		{
			lbPre = PlayerLogic[player][PRE_SNAP];
			lbMid = PlayerLogic[player][MIDDLE_OF_PLAY];
			x = lbPre->x;

			if ((this == DefPlay) || (player != PreSnapPlayer)) // DJM-I put this in to 
				x = MIDFIELDX * 2 + 1 - x; // flip center logic.

			// fix symmetry
			if (x > MIDFIELDX * 2 - 7)
				x = MIDFIELDX * 2 - 7;
			else if (x < 7)
				x = 7;

			lbPre->x = x;

			for (playPart = PRE_SNAP; playPart < END_OF_PLAY; playPart++)
			{
				if (playPart > PRE_SNAP)
				{
					while (lbPre->nextLogicBox)
						lbPre = lbPre->nextLogicBox;

					lbMid->x = lbPre->x;
					lbMid->y = lbPre->y;
				}

				lb = PlayerLogic[player][playPart];

				while (lb)
				{
					for (command = 0; command < lb->comList.comCnt; command++)
					{
						com = &lb->comList.com[command];

						switch (com->type)
						{
							case COM::ONSIDE_KICK_LEFT:
								com->type = COM::ONSIDE_KICK_RIGHT;
								break;

							case COM::ONSIDE_KICK_RIGHT:
								com->type = COM::ONSIDE_KICK_LEFT;
								break;

							case COM::BLOCK: /* Switch push right & push left */
								if (com->data1 == 2)
									com->data1 = 3;
								else if (com->data1 == 3)
									com->data1 = 2;
								break;

							case COM::RUN: /* Switch run right & run left */
								if (com->data1 == 4)
									com->data1 = 5;
								else if (com->data1 == 5)
									com->data1 = 4;
								break;

							case COM::BLOCK_MOVE_TO:
							case COM::MOVE_TO:
							case COM::HANDOFF_TO:
							case COM::FAKE_HANDOFF_TO:
							case COM::FAKE_PASS_TO:
							case COM::PASS:
							case COM::MOTION_STOP:
							case COM::MOTION_REVERSE:
							case COM::MOTION:
							case COM::ZONE:
								FlipLogicX(&com->data1);
								break;
						}
					}

					if (lb = lb->nextLogicBox)	// first lb already flipped
						FlipLogicX(&lb->x);
				}
			}
		}
	}
}


// ******************************************************************
// ******************************************************************

void D_PLAY::EraseLogic(short player)
{
	// Delete all logic in play.
	short playerToErase, playPart;
	short firstPlayer = 0, lastPlayer = PlayerCount;

	if (player != -1)
	{
		firstPlayer = player;
		lastPlayer = player + 1;
	}

	D_LOGIC_BOX *lb;

	for (playerToErase=firstPlayer; playerToErase<lastPlayer; playerToErase++) // Do for each player.
	{
		for (playPart=PRE_SNAP; playPart<END_OF_PLAY; playPart++) // Delete logic after
		{
			lb = PlayerLogic[playerToErase][playPart];

			if (playPart != PRE_SNAP)
			{
				lb->x = PlayerLogic[playerToErase][PRE_SNAP]->x; // PRE_SNAP.
				lb->y = PlayerLogic[playerToErase][PRE_SNAP]->y; 
			}

			if (lb->nextLogicBox)
			{
				delete lb->nextLogicBox;
				lb->nextLogicBox = NULL;
			}

			lb->comList.comCnt = 0;
		}
	}
}


// ******************************************************************
// ******************************************************************

void D_PLAY::Erase()
{
	short player, playPart;

	for (player=0; player<PlayerCount; player++)
	{
		for (playPart=PRE_SNAP; playPart<END_OF_PLAY; playPart++)
		{
			if (PlayerLogic[player][playPart])
				delete PlayerLogic[player][playPart];

			PlayerLogic[player][playPart] = NULL;
		}
	}

	PlayerCount = 0;
	EditedFlag = FALSE;
}


// ******************************************************************
// ******************************************************************

D_LOGIC_BOX *D_PLAY::SaveLogic(D_LOGIC_BOX *lb)
{
	short player, playPart;
	short lbPart;
	D_LOGIC_BOX *lbPtr;
	short firstPlayer = 0, lastPlayer = NUM_PLAY_PLAYERS;

	if (lb)
	{
		firstPlayer = lb->player;
		lastPlayer = lb->player + 1;

		lbPart = PlayPart;
#if 0
		for (playPart=PRE_SNAP; playPart<END_OF_PLAY; playPart++)	// find lbPart
		{
			lbPtr = PlayerLogic[lb->player][playPart];

			while (lbPtr)
			{
				if (lbPtr == lb)
					lbPart = playPart;

				lbPtr = lbPtr->nextLogicBox;
			}
		}
#endif
	}

	for (player=firstPlayer; player<lastPlayer; player++)
		for (playPart=PRE_SNAP; playPart<END_OF_PLAY; playPart++)
			if (!SavedLogic[player][playPart])
			{
				SavedLogic[player][playPart] = PlayerLogic[player][playPart];
				PlayerLogic[player][playPart] = new D_LOGIC_BOX;
				PlayerLogic[player][playPart]->Copy(SavedLogic[player][playPart]);
			}

	if (lb)
	{
		lbPtr = PlayerLogic[lb->player][lbPart];

		while (lbPtr)
		{
			if (lbPtr->num == lb->num)
				lb = lbPtr;

			lbPtr = lbPtr->nextLogicBox;
		}
	}

	return lb;
}


// ******************************************************************
// ******************************************************************

// If restoreFlag is TRUE, revert the logic back to SavedLogic.  By default, restoreFlag is TRUE

void D_PLAY::RestoreLogic(short restoreFlag)
{
	short player, playPart;
	short oldX, oldY;			// In case user attaches logic, moves player, and then tries to undo

	for (player=0; player<NUM_PLAY_PLAYERS; player++)
		for (playPart=PRE_SNAP; playPart<END_OF_PLAY; playPart++)
			if (SavedLogic[player][playPart])
				if (restoreFlag)
				{
					if (PlayerLogic[player][playPart])
					{
						oldX = PlayerLogic[player][playPart]->x;
						oldY = PlayerLogic[player][playPart]->y;
						delete PlayerLogic[player][playPart];
					}

					PlayerLogic[player][playPart] =	SavedLogic[player][playPart];
					PlayerLogic[player][playPart]->x = oldX;
					PlayerLogic[player][playPart]->y = oldY;
					SavedLogic[player][playPart] = NULL;
				}
				else
				{
					delete SavedLogic[player][playPart];
					SavedLogic[player][playPart] = NULL;				
				}
}


// ******************************************************************
// ******************************************************************

void D_PLAY::PlayerSelect(short num)
{
	::PlayerPtr = PlayerPtr[num];
	PlayerNum = num;
	if (PlayerLogic[num][PlayPart])
	{
		PlayerX = PlayerLogic[num][PlayPart]->x;
		PlayerY = PlayerLogic[num][PlayPart]->y;
	}
	PlayerPosition = PlayerDepth[num].Pos;
}


// ******************************************************************
// ******************************************************************

short D_PLAY::OnField(D_PLAYER *playerPtr)
{
	for (short player=0; player<PlayerCount; player++)
	{
		if (PlayerPtr[player] == playerPtr)
			return TRUE;
	}

	return FALSE;
}


// ******************************************************************
// ******************************************************************

void D_PLAY::ActivateSub(short player, D_PLAYER *sub)
{
	PlayerPtr[player] = sub;
	PlayerDepth[player] = PlayerPtr[player]->GetDepth();
}


// ******************************************************************
// ******************************************************************

void D_PLAY::InitMoveTo(short num)
{
	short i;
	PlayerSelect(num);

#if 0

	if (ShowAllLogicFlag)
	{
		for (i = 0; i < PlayerCount; i++)
			if (i != num)
			{
				offenseFlag = GetReceiverColor(i);
				PlayerLogic[i][PlayPart]->DrawLogic(offenseFlag);
			}

		for (i = 0; i < PlayerCount; i++)
			if (i != num)
				PlayerLogic[i][PlayPart]->DrawLogicBox();
	}
	else
	{
		i = CurrPlayer;

		if (i != -1 && i != num)
		{
			lb = PlayerLogic[i][PlayPart];
			offenseFlag = GetReceiverColor(i);
			lb->DrawLogic(offenseFlag);
			lb->DrawLogicBox();
		}
	}

	for (i = 0; i < PlayerCount; i++)
	{
		if (i != num)
		{
			PlayerSelect(i);
			DrawPlayer();
		}
	}
#endif

	PlayerSelect(num);
	i = PlayerX - 18;

	if (i < CurBox.x)
		i = CurBox.x;

	vm_get(putBuf, i, PlayerY - 8 - FieldPos + FieldOffY, 36, 16);
	vm_clip = FALSE;
//	DrawTeams();
	PlayerMoving = num;
}


// ******************************************************************
// ******************************************************************

void D_PLAY::PlayerMoveTo(short NewX, short NewY)
{
	short clipX, nY;
	CurBox.Clip();

	D_POSITION pos = PlayerDepth[PlayerNum].Pos;

	// Restrict offensive linemen positions
	if (!Special() && (pos & P_POS_OL))
	{
		NewY = LINEOFSCRIM + (YARDH + HALF_YARDH);

		if (NewX < MIDFIELDX - (4 * YARDW))
			NewX = MIDFIELDX - (4 * YARDW);

		if (NewX > MIDFIELDX + (4 * YARDW))
			NewX = MIDFIELDX + (4 * YARDW);
	}

	if (PlayerBump(PlayerNum, NewX, NewY))
		DrawPlayer();
	else
	{
		if (FieldPos == PrevFieldPos)
		{
			clipX = PlayerX - 18;

			if (clipX < CurBox.x)
				clipX = CurBox.x;

			nY = PlayerY - 8 - FieldPos + FieldOffY;

			if (nY < vm_height - 16)	// don't want to draw below bottom of screen
				vm_put(putBuf, clipX, nY, 36, 16);
		}

		if (PlayerLogic[PlayerNum][PlayPart])
		{
			PlayerX = PlayerLogic[PlayerNum][PlayPart]->x = NewX;
			PlayerY = PlayerLogic[PlayerNum][PlayPart]->y = NewY;
		}
		clipX = PlayerX - 18;

		if (clipX < CurBox.x)
			clipX = CurBox.x;

		nY = NewY - 8 - FieldPos + FieldOffY;

		if (nY < vm_height - 16)		// don't want to draw below bottom of screen
		{
			vm_get(putBuf, clipX, nY, 36, 16);
			DrawPlayer();
		}
	}

	vm_clip = FALSE;
}


// ******************************************************************
// ******************************************************************

short D_PLAY::PlayerMoveable(short num)
{
	D_LOGIC_BOX *lb = PlayerLogic[num][PRE_SNAP];

	return !(lb->nextLogicBox ||
			(lb->comList.comCnt && 
			(lb->comList.com[0].type == COM::MOTION)) ||
			((num == PreSnapPlayer) && (this == OffPlay)));
}


// ******************************************************************
// ******************************************************************

D_PLAY *D_PLAY::OtherTeam()
{
	D_PLAY *newTeam = (this == OffPlay) ? DefPlay : OffPlay;

	if (!newTeam->Loaded())
		return this;

	return newTeam;
}


// ******************************************************************
// ******************************************************************

D_COMMAND *D_PLAY::getCom(short pl, short com, short playPart)
{
	short command;
	D_LOGIC_BOX *lb = PlayerLogic[pl][playPart];

	while (lb)
	{
		D_COMMAND_LIST *list = &lb->comList;

		for (command = 0; command < list->comCnt; command++)
		{
			if (list->com[command].type == com)
				return &list->com[command];
		}

		lb = lb->nextLogicBox;
	}

	return NULL;
}


// ******************************************************************
// ******************************************************************

void D_PLAY::PlayerMoved(short num, short x, short y)
{
	D_LOGIC_BOX **lbArr, *lb;
	short playerSave = PlayerNum;
	lbArr = PlayerLogic[num];

	if (PlayPart == PRE_SNAP)
	{
		if (x != -1)
		{
			lbArr[MIDDLE_OF_PLAY]->x = x;
			lbArr[MIDDLE_OF_PLAY]->y = y;
			EditedFlag = TRUE;
		}
		else
		{
			lb = lbArr[PRE_SNAP];

			while (lb->nextLogicBox)
				lb = lb->nextLogicBox;

			if ((lbArr[MIDDLE_OF_PLAY]->x != lb->x) || (lbArr[MIDDLE_OF_PLAY]->y != lb->y))
			{
				lbArr[MIDDLE_OF_PLAY]->x = lb->x;
				lbArr[MIDDLE_OF_PLAY]->y = lb->y;
				EditedFlag = TRUE;
			}
		}
	}

	PlayerMoving = -1;
	PlayerSelect(num);
//	DrawPlayer();	CWC
	vm_clip = FALSE;
	PlayerSelect(playerSave);
}


// ******************************************************************
// ******************************************************************

D_LOGIC_BOX *D_PLAY::LogicFind(short x, short y)
{
	short i;
	D_LOGIC_BOX *lb;

	if (CurrPlayer >= 0)
	{
		lb = PlayerLogic[CurrPlayer][PlayPart];

		while (lb)
		{
			if (lb->num)
			{
				if (abs(lb->x - x) < 4 && abs(lb->y - y) < 3)
					return lb;
			}
			else
				if (abs(lb->x - x) < 8 && abs(lb->y - y) < 6)
					return lb;

			lb = lb->nextLogicBox;
		}
	}

	for (i=0; i<PlayerCount; i++)
	{
		lb = PlayerLogic[i][PlayPart];

		while (lb)
		{
			if (lb->num)
			{
				if (abs(lb->x - x) < 4 && abs(lb->y - y) < 3)
					return lb;
			}
			else
				if (abs(lb->x - x) < 8 && abs(lb->y - y) < 6)
					return lb;

			lb = lb->nextLogicBox;
		}
	}

	return NULL;
}


// ******************************************************************
// ******************************************************************

void D_PLAY::Draw()
{
	short i, routeColor, pSave = PlayerNum, cp = CurrPlayer;

	if (ShowAllLogicFlag)
	{
		for (i = 0; i < PlayerCount; i++)
		{
			if (i != cp)
			{
				routeColor = GetReceiverColor(i);
				PlayerLogic[i][PlayPart]->DrawLogic(routeColor);
			}
		}

		for (i = 0; i < PlayerCount; i++)
		{
			if (i != cp)
				PlayerLogic[i][PlayPart]->DrawLogicBox();
		}
	}

	for (i = 0; i < PlayerCount; i++)
	{
		if ((i != cp) && (i != PlayerMoving))
		{
			PlayerSelect(i);
			DrawPlayer();
		}
	}

	if (PlayerCount && (cp != -1) && (PlayerMoving != cp))
	{
		PlayerSelect(cp);
		routeColor = GetReceiverColor(cp);
		PlayerLogic[cp][PlayPart]->DrawLogic(routeColor);
		PlayerLogic[cp][PlayPart]->DrawLogicBox();
		DrawPlayer();
	}

	vm_clip = FALSE;
	PlayerSelect(pSave);
}


// ******************************************************************
// Draw a mini play which has been loaded (these are the mini-diagrams)
// ******************************************************************
// bNeedToScale is TRUE for save dialog box

void D_PLAY::DrawMiniPlayMem(RECT *pRectPic,
							 BOOL bColor)
{
	short i, playPartSave, offenseFlag, routeColor, x, y;
	char *str;

	if (pRectPic != NULL)
	{
		CurBox.x = CurBox.y = 0;
		CurBox.w = (short) (pRectPic->right - pRectPic->left);
		CurBox.h = (short) (pRectPic->bottom - pRectPic->top);
	}

	vm_clip = TRUE;
	vm_lclip = 0;
	vm_rclip = 639;
	vm_tclip = CurBox.y + 1;
	vm_bclip = CurBox.y + CurBox.h - 3;
	playPartSave = PlayPart;
	PlayPart = MIDDLE_OF_PLAY;

	for (i=0; i<PlayerCount; i++)
	{
		routeColor = GetReceiverColor(i);
		PlayerLogic[i][PlayPart]->DrawMiniLogicMem(routeColor, bColor);
	}

	offenseFlag = PlayCategory & 1;

	for (i=0; i<PlayerCount; i++)
	{
		PlayerSelect(i);
		x = PlayerX; y = PlayerY;

		EditToDiagram(&x, &y);

		short shapeNum = PlayerGetSymbolNum(PlayerDepth[i].Pos);

		if (offenseFlag && !SpecialFlag && (shapeNum >= S_BC))
			shapeNum = (PlayerNum == PreSnapPlayer) ? S_BC : S_BO;

		shapeNum += (S_BLB2 - S_BLB);

		if (shapeNum > S_BC2)
			shapeNum--;

		if (!offenseFlag)
			shapeNum += S_RLB2 - S_BLB2;

		vm_flip_draw(&ShapeTable[shapeNum], CurBox.x + x - 2, CurBox.y + y - 1, 0);
	}

	// Draw title
	vm_clip = FALSE;
	PlayPart = playPartSave;

	if (bColor)	// On a printout, don't do the title
	{
		vm_fontselect((void *) SmallTeamFont);
		str = GetCatStr();
		vm_fgcolor = 2;
		vm_hstr(str, CurBox.x + ((CurBox.w - vm_hstr_width(str) + 1) >> 1), CurBox.y + 2);
		vm_fgcolor = 3;
		vm_hstr(str, CurBox.x + ((CurBox.w - vm_hstr_width(str) + 1) >> 1), CurBox.y + 1);
	}

	if (Super)
	{
		vm_fgcolor = 2;
		vm_hstr(PlayName, CurBox.x + ((CurBox.w - vm_hstr_width(PlayName) + 1) >> 1), CurBox.y + CurBox.h - 10);
		vm_fgcolor = 3;
		vm_hstr(PlayName, CurBox.x + ((CurBox.w - vm_hstr_width(PlayName) + 1) >> 1), CurBox.y + CurBox.h - 11);
		str = GetCatStr((short)(SpecialFlag << 8) + PlayCategory);
		vm_fgcolor = 2;
		vm_hstr(str, CurBox.x + ((CurBox.w - vm_hstr_width(str) + 1) >> 1), CurBox.y + 12);
		vm_fgcolor = (offenseFlag) ? 1 : 14;
		vm_hstr(str, CurBox.x + ((CurBox.w - vm_hstr_width(str) + 1) >> 1), CurBox.y + 11);
	}
}


// This is called by the Wizard to draw formations and pre-snap logics, and play logics.  Drawn to pRectPic

#define WIZARD_SYMBOL_DIAMETER 6

void D_PLAY::DrawMiniWizardPlayMem(RECT *pRectPic,
								   char *str,
								   short man,
								   short PlayTypeflag,
								   BOOL bOffPlay)
{
	short i, playPartSave, offenseFlag, routeColor, x, y;

	short width = (short) (pRectPic->right-pRectPic->left);
	short height = (short) (pRectPic->bottom-pRectPic->top);

	if (pRectPic == NULL)
		MessageBox(NULL, "RECT is NULL in DrawMiniPlayMem", "", MB_OK);

	vm_clip = TRUE;
	vm_lclip = 0;
	vm_rclip = 639;
	vm_tclip = pRectPic->top+1;
	vm_bclip = pRectPic->bottom-3;

	playPartSave = PlayPart;		// Save this so we can draw the correct kind of play

	if (PlayTypeflag >= WIZARD_MIDDLE_PLAY)
		PlayPart = MIDDLE_OF_PLAY;
	else
		PlayPart = PRE_SNAP;		// WIZARD_MIDDLE_PLAY_OFFENSIVE_LINE_ONLY can use PRE_SNAP as offensive line doesn't move

	if ((PlayTypeflag != WIZARD_MIDDLE_PLAY_OFFENSIVE_LINE_ONLY) &&		// Don't draw any logic if offensive line only
			(PlayTypeflag != WIZARD_MIDDLE_PLAY_HANDOFF_OFFENSIVE_LINE_ONLY))
		for (i=0; i<PlayerCount; i++)
		{
			routeColor = GetReceiverColor(i);

//			if (i == man)
//				vm_lcolor = 3;	// yellow
//			else
//				vm_lcolor = 1;	// blue

			PlayerLogic[i][PlayPart]->DrawMiniWizardLogicMem(pRectPic, routeColor, PlayTypeflag);
		}

	offenseFlag = PlayCategory & 1;

	HBRUSH hBrushBlue = CreateSolidBrush(RGB(0,0,255));
	HBRUSH hBrushDarkBlue = CreateSolidBrush(RGB(0,0,128));
	HBRUSH hBrushRed = CreateSolidBrush(RGB(255,0,0));
	HBRUSH hBrushDarkRed = CreateSolidBrush(RGB(128,0,0));
	HBRUSH hBrushYellow = CreateSolidBrush(RGB(255,255,0));

	HPEN hPenBlue = CreatePen(PS_SOLID, 0, RGB(0,0,255));
	HPEN hPenDarkBlue = CreatePen(PS_SOLID, 0, RGB(0,0,128));
	HPEN hPenRed = CreatePen(PS_SOLID, 0, RGB(255,0,0));
	HPEN hPenDarkRed = CreatePen(PS_SOLID, 0, RGB(128,0,0));
	HPEN hPenYellow = CreatePen(PS_SOLID, 0, RGB(255,255,0));

	HBRUSH hOldBrush;
	HPEN hOldPen;

	for (i=0; i<PlayerCount; i++)
	{
		PlayerSelect(i);
		x = PlayerX; y = PlayerY;

		WizardEditToDiagram(PlayTypeflag, width, height, &x, &y);

		if (((PlayTypeflag == WIZARD_MIDDLE_PLAY_OFFENSIVE_LINE_ONLY) || (PlayTypeflag == WIZARD_MIDDLE_PLAY_HANDOFF_OFFENSIVE_LINE_ONLY)) && 
				!((PlayerDepth[i].Pos & P_POS_OL) ||
				(PlayerDepth[i].Pos == QUARTERBACK)))
			continue;		// Show offensive line only

		if (PlayTypeflag == WIZARD_MIDDLE_PLAY_OFFENSIVE_LINE_ONLY)	// Adjust for the art
			y += 15;

		if (PlayTypeflag == WIZARD_MIDDLE_PLAY_HANDOFF_OFFENSIVE_LINE_ONLY)
			y -= 12;

		if ((PlayTypeflag <= WIZARD_PRE_SNAP_LOGIC) && bOffPlay)		// Adjust play so it is further up on the box
			y -= 10;

		if ((PlayTypeflag <= WIZARD_PRE_SNAP_LOGIC) && !bOffPlay)		// Adjust play so it is further down on the box
			y += 15;

		if ((PlayTypeflag == WIZARD_MIDDLE_PLAY_OFFENSIVE_LINE_ONLY) ||		// Space it out a bit
				(PlayTypeflag == WIZARD_MIDDLE_PLAY_HANDOFF_OFFENSIVE_LINE_ONLY))
			x = (short) ((x-64)*2.5 + 64);

		short shapeNum = PlayerGetSymbolNum(PlayerDepth[i].Pos);

		if (offenseFlag && !SpecialFlag && (shapeNum >= S_BC))
			shapeNum = (PlayerNum == PreSnapPlayer) ? S_BC : S_BO;

		shapeNum += (S_BLB2 - S_BLB);

		if (shapeNum > S_BC2)
			shapeNum--;

		if (!offenseFlag)
			shapeNum += S_RLB2 - S_BLB2;

		if (i == man)			// Selected person always yellow
		{
			hOldBrush = SelectObject(hdcOsbBuffer, hBrushYellow);
			hOldPen = SelectObject(hdcOsbBuffer, hPenYellow);
		}
		else
			if (bOffPlay)
			{
				switch (PlayTypeflag)
				{
					case WIZARD_FORMATION:		// If only a formation or pre-snap logic, only 1 color
					case WIZARD_PRE_SNAP_LOGIC:
					case WIZARD_MIDDLE_PLAY_OFFENSIVE_LINE_ONLY:
						hOldBrush = SelectObject(hdcOsbBuffer, hBrushBlue);		// Just make it blue
						hOldPen = SelectObject(hdcOsbBuffer, hPenBlue);
						break;
					case WIZARD_MIDDLE_PLAY_RECEIVERS:
						if (WizardIsReceiver(i))		// all receivers and running backs are in light blue.  All others in dark blue
						{
							hOldBrush = SelectObject(hdcOsbBuffer, hBrushBlue);
							hOldPen = SelectObject(hdcOsbBuffer, hPenBlue);
						}
						else
						{
							hOldBrush = SelectObject(hdcOsbBuffer, hBrushDarkBlue);
							hOldPen = SelectObject(hdcOsbBuffer, hPenDarkBlue);
						}
						break;
					case WIZARD_MIDDLE_PLAY_QUARTERBACK:
						hOldBrush = SelectObject(hdcOsbBuffer, hBrushDarkBlue);
						hOldPen = SelectObject(hdcOsbBuffer, hPenDarkBlue);
						break;
					case WIZARD_MIDDLE_PLAY_OFFENSIVE_LINE:
						if (WizardIsOffensiveLineman(i))		// all offensive linebackers are in light blue.  All others in dark blue
						{
							hOldBrush = SelectObject(hdcOsbBuffer, hBrushBlue);
							hOldPen = SelectObject(hdcOsbBuffer, hPenBlue);
						}
						else
						{
							hOldBrush = SelectObject(hdcOsbBuffer, hBrushDarkBlue);
							hOldPen = SelectObject(hdcOsbBuffer, hPenDarkBlue);
						}
						break;
				}

			}
			else		// Defensive Play
			{
				switch (PlayTypeflag)
				{
					case WIZARD_FORMATION:		// If only a formation or pre-snap logic, only 1 color
					case WIZARD_PRE_SNAP_LOGIC:
						hOldBrush = SelectObject(hdcOsbBuffer, hBrushRed);		// Just make it red
						hOldPen = SelectObject(hdcOsbBuffer, hPenRed);
						break;
					case WIZARD_MIDDLE_PLAY:
						if (!WizardIsDefensiveBack(i))		// all defensive backs are in light red.  All others in dark red
						{
							hOldBrush = SelectObject(hdcOsbBuffer, hBrushDarkRed);
							hOldPen = SelectObject(hdcOsbBuffer, hPenDarkRed);
						}
						else
						{
							hOldBrush = SelectObject(hdcOsbBuffer, hBrushRed);
							hOldPen = SelectObject(hdcOsbBuffer, hPenRed);
						}						
						break;
					case WIZARD_MIDDLE_PLAY_DEFENSIVE_LINE:
						if (WizardIsDefensiveBack(i))		// all defensive backs are in dark red.  All others in light red
						{
							hOldBrush = SelectObject(hdcOsbBuffer, hBrushDarkRed);
							hOldPen = SelectObject(hdcOsbBuffer, hPenDarkRed);
						}
						else
						{
							hOldBrush = SelectObject(hdcOsbBuffer, hBrushRed);
							hOldPen = SelectObject(hdcOsbBuffer, hPenRed);
						}						
						break;
				}
			}

//		vm_flip_draw(&ShapeTable[shapeNum], pRectPic->left + x - 2, pRectPic->top + y - 1, 0);	// Circle not big enough
		if (bOffPlay)
			Ellipse(hdcOsbBuffer, 
				pRectPic->left + x - WIZARD_SYMBOL_DIAMETER/2, 
				pRectPic->top + y - WIZARD_SYMBOL_DIAMETER/2, 
				pRectPic->left + x + WIZARD_SYMBOL_DIAMETER/2, 
				pRectPic->top + y + WIZARD_SYMBOL_DIAMETER/2);
		else
		{
			if ((GetPosition(i).Pos == DEF_GUARD) || (GetPosition(i).Pos == DEF_TACKLE) || (GetPosition(i).Pos == DEF_END))
				Rectangle(hdcOsbBuffer, 
					pRectPic->left + x - WIZARD_SYMBOL_DIAMETER/2, 
					pRectPic->top + y - WIZARD_SYMBOL_DIAMETER/2, 
					pRectPic->left + x + WIZARD_SYMBOL_DIAMETER/2, 
					pRectPic->top + y + WIZARD_SYMBOL_DIAMETER/2);
			else if (GetPosition(i).Pos == LINEBACKER)
			{
				POINT pt[3];

				pt[0].x = pRectPic->left + x - WIZARD_SYMBOL_DIAMETER/2;
				pt[0].y = pRectPic->top + y + WIZARD_SYMBOL_DIAMETER/2;
				pt[1].x = pRectPic->left + x + WIZARD_SYMBOL_DIAMETER/2;
				pt[1].y = pRectPic->top + y + WIZARD_SYMBOL_DIAMETER/2;
				pt[2].x = pRectPic->left + x;
				pt[2].y = pRectPic->top + y - WIZARD_SYMBOL_DIAMETER/2;

				Polygon(hdcOsbBuffer, pt, 3);
			}
			else		// Cornerback or safety
				Ellipse(hdcOsbBuffer, 
					pRectPic->left + x - WIZARD_SYMBOL_DIAMETER/2, 
					pRectPic->top + y - WIZARD_SYMBOL_DIAMETER/2, 
					pRectPic->left + x + WIZARD_SYMBOL_DIAMETER/2, 
					pRectPic->top + y + WIZARD_SYMBOL_DIAMETER/2);
		}

		SelectObject(hdcOsbBuffer, hOldBrush);
		SelectObject(hdcOsbBuffer, hOldPen);
	}

	DeleteObject(hBrushBlue);
	DeleteObject(hBrushDarkBlue);
	DeleteObject(hBrushYellow);
	DeleteObject(hBrushRed);
	DeleteObject(hBrushDarkRed);

	DeleteObject(hPenBlue);
	DeleteObject(hPenDarkBlue);
	DeleteObject(hPenYellow);
	DeleteObject(hPenRed);
	DeleteObject(hPenDarkRed);

	// Draw title (centered)
	if (PlayTypeflag < WIZARD_MIDDLE_PLAY)
	{
		vm_clip = FALSE;
		vm_fontselect((void *) SmallTeamFont);

		if (bOffPlay)
		{
			vm_fgcolor = 2;
			vm_hstr(str, pRectPic->left + ((width - vm_hstr_width(str) + 1) >> 1), pRectPic->top + 2);
			vm_fgcolor = 3;
			vm_hstr(str, pRectPic->left + ((width - vm_hstr_width(str) + 1) >> 1), pRectPic->top + 1);
		}
		else
		{
			vm_fgcolor = 2;																	// Don't know font_height
			vm_hstr(str, pRectPic->left + ((width - vm_hstr_width(str) + 1) >> 1), pRectPic->bottom - 15 - 1);
			vm_fgcolor = 3;
			vm_hstr(str, pRectPic->left + ((width - vm_hstr_width(str) + 1) >> 1), pRectPic->bottom - 15 - 2);
		}

		if (Super)
		{
			vm_fgcolor = 2;
			vm_hstr(PlayName, pRectPic->left + ((width - vm_hstr_width(PlayName) + 1) >> 1), pRectPic->bottom - 10);
			vm_fgcolor = 3;
			vm_hstr(PlayName, pRectPic->left + ((width - vm_hstr_width(PlayName) + 1) >> 1), pRectPic->bottom - 11);
			str = GetCatStr((short) (SpecialFlag << 8) + PlayCategory);
			vm_fgcolor = 2;
			vm_hstr(str, pRectPic->left + ((width - vm_hstr_width(str) + 1) >> 1), pRectPic->top + 12);
			vm_fgcolor = (offenseFlag) ? 1 : 14;
			vm_hstr(str, pRectPic->left + ((width - vm_hstr_width(str) + 1) >> 1), pRectPic->top + 11);
		}
	}

	PlayPart = playPartSave;
}


// ******************************************************************
// call after loading
// ******************************************************************

void D_PLAY::InitTeam()
{
	EditedFlag = FALSE;
	CurrPlayer = -1;
	PlayerMoving = -1;
	GetPlayers();
}


// ******************************************************************
// ******************************************************************

void D_PLAY::SaveTeamData(OutputFileHandler &OutFile)
{
	OutFile.Write(&EditedFlag, sizeof(short));
	OutFile.Write(&DirType, sizeof(short));
	OutFile.Write(&SpecialFlag, sizeof(char));
	OutFile.Write(&PlayCategory, sizeof(char));
	OutFile.Write(&UserCategory, sizeof(char));
	OutFile.Write(PlayFile, MAXPATH + 1);
	OutFile.Write(PlayName, 9);
}


// ******************************************************************
// ******************************************************************

char *D_PLAY::GetTeamData(char *data)
{
	char *d = data;
	EditedFlag = *((short *)d);
	DirType = *((short *)d + 1);
	SpecialFlag = *(d + 4);
	PlayCategory = *(d + 5);
	UserCategory = *(d + 6);
	strcpy(PlayFile, d + 7);
	d += 7 + MAXPATH + 1;
	strcpy(PlayName, d);
	return (d + 9);
}


// ******************************************************************
// ******************************************************************

short D_PLAY::GetReceiverColor(short num)
{ 
	short player, rec, color;
	color = (PlayCategory & 1) ? 6 : 14;

	// Pass route colors for multiple receivers
	static short colorTable[] =
	{
		15, 3, 7, 10, 2
	};

	short qb = 0;
	D_POSITION pos;
	D_COMMAND *com;

	// Find QB or kicker (in case of fake punt/kick)
	for (player = 0; player < 11; player++)
	{
		pos = PlayerDepth[player].Pos;

		if ((pos == QUARTERBACK) || (!qb && (pos == PUNTER)))
		{
			qb = player + 1;
			break; // DJM-Should this break out here? What if QB's after P?
		}
	}

	// Get running route color
	if (qb && !(PlayerDepth[num].Pos & P_POS_OL) &&
			(PlayPart == MIDDLE_OF_PLAY) &&
			(com = getCom(qb - 1, COM::HANDOFF_TO, MIDDLE_OF_PLAY)))
	{
		if ((com + 1)->data1 == num)
			return (1);
	}

	// Get pass route colors for checking receivers
	if (qb && !(PlayerDepth[num].Pos & P_POS_OL) &&
			(PlayPart == MIDDLE_OF_PLAY) &&
			(com = getCom(qb - 1, COM::CHECK_NUM, MIDDLE_OF_PLAY)))
	{
		for (rec = 0; rec < com->data1; rec++)
		{
			if ((com + rec + 1)->data1 == num)
				return colorTable[rec];
		}
	}

	return color;
}


// ******************************************************************
// ******************************************************************

void D_PLAY::EditToDiagram(short *x,
						   short *y)
{
	*x = AddMagnitude((long) *x * 13, 28) / 56 + 3;
	*y = AddMagnitude((*y - LINEOFSCRIM) * 3, 3) / 14 + 100;
}


void D_PLAY::WizardEditToDiagram1(short *x,
								  short *y)
{
	*x = AddMagnitude((long) *x * 13, 28) / 56 + 3;
	*y = AddMagnitude((*y - LINEOFSCRIM) * 3, 3) / 9 + 75;
}

void D_PLAY::WizardEditToDiagram2(short *x,
								  short *y)
{
	*x = AddMagnitude((long) *x * 13, 28) / 40 + 3;
	*y = AddMagnitude((*y - LINEOFSCRIM) * 3, 3) / 5 + 65;
}

void D_PLAY::WizardEditToDiagram(short PlayTypeflag,
								 short width,
								 short height,
								 short *px,
								 short *py)
{
	switch (PlayTypeflag)
	{
		case WIZARD_FORMATION:
		case WIZARD_PRE_SNAP_LOGIC:
		case WIZARD_MIDDLE_PLAY_OFFENSIVE_LINE_ONLY:
		case WIZARD_MIDDLE_PLAY_HANDOFF_OFFENSIVE_LINE_ONLY:	// Used for step 1 and 2 of the Play Wizard and some other times as well
			WizardEditToDiagram1(px, py);
			*px -= (ShapeTable[S_MINI_PLAY].width-width)/2;		// This is because we have a narrower and shorter display box
			*py -= (ShapeTable[S_MINI_PLAY].height-height)/2;
			break;

		case WIZARD_MIDDLE_PLAY:				// For drawing the wizard mini-plays
		case WIZARD_MIDDLE_PLAY_RECEIVERS:
		case WIZARD_MIDDLE_PLAY_QUARTERBACK:
		case WIZARD_MIDDLE_PLAY_OFFENSIVE_LINE:
		case WIZARD_MIDDLE_PLAY_DEFENSIVE_LINE:
			WizardEditToDiagram2(px, py);
			break;
	}
}

// ******************************************************************
// ******************************************************************

short D_PLAY::Categorize(short playCat)
{
	D_PLAYCAT PlayCat;
	D_PLAYCAT::D_CAT cat;

	if (playCat == -1)
	{
		cat = PlayCat.Get(this);
		playCat = *(short *) &cat;
	}

	SpecialFlag = (uchar)(playCat >> 8);
	UserCategory = PlayCategory = (uchar)(playCat & 0xff);
	return playCat;
}


// ******************************************************************
// ******************************************************************

char *D_PLAY::GetCatStr(short playCat)	// playCat defaults to -1
{
	CatStr[0] = 0;
	D_PLAYCAT::D_CAT cat;

	cat.special = 0;

	if (playCat != -1)
	{
		*(short *) &cat = playCat;
		cat.special = playCat >> 8;		// Compiler bug....
	}
	else
	{
		cat.special = SpecialFlag;
		*(char *) &cat.b = UserCategory;

		if ((UserCategory == 0xff) || (UserCategory == 0xfe))
		{
			strcpy(CatStr, "USER SPECIFIC");
			return CatStr;
		}
	}

	if (!cat.special)
	{
		if (cat.b.dist == D_PLAYCAT::GOAL)
		{
			strcpy(CatStr, "GOAL LINE ");

			if (cat.b.pass)
				strcat(CatStr, "PASS");
			else
				strcat(CatStr, "RUN");

			return CatStr;
		}
		else if (cat.b.side == D_PLAYCAT::S_RAZZLE)
		{
			strcpy(CatStr, "RAZZLE DAZZLE ");

			if (cat.b.pass)
				strcat(CatStr, "PASS");
			else
				strcat(CatStr, "RUN");

			return CatStr;
		}

		if (cat.b.pass)
			strcpy(CatStr, "PASS");
		else
			strcpy(CatStr, "RUN");

		if (cat.b.pass)
			switch (cat.b.dist)
			{
				case D_PLAYCAT::SHORT:
					strcat(CatStr, " SHORT");
					break;

				case D_PLAYCAT::MEDIUM:
					strcat(CatStr, " MEDIUM");
					break;

				case D_PLAYCAT::LONG:
					strcat(CatStr, " LONG");
					break;
			}

		if (cat.b.off || !cat.b.pass)
			switch (cat.b.side)
			{
				case D_PLAYCAT::S_RIGHT:
					strcat(CatStr, " RIGHT");
					break;

				case D_PLAYCAT::S_LEFT:
					strcat(CatStr, " LEFT");
					break;

				case D_PLAYCAT::S_MIDDLE:
					strcat(CatStr, " MIDDLE");
					break;
			}
	}
	else
	{
		switch (cat.special)
		{
			case 1:
				strcpy(CatStr, "FIELD GOAL/PAT");

				if (!cat.b.off)
					strcat(CatStr, " DEF");

				break;

			case 2:
				if (cat.b.off)
					strcpy(CatStr, "KICK OFF");
				else
					strcpy(CatStr, "KICK RETURN");

				break;

			case 3:
				if (cat.b.off)
					strcpy(CatStr, "PUNT");
				else
					strcpy(CatStr, "PUNT RETURN");

				break;

			case 4:
				if (cat.b.off)
					strcpy(CatStr, "ONSIDE KICK");
				else
					strcpy(CatStr, "ONSIDE RETURN");

				break;

			case 5:
				strcpy(CatStr, "FAKE FIELD GOAL RUN");
				break;

			case 6:
				strcpy(CatStr, "FAKE FIELD GOAL PASS");
				break;

			case 7:
				strcpy(CatStr, "FAKE PUNT RUN");
				break;

			case 8:
				strcpy(CatStr, "FAKE PUNT PASS");
				break;

			case 9:
				if (cat.b.off)
					strcpy(CatStr, "FREE KICK");
				else
					strcpy(CatStr, "FREE KICK RETURN");

				break;

			case 10:
				if (cat.b.off)
					strcpy(CatStr, "SQUIB KICK");
				else
					strcpy(CatStr, "SQUIB RETURN");

				break;

			case 11:
				strcpy(CatStr, "RUN CLOCK");
				break;

			case 12:
				strcpy(CatStr, "STOP CLOCK");
				break;

			case 13:
				strcpy(CatStr, "RUN RANDOM");
				break;

			case 14:
				strcpy(CatStr, "PASS LONG RANDOM");
				break;

			case 15:
				strcpy(CatStr, "PASS MEDIUM RANDOM");
				break;

			case 16:
				strcpy(CatStr, "PASS SHORT RANDOM");
				break;
		}
	}

	return CatStr;
}


// ******************************************************************
// ******************************************************************

uchar D_PLAY::UserCat(uchar *val)
{
	if (val)
	{
		UserCategory = *val;

		if (Super)
			PlayCategory = UserCategory;
	}

	return UserCategory;
}


// ******************************************************************
// ******************************************************************

short D_PLAY::FindMan2Man(D_MAN_TO_MAN *manToManList)
{
	short player, numFound = 0;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		D_COMMAND *com =
				getCom(player, COM::PASS_DEFENSE, MIDDLE_OF_PLAY);

		if (com && (com->data1 == 2))
		{
			D_SUB_STR *subStr = PlayerPtr[player]->GetSubStr();
			D_MAN_TO_MAN tmpM2M;

			strcpy(tmpM2M.Pos, subStr->Pos);
			strcpy(tmpM2M.Num, subStr->Num);

			tmpM2M.Coverage = (com->data2 >> 6) & 0x03;
			tmpM2M.Rating = (com->data2 >> 3) & 0x07;
			tmpM2M.Rank = com->data2 & 0x07;

			for (short i = 0; i < numFound; i++)
			{
				if ((tmpM2M.Coverage < manToManList[i].Coverage) ||
						((tmpM2M.Coverage == manToManList[i].Coverage) &&
						((tmpM2M.Rating < manToManList[i].Rating) ||
						((tmpM2M.Rating == manToManList[i].Rating) &&
						(tmpM2M.Rank < manToManList[i].Rank)))))
				{
					for (short j = numFound; j > i; j--)
					{
						memcpy(&manToManList[j], &manToManList[j - 1],
								sizeof(D_MAN_TO_MAN));
					}

					break;
				}
			}
		
			memcpy(&manToManList[i], &tmpM2M, sizeof(D_MAN_TO_MAN));

			numFound++;
		}
	}

	return (numFound);
}


// D_PLAY private member functions-----------------------------------

// ******************************************************************
// ******************************************************************

short D_PLAY::ComSize(short comType)
{
	short res = -1;

	switch (comType)
	{
		case COM::ZONE :
		case COM::PASS :
		case COM::BLOCK_MOVE_TO :
		case COM::MOVE_TO :
		case COM::MOTION_STOP :
		case COM::MOTION_REVERSE :
		case COM::HANDOFF_TO :
		case COM::FAKE_HANDOFF_TO :
		case COM::FAKE_PASS_TO : 
			res = 2; 
			break;

		case COM::SNAP_TO :
		case COM::CHECK_NUM :
		case COM::TIMED_PASS :
		case COM::THROW_PASS_AS :
		case COM::RUN :
		case COM::PASS_DEFENSE :
		case COM::SHADE_COVERAGE :
		case COM::SHIFT_TO :
		case COM::RUN_PURSUIT_IS :
		case COM::PURSUIT_IS :
		case COM::PASS_COVER_IS :
		case COM::RUN_DEFENSE :
		case COM::RUN_RUSH :
		case COM::KEY_ON :
		case COM::BLOCK :
		case COM::WAIT_FOR :
		case COM::PLAYER : 
			res = 1; 
			break;

		default : 
			res = 0; 
			break;
	}

	return res;
}


// ******************************************************************
// ******************************************************************

long D_PLAY::AddMagnitude(long num, short a)
{
	num += (num < 0) ? -a : a;
	return num;
}


// ******************************************************************
// ******************************************************************

void D_PLAY::EditToField(short *x, short *y)
{
	*x = AddMagnitude((long)*x * 4799, 313) / 616 - 2400;
	*y = AddMagnitude((long)(LINEOFSCRIM - *y) * 90*1, YARDH/2) / YARDH;
}											// *70/96 is the scale factor


// ******************************************************************
// ******************************************************************

void D_PLAY::FieldToEdit(short *x, short *y)
{
	*x = AddMagnitude(((long)*x + 2400) * 616, 2399) / 4799;
	*y = LINEOFSCRIM - AddMagnitude((long)*y*YARDH*1, 45) / 90;
}


// ******************************************************************
// ******************************************************************

void D_PLAY::FlipLogicX(short far *x)
{
	*x = MIDFIELDX * 2 + 1 - *x;

	// fix symmetry
	if (*x > MIDFIELDX * 2)
		*x = MIDFIELDX * 2;
	else if (*x < 3)
		*x = 0;
}


// ******************************************************************
// ******************************************************************

short D_PLAY::PlayerGetSymbolNum(short pos)
{
	short shapeNum = S_BO;

	switch (pos & 0xfff0)
	{
		case P_POS_DL:
			shapeNum = S_BDL;
			break;

		case P_POS_LB:
			shapeNum = S_BLB;
			break;

		case P_POS_DB:
			shapeNum = S_BO;
			break;
	}

	return shapeNum;
}


// ******************************************************************
// ******************************************************************

void D_PLAY::GetPlayers()
{
	short player;
	char emptySlots = 0;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
		PlayerPtr[player] = NULL;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		PlayerPtr[player] = Roster->GetDepthPlayer(PlayerDepth[player], this);

		if (PlayerDepth[player] != PlayerPtr[player]->GetDepth())
		{
			PlayerPtr[player] = NULL;
			emptySlots = TRUE;
		}
	}

	if (!emptySlots)
		return;

	emptySlots = FALSE;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		if (PlayerPtr[player] == NULL)
		{
			PlayerPtr[player] = Roster->GetDepthPlayer(PlayerDepth[player], this);

			if (PlayerDepth[player].Pos == (PlayerPtr[player]->GetDepth()).Pos)
				PlayerDepth[player].Rank = PlayerPtr[player]->GetDepth().Rank;
			else
			{
				PlayerPtr[player] = NULL;
				emptySlots = TRUE;
			}
		}
	}

	if (!emptySlots)
		return;

	emptySlots = FALSE;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		if (PlayerPtr[player] == NULL)
		{
			PlayerPtr[player] = Roster->GetDepthPlayer(PlayerDepth[player], this);

			if ((PlayerDepth[player].Pos & 0xfff0) == ((PlayerPtr[player]->GetDepth()).Pos & 0xfff0))
				PlayerDepth[player] = PlayerPtr[player]->GetDepth();
			else
			{
				PlayerPtr[player] = NULL;
				emptySlots = TRUE;
			}
		}
	}

	if (!emptySlots)
		return;

	emptySlots = FALSE;

	for (player = 0; player < NUM_PLAY_PLAYERS; player++)
	{
		if (PlayerPtr[player] == NULL)
		{
			PlayerPtr[player] = Roster->GetDepthPlayer(PlayerDepth[player], this);
			PlayerDepth[player] = PlayerPtr[player]->GetDepth();
		}
	}
}


// ******************************************************************
// Player_Select before calling
// ******************************************************************

void D_PLAY::DrawPlayer()
{
	short shapeNum, i;
	short boxX, boxY, boxWidth, boxHeight;
	char jerseyNumStr[3], PosStr[7];

	switch (ViewPlayersBy)
	{
		case V_POSITION:
			// Depth chart position to be precise.
			vm_fontselect((void *)MediumTeamFont);
			vm_fgcolor = (this == OffPlay) ? 6 : 13;

			if (PlayerLogic[PlayerNum][PlayPart] == WhiteLogicBox)
				vm_fgcolor = 15;

			if (PlayerNum == PlayerMoving)
				vm_fgcolor = 3;

			strcpy(PosStr, ::PlayerPtr->GetCustPosStr());
			boxWidth = vm_hstr_width(PosStr);
			vm_hstr(PosStr, PlayerX - (boxWidth >> 1), PlayerY - FieldPos + FieldOffY - 8);

			boxWidth += 4; 
			boxHeight = 13;
			boxX = boxWidth >> 1;
			boxY = 7;
			break;

		case V_NUMBER:
			// This is so you can recognize them on the field.
			vm_fontselect((void *)MediumTeamFont);
			vm_fgcolor = (this == OffPlay) ? 6 : 13;

			if (PlayerLogic[PlayerNum][PlayPart] == WhiteLogicBox)
				vm_fgcolor = 15;

			if (PlayerNum == PlayerMoving)
				vm_fgcolor = 3;

			i = ::PlayerPtr->GetJersey();
			jerseyNumStr[0] = (i / 10) + '0';
			jerseyNumStr[1] = (i % 10) + '0';
			jerseyNumStr[2] = 0;
			boxWidth = vm_hstr_width(jerseyNumStr);
			vm_hstr(jerseyNumStr, PlayerX - (boxWidth >> 1),
					PlayerY - FieldPos + FieldOffY - 8);
			boxWidth += 2; boxHeight = 13;
			boxX = (boxWidth >> 1) + 1;
			boxY = 7;
			break;

		case V_SYMBOL :
			// I don't think I ever use this view for anything personally.
			// It's kinda pointless except that it looks similar to the mini-diagram.
			D_POSITION pos = PlayerDepth[PlayerNum].Pos;
			shapeNum = PlayerGetSymbolNum(pos);

			if ((this == OffPlay) && (PlayerNum == PreSnapPlayer) && !SpecialFlag)
				shapeNum = S_BC;  // The guy snapping the ball is very special.

			boxX = 9;
			boxY = 8;
			boxWidth = 18;
			boxHeight = (shapeNum == S_BO) ? 16 : 15;

			if (PlayerNum == PlayerMoving)  // Make 'im yella
				shapeNum += S_YLB - S_BLB;
			else if (PlayerLogic[PlayerNum][PlayPart] == WhiteLogicBox) // The chosen one!
				shapeNum += S_WLB - S_BLB;
			else if (this == DefPlay) // The devil's color.
				shapeNum += S_RLB - S_BLB;

			short flipFlag = 0;

			if (this == OffPlay)
				flipFlag = (shapeNum == S_WLB || shapeNum == S_BLB || shapeNum == S_YLB);

			if ((PlayerY - 6 - FieldPos + FieldOffY > 0) &&		// Don't want to draw in an illegal area
					(PlayerY - 6 - FieldPos + FieldOffY + ShapeTable[shapeNum].height < GRAPH_YRES))
				vm_flip_draw(&ShapeTable[shapeNum], PlayerX - 7, PlayerY - 6 - FieldPos + FieldOffY, flipFlag);
			break;
	}

	vm_fill = FALSE;

#if 0 //#ifdef DEBUG

	if ((PlayerNum == PreSnapPlayer) || 
			(PlayerNum == AfterSnapPlayer) ||
			(PlayerNum == KickingPlayer))
	{
		if (PlayerNum == PreSnapPlayer)
		{
			vm_lcolor = 13;
			vm_rect(PlayerX - boxX, 
					PlayerY - boxY - FieldPos + FieldOffY, 
					boxWidth, boxHeight);
		}

		if (PlayerNum == AfterSnapPlayer)
		{
			vm_lcolor = 6;
			vm_rect(PlayerX - boxX + 1, 
					PlayerY - boxY - FieldPos + FieldOffY + 1, 
					boxWidth - 2, boxHeight - 2);
		}

		if (PlayerNum == KickingPlayer)
		{
			vm_lcolor = 15;
			vm_rect(PlayerX - boxX + 2, 
					PlayerY - boxY - FieldPos + FieldOffY + 2, 
					boxWidth - 4, boxHeight - 4);
		}
	}

#else // #ifdef DEBUG
	if ((this == DefPlay) && (PlayerNum == PreSnapPlayer))
	{
		vm_lcolor = 13;
		vm_rect(PlayerX - boxX, 
				PlayerY - boxY - FieldPos + FieldOffY, boxWidth, 
				boxHeight);
	}

#endif // #ifdef DEBUG

	vm_fill = TRUE;
}


// ******************************************************************
// ******************************************************************

short D_PLAY::PlayerBump(short num, short x, short y)
{
	short i;
	long dx, dy;
	D_LOGIC_BOX *lb;

	// Check to see if he is invading anybody's space
	for (i = 0; i < PlayerCount; i++)
	{
		if (i != num)
		{
			lb = PlayerLogic[i][PlayPart];
			dx = (long)(lb->x - x);
			dx = (dx << 1) + dx; // *3 for x-aspect
			dy = (long)(lb->y - y) << 2; // *4 for y-aspect

			if (dx * dx + dy * dy < 3600)
				return (TRUE);
		}
	}

	return (FALSE);
}


// ******************************************************************
// ******************************************************************

void D_PLAY::CheckPrnLimit(short val)
{
	if (val > PrnMaxY)
		PrnMaxY = val;

	if (val < PrnMinY)
		PrnMinY = val;
}


// ******************************************************************
// ******************************************************************

short D_PLAY::IDToRank(short pos)
{
	short rank;

	switch (pos)
	{
		case OFF_GUARD:
			rank = ++numPos[0];
			break;

		case OFF_TACKLE:
			rank = ++numPos[1];
			break;

		case OFF_CENTER:
			rank = ++numPos[2];
			break;

		case QUARTERBACK:
			rank = ++numPos[3];
			break;

		case RUNNING_BACK:
			rank = ++numPos[4];
			break;

		case FULLBACK:
			rank = ++numPos[5];
			break;

		case HALFBACK:
			rank = ++numPos[6];
			break;

		case WIDE_RECEIVER:
			rank = ++numPos[7];
			break;

		case TIGHT_END:
			rank = ++numPos[8];
			break;

		case DEF_GUARD:
			rank = ++numPos[9];
			break;

		case DEF_TACKLE:
			rank = ++numPos[10];
			break;

		case DEF_END:
			rank = ++numPos[11];
			break;

		case LINEBACKER:
			rank = ++numPos[12];
			break;

		case CORNERBACK:
			rank = ++numPos[13];
			break;

		case SAFETY:
			rank = ++numPos[14];
			break;

		case KICKER:
			rank = ++numPos[15];
			break;

		case PUNTER:
			rank = ++numPos[16];
			break;
	}

	return rank;
}


// PLAY.CPP functions------------------------------------------------

// ******************************************************************
// returns team
// ******************************************************************

D_LOGIC_BOX *LogicTeamFind(D_PLAY **team, short x, short y)
{
	D_PLAY *t;
	D_LOGIC_BOX *lb;
	lb = (t = LastTeamAccess)->LogicFind(x, y);

	if (!lb && !ShowOneTeamFlag)
		lb = (t = t->OtherTeam())->LogicFind(x, y);

	if (lb)
		LastTeamAccess = *team = t;

	return lb;
}


// ******************************************************************
// ******************************************************************

void InitTeams()
{
	char LeagueFileName[MAXPATH + 1];

	if (!putBuf)
	{
		putBuf = (char far *)vm_hmalloc(vm_getsize(36, 16), 0);
		sys_assert(putBuf, "Not enough memory for buffer.");
	}

	CatDirFileExtension(LeagueFileName, LeaguePath, LeagueFile, "LGE");
	InputFileHandler LeagueInFile(LeagueFileName);
	Roster = new D_ROSTER;
	Roster->Load(LeagueInFile, TeamId);
}


// ******************************************************************
// ******************************************************************

void FreeTeams()
{
	if (Roster)
		delete Roster;

	OffPlay->Erase();
	DefPlay->Erase();

	if (putBuf)
		vm_hfree(putBuf);
}


// ******************************************************************
// ******************************************************************

void DrawTeams()
{
	if (!ShowOneTeamFlag)
	{
		(LastTeamAccess->OtherTeam())->Draw();
		LastTeamAccess->Draw();
	}
	else
		((ShowOneTeamFlag == 1) ? OffPlay : DefPlay)->Draw();
}


// ******************************************************************
// ******************************************************************

void DrawMiniPlayStock(char *fileName,
					   RECT *pRectPic)
{
	D_PLAY *miniPlay = new D_PLAY;
	short playSize;
	InputFileHandler InPlayFile;

	if (InPlayFile.Open(StockPlays->StockDatFile))
	{
		long playOff = StockPlays->GetStockOffset(fileName,	&playSize);

		if (playOff != -1)
		{
			InPlayFile.JumpTo(playOff);
			miniPlay->Load(InPlayFile, playSize, FALSE);
			InPlayFile.Close();
			if ((FileReqType != FR_PLAN_EDIT) ||
					(((miniPlay->Special() ? 2 : 0) ==
					(TeamDirType & 0x02)) &&
					((miniPlay->PlayCat() & 1) ==
					!(TeamDirType & 0x01))))
				miniPlay->DrawMiniPlayMem(pRectPic, TRUE);

			miniPlay->Erase();
		}
	}

	delete miniPlay;
}


// ******************************************************************
// ******************************************************************

void DrawMiniPlayDisk(char *fileName,
					  RECT *pRectPic)
{
	D_PLAY *miniPlay = new D_PLAY;
	InputFileHandler InPlayFile;

	if (InPlayFile.Open(fileName))
	{
		if (InPlayFile.FindFirstBlock(PlayBlockStr))
		{
			miniPlay->Load(InPlayFile, (short) InPlayFile.BlockSize, FALSE);

			if ((FileReqType != FR_PLAN_EDIT) ||
					(((miniPlay->Special() ? 2 : 0) == (TeamDirType & 0x02)) &&
					((miniPlay->PlayCat() & 1) == !(TeamDirType & 0x01))))
				miniPlay->DrawMiniPlayMem(pRectPic, TRUE);

			miniPlay->Erase();
		}

		InPlayFile.Close();
	}

	delete miniPlay;
}


// ******************************************************************
// ******************************************************************

void DrawMiniPlay(PlayInPlan *playPtr)
{
	char stockName[9];

	if (playPtr->Info.StockFlag)
	{
		stockName[0] = 0;
		strncat(stockName, playPtr->Stock.PlayName, 8);
		DrawMiniPlayStock(stockName, NULL);
	}
	else
		DrawMiniPlayDisk(playPtr->Cust.PlayName, NULL);
}


// ******************************************************************
// ******************************************************************

void ClearMiniPlay(BOOL bColor)
{
	if (bColor)
		vm_flip_draw(&ShapeTable[S_MINI_PLAY], CurBox.x, CurBox.y, 0);
	else
		vm_flip_draw(&ShapeTable2[0], CurBox.x, CurBox.y, 0);
}


// ******************************************************************
// Call with play file immediately after finding OldPlayBlockStr block.
// ******************************************************************

void UpdatePlay(InputFileHandler &inPlayFile)
{
	char playFile[MAXPATH + 1];
	OutputFileHandler outPlayFile;
	D_PLAY tempPlay;

	// Read old play into memory
	tempPlay.OldLoad(inPlayFile, (short) inPlayFile.BlockSize);
	inPlayFile.GetFullPath(playFile);
	inPlayFile.Close();

	// Update play data and categorize it
	tempPlay.InitTeam();
	tempPlay.Categorize();

	// Save new play
	outPlayFile.NewFile(playFile);
	outPlayFile.StartBlock(PlayBlockStr);
	tempPlay.Save(outPlayFile);
	outPlayFile.EndBlock();
	outPlayFile.Close();

	// Open new play file and return
	inPlayFile.Open(playFile);
	inPlayFile.FindFirstBlock(PlayBlockStr);
}


// ******************************************************************
// ******************************************************************

short D_StockLoad(long StockOffset, short StockSize, D_PLAY *currPlay)
{
	InputFileHandler InFile;

	// Open stock play volume
	if (!InFile.Open(StockPlays->StockDatFile))
	{
		D_OKBox("Can't find STOCK.DAT");
		return (FALSE);
	}

	// Go to play offset and load
	InFile.JumpTo(StockOffset);
	currPlay->Load(InFile, StockSize, TRUE);
	currPlay->InitTeam();

	// Set LastTeamAccess to desired play
	LastTeamAccess = currPlay;

	if (ShowOneTeamFlag)
		LastTeamAccess = (ShowOneTeamFlag == 1) ? OffPlay : DefPlay;

	InFile.Close();
	return (TRUE);
}


// ******************************************************************
// ******************************************************************

short D_PlayLoad(char *fileStr, D_PLAY *currPlay)
{
	InputFileHandler InFile;

	if (!InFile.Open(fileStr))
	{
		D_OKBox(BadFileStr);
		return (FALSE);
	}

	if (InFile.FindFirstBlock(PlayBlockStr))
		currPlay->Load(InFile, (short) InFile.BlockSize, TRUE);
	else
	{
		InFile.Close();
		D_OKBox(BadFileStr);
		return (FALSE);
	}

	currPlay->InitTeam();
	LastTeamAccess = currPlay;

	if (ShowOneTeamFlag)
		LastTeamAccess = (ShowOneTeamFlag == 1) ? OffPlay : DefPlay;

	InFile.GetFullPath(currPlay->File());
	InFile.GetFile(currPlay->Name());
	InFile.Close();
	return (TRUE);
}

// JDW - 96-08-19 
void DrawPlayBox (int xBox, int yBox, char *szTop, char *szBottom)
{
	SIZE	sSizeStr;

	Rectangle(Printer->GetPrinterDC(), xBox, yBox, xBox+Printer->hPlayList, yBox+Printer->vPlayList); 

	Printer->LoadAltFont("Arial", 120, 0, FONT_NORMAL, FALSE);
	Printer->SetAltFont();
	GetTextExtentPoint32(Printer->GetPrinterDC(), szTop, strlen(szTop), &sSizeStr);
	TextOut(Printer->GetPrinterDC(), xBox + ((Printer->hPlayList - sSizeStr.cx)>>1), yBox + (Printer->vPlayList >> 1) - sSizeStr.cy, (PCHAR) szTop, strlen((PCHAR) szTop));

	Printer->LoadAltFont("Arial", 90, 0, FONT_NORMAL, FALSE);
	Printer->SetAltFont();
	GetTextExtentPoint32(Printer->GetPrinterDC(), szBottom, strlen(szBottom), &sSizeStr);
	TextOut(Printer->GetPrinterDC(), xBox + ((Printer->hPlayList - sSizeStr.cx)>>1), yBox + (Printer->vPlayList >> 1) + (((Printer->vPlayList >> 1) - sSizeStr.cy) >> 1), (PCHAR) szBottom, strlen((PCHAR) szBottom));

	Printer->SetStandardFont();

}

