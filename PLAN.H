/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
  PARTICULAR PURPOSE.

  Copyright (C) 1993-1996  Sierra On-Line.  All Rights Reserved.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
// PLAN.H
// Header file for PLAN.CPP


#ifndef _PLAN_H
#define _PLAN_H


enum D_PLAN_MODS
{
	P_STOCK_OFF, P_STOCK_DEF,
	P_EXHIB_OFF, P_EXHIB_DEF,
	P_LEAGUE_OFF, P_LEAGUE_DEF
};


struct StockInPlan
{
	char StockFlag;
	uchar PlayCat;
	uchar SpecCat;
	uchar UserCat;
	char PlayName[8];
	long Offset;
	short Size;
};


struct CustInPlan
{
	char StockFlag;
	uchar PlayCat;
	uchar SpecCat;
	uchar UserCat;
	char PlayName[MAXPATH + 1];
};


union PlayInPlan
{
	CustInPlan Info;
	StockInPlan Stock;
	CustInPlan Cust;

	void GetPlayName(char *playName);
};


class far D_PLAN
{
public:
	D_PLAN();
	~D_PLAN();
	void far *operator new(size_t Size);
	void operator delete(void far *Ptr);
	void Clear();
	void InitSpec();
	short Load(InputFileHandler &inPlanFile);
	short OldLoad(InputFileHandler &inPlanFile,
				short halfFlag = -1);
	short Save(OutputFileHandler &outPlanFile);
	void LoadInfo(InputFileHandler &inPlanFile);
	void SaveInfo(OutputFileHandler &outPlanFile);
	void NewPlan(short offFlag);
	short SlotFilled(short playNum);
	char PlayNotFound(short playNum);
	void AddPlay(PlayInPlan *play, short playNum);
	short GetPlay(PlayInPlan *play, short playNum);
	void RemovePlay(short playNum);
	void Print(short titleFlag = FALSE);
	void PlayNumToRowColStr(short playNum, char *rowColStr);
	void DrawInfoMem();

#if 0		// This has been dropped from the game
	void SetAudibles(char audibles[])
	{
		memcpy(Audible, audibles, sizeof(Audible));
		EditedFlag = TRUE;
	}

	void GetAudibles(char audibles[])
	{
		if (audibles)
			memcpy(audibles, Audible, sizeof(Audible));
	}
#endif

	char Active(char active = -1)
	{
		if (active != -1)
			PlanActive = active;

		return PlanActive;
	}

	char HasBeenEdited()
	{
		if (!PlanActive)
			return FALSE;

		return EditedFlag;
	}

	char Offense()
	{
		if (!PlanActive)
			return -1;

		return OffFlag;
	}

	char *File()
	{
		return PlanFile;
	}
	
	char *Name()
	{
		return PlanName;
	}

	short NumPlays()
	{
		return (NumNormPlays + NumSpecPlays);
	}

	short NumRegPlays()
	{
		return NumNormPlays;
	}

	short GetNumStockPlays()
	{
		return NumStockPlays;
	}

	short GetNumCustPlays()
	{
		return NumCustPlays;
	}

	short GetNumSpecPlays()
	{
		return NumSpecPlays;
	}

	enum
	{
		NUM_NORM_PLAYS = 64, NUM_SPEC_PLAYS = 10,
		NUM_CLOCK_PLAYS = 2, TOT_NUM_PLAYS = 86
	};

private:

	PlayInPlan NormalPlays[NUM_NORM_PLAYS];
	PlayInPlan SpecialPlays[NUM_SPEC_PLAYS];
	PlayInPlan StockSpecialPlays[NUM_SPEC_PLAYS];
	PlayInPlan ClockPlays[NUM_CLOCK_PLAYS];
	char Audible[4];
	char PlanType;
	char PlanActive;
	char OffFlag;
	char EditedFlag;
	short NumNormPlays;
	short NumSpecPlays;
	char *PlanFile;
	char *PlanName;
	short NumStockPlays;
	short NumCustPlays;
	char NotFoundPlays[TOT_NUM_PLAYS];

	short Valid(short playNum)
	{
		return ((playNum >= 0) && (playNum < (NUM_NORM_PLAYS + NUM_SPEC_PLAYS)));
	}
};


extern D_PLAN *Gameplan;


BOOL DrawPlanInfoDisk(char *fileName);
void ClearPlanInfo();


#endif // #ifndef _PLAN_H
