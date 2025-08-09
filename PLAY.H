/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
  PARTICULAR PURPOSE.

  Copyright (C) 1993-1996  Sierra On-Line.  All Rights Reserved.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
// PLAY.H
// Header file for PLAY.CPP and D_PLAY class definition


#ifndef PLAY_H
#define PLAY_H


// D_PLAY class definition---------------------------------------


typedef enum D_PLAY_PARTS
{
	PRE_SNAP = 0,
	MIDDLE_OF_PLAY,
	END_OF_PLAY
};


enum D_FILE_MODS
{
	F_STOCK_OFF, F_STOCK_DEF, F_SPEC_TEAM, F_CUSTOM,
	F_EXHIB_OFF, F_EXHIB_DEF, F_EXHIB_SPEC_TEAM
};


union PlayInPlan;


class far D_PLAY
{
public:
	enum
	{
		NUM_PLAY_PLAYERS = 11
	};

	D_PLAY();
	~D_PLAY();
	D_PLAY(const D_PLAY *play);
	void Initialize(BOOL bOffPlay);
	void far *operator new(size_t Size);
	void operator delete(void far *Ptr);
	short Load(InputFileHandler &InFile, short size, BOOL bScale);
	short LoadFormation(InputFileHandler &InFile, short size);
	short LoadPreSnapLogic(InputFileHandler &InFile, short size);
	short OldLoad(InputFileHandler &InFile, short size = 0);
	short Save(OutputFileHandler &OutFile);
	short SaveFormation(OutputFileHandler &OutFile);
	short SavePreSnapLogic(OutputFileHandler &OutFile);
	void Print(short Playnum = -1, short posInPlan = -1);
	void PrintPlayer();
	void EditToPrint(short *x, short *y);
	void Flip();
	void EraseLogic(short player = -1);
	void Erase();
	D_LOGIC_BOX *SaveLogic(D_LOGIC_BOX *lb = NULL);
	void RestoreLogic(short restoreFlag = TRUE);
	void PlayerSelect(short num);
	void InitTeam();
	void SaveTeamData(OutputFileHandler &OutFile);
	char *GetTeamData(char *data);
	void InitMoveTo(short num);
	short PlayerMoveable(short num);
	void PlayerMoveTo(short NewX, short NewY);
	D_PLAY *OtherTeam();
	void PlayerMoved(short num, short x = -1, short y = -1);
	short OnField(D_PLAYER *playerPtr);
	void ActivateSub(short player, D_PLAYER *sub);
	void Draw();
	void DrawMiniPlayMem(RECT *pRectPic, BOOL bColor);
	void DrawMiniWizardPlayMem(RECT *pRectPic, char *fileName, short man, short PlayTypeflag, BOOL bAdjust);
	D_LOGIC_BOX *LogicFind(short x, short y);
	short GetReceiverColor(short num);
	void EditToDiagram(short *x, short *y);
	void WizardEditToDiagram(short PlayTypeflag, short width, short height,short *x, short *y);
	void WizardEditToDiagram1(short *x, short *y);
	void WizardEditToDiagram2(short *x, short *y);
	short Categorize(short playCat = -1);
	char *GetCatStr(short playCat = -1);
	uchar UserCat(uchar *val = NULL);

	short FindMan2Man(D_MAN_TO_MAN *manToManList);

	short Loaded()
	{
		return (PlayerCount > 0);
	}
	
	short CurrentPlayer(short currPlayer = -2) 
	{
		if (currPlayer != -2)
			CurrPlayer = currPlayer;

		return CurrPlayer;
	}
	
	short HasBeenEdited(short editedFlag = -1) 
	{
		if (editedFlag != -1)
			EditedFlag = editedFlag;

		if (PlayerCount == 0)
			return FALSE;

		return EditedFlag;
	}
	
	short PreferredPlayer(short player = -2)
	{
		if (player != -2)
			PreSnapPlayer = player;

		return PreSnapPlayer;
	}
	
	short MovingPlayer(short player = -2)
	{
		if (player != -2)
			PlayerMoving = player;

		return PlayerMoving;
	}
	
	short Type()
	{
		return DirType;
	}

	uchar Special(uchar val = 0xff) 
	{
		if (val != 0xff)
			SpecialFlag = val;

		return SpecialFlag;
	}
	
	uchar PlayCat()
	{
		return PlayCategory;
	}
	
	char *File()
	{
		return PlayFile;
	}
	
	char *Name()
	{
		return PlayName;
	}
	
	D_PLAYER *GetPlayer(short Num)
	{
		return PlayerPtr[Num];
	}
	
	D_DEPTH_POS GetPosition(short Num) 
	{
		return PlayerDepth[Num];
	}
	
	D_LOGIC_BOX *GetLogic(short Player, short PlayPart) 
	{
		return PlayerLogic[Player][PlayPart];
	}

private:
	short PlayerCount;
	short CurrPlayer;
	short EditedFlag;
	short PlayerMoving;
	short DirType;
	uchar SpecialFlag;
	uchar PlayCategory;
	uchar UserCategory;
	char *PlayFile;
	char *PlayName;
	D_PLAYER *PlayerPtr[NUM_PLAY_PLAYERS];
	D_DEPTH_POS PlayerDepth[NUM_PLAY_PLAYERS];
	D_LOGIC_BOX *PlayerLogic[NUM_PLAY_PLAYERS][2];
	short PreSnapPlayer;
	short AfterSnapPlayer;
	short KickingPlayer;
	char *CatStr;
	D_LOGIC_BOX *SavedLogic[NUM_PLAY_PLAYERS][2];

	static short PrnMinY;
	static short PrnMaxY;
	void CheckPrnLimit(short val);

	D_COMMAND *getCom(short pl, short com, short playPart);
	short ComSize(short comType);
	long AddMagnitude(long num, short a);
	void EditToField(short *x, short *y);
	void FieldToEdit(short *x, short *y);
	void FlipLogicX(short far *x);
	short PlayerGetSymbolNum(short pos);
	void GetPlayers();
	void DrawPlayer();
	short PlayerBump(short num, short x, short y);
	short IDToRank(short pos);
};


// Global variables----------------------------------------------


extern D_PLAY *OffPlay, *DefPlay;
extern D_PLAY *LastTeamAccess;


// Global functions----------------------------------------------


D_LOGIC_BOX *LogicTeamFind(D_PLAY **team, short x, short y);
void InitTeams();
void FreeTeams();
void DrawTeams();

void DrawMiniPlayStock(char *fileName,
					   RECT *pRectPic);

void DrawMiniPlayDisk(char *fileName,
					   RECT *pRectPic);

void DrawMiniPlay(PlayInPlan *playPtr);
void ClearMiniPlay(BOOL bColor);
void UpdatePlay(InputFileHandler &inPlayFile);
short D_StockLoad(long StockOffset, short StockSize, D_PLAY *currPlay);
short D_PlayLoad(char *fileStr, D_PLAY *currPlay);


#endif // #ifndef PLAY_H
