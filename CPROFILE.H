/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
  PARTICULAR PURPOSE.

  Copyright (C) 1993-1996  Sierra On-Line.  All Rights Reserved.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
// CPROFILE.H
// Header file for CPROFILE.CPP and D_PROF class definition.


#ifndef CPROFILE_H
#define CPROFILE_H


struct PlaysForSit
{
	struct
	{
		uchar Num;
		uchar Weight;
	} Play[3];
};


struct PlaysForSitEdit
{
	struct
	{
		short Num;
		short Weight;
	} Play[3];

	short StopClock;
};


struct SubstitutionTable
{
	short Percentage[8][2];
};


struct PlaySituation
{
	char Time;
	char Down;
	char YTG;
	char Pos;
	char PtDif;
	short SitNum;

	short GetSit(char *time, char *down, char *ytg, char *pos, char *ptDif);
	short GetSitNum();
	short GetSitNum(char time, char down, char ytg, char pos, char ptDif);
	short SetSit(char time, char down, char ytg, char pos, char ptDif);
	short SetSitNum(short sitNum);
};


struct PATPlaySituation
{
	char Time;
	char PtDif;
	short SitNum;

	short GetSit(char *time, char *ptDif);
	short GetSitNum();
	short GetSitNum(char time, char ptDif);

	short SetSit(char time, char ptDif);
	short SetSitNum(short sitNum);
};


class far D_PROF
{
public:
	enum
	{
		NUM_SITUATIONS = 2520,
		NUM_PAT_SITS = 60,
	};

	D_PROF();
	~D_PROF();
	void far *operator new(size_t Size);
	void operator delete(void far *Ptr);
	short Load(InputFileHandler &InFile);
	short Save(OutputFileHandler &OutFile);
	void LoadInfo(InputFileHandler &InFile);
	void SaveInfo(OutputFileHandler &OutFile, short planFlag);
	void Clear();
	void Generalize(short playNum = -1);

	short Generalize(PlaySituation &sit, short playNum = -1);
	short Generalize(PATPlaySituation &sit, short playNum = -1);

	short Cycle(PlaySituation &sit, short playNum);
	short Cycle(PATPlaySituation &sit, short playNum);

	void ChangePlays(PlaysForSitEdit *plays, ushort sitNum);

	void GetPlays(PlaysForSitEdit *plays, PlaySituation &sit);
	void GetPlays(PlaysForSitEdit *plays, PATPlaySituation &sit);

	void GetSubs(SubstitutionTable *table);
	void SetSubs(SubstitutionTable *table);
	void Print(short titleFlag, BOOL printPAT, short totPages,
			short totSits, short timeFlag[], short downFlag[],
			short ytgFlag[], short posFlag[], short ptsFlag[]);

	short CopySit(short sitToCopy, short timeFlag[], short downFlag[],
			short ytgFlag[], short posFlag[], short ptsFlag[],
			short whatToCopy[]);

	void SavePlayCall();
	void RestorePlayCall(short restoreFlag = TRUE);

	void save_pat_logic();
	void restore_pat_logic(short restoreFlag = TRUE);

	void DrawInfoMem();

	
	char Active()
	{
		return ActiveFlag;
	}

	
	char Offense()
	{
		if (!ActiveFlag)
			return -1;

		return OffFlag;
	}

	
	char HasBeenEdited(char editedFlag = -1)
	{
		if (editedFlag != -1)
			EditedFlag = editedFlag;

		if (!ActiveFlag)
			return FALSE;

		return EditedFlag;
	}

	
	char FGPos(char fgRange = -1)
	{
		if (fgRange != -1)
		{
			FGRange = fgRange;
			EditedFlag = TRUE;
		}

		return FGRange;
	}

	
	short Specific()
	{
		return NumSpecific;
	}
	
	
	char *File()
	{
		return ProfFile;
	}
	
	
	char *Name()
	{
		return ProfName;
	}

	
	void set_pass(char far *_pass)
	{
		strcpy(password, _pass);
	}

	
	char far *get_pass()
	{
		return (password);
	}

private:
	SubstitutionTable Subs;
	PlaysForSit PlaysToCall[NUM_SITUATIONS];
	PlaysForSit PATLogic[NUM_PAT_SITS];
	char FGRange;
	char ActiveFlag;
	char OffFlag;
	char EditedFlag;
	short NumSpecific;

	char *ProfFile;
	char *ProfName;
	short PlanFlag;

	char password[33];

	short CountSpecific();
	short GetGeneralType(char playCat);
};


extern D_PROF *Profile;


BOOL DrawProfInfoDisk(char *fileName);
void ClearProfInfo();


#endif // #ifndef CPROFILE_H
