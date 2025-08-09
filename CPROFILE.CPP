/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
  PARTICULAR PURPOSE.

  Copyright (C) 1993-1996  Sierra On-Line.  All Rights Reserved.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
// CPROFILE.C
// D_PROF routines.


#include "all.h"
#include "winprint.h"
#include "resource.h"
INCLUDE_ASSERT
#pragma warning (disable : 4244) // disable conversion from short to char and long to short warnings


// PlaySituation member functions------------------------------------

// ******************************************************************
// ******************************************************************

short PlaySituation::GetSit(char *time, 
							char *down, 
							char *ytg,
							char *pos, 
							char *ptDif)
{
	if (time)
		*time = Time;

	if (down)
		*down = Down;

	if (ytg)
		*ytg = YTG;

	if (pos)
		*pos = Pos;

	if (ptDif)
		*ptDif = PtDif;

	return GetSitNum();
}


// ******************************************************************
// ******************************************************************

short PlaySituation::GetSitNum()
{
	SitNum = (Time * 560) + (Down * 140) + (YTG * 35) + (Pos * 7) +
			PtDif;

	if ((SitNum % 140) > 111)
		SitNum -= 7;

	if ((SitNum % 140) > 76)
		SitNum -= 7;

	if (SitNum / 140)
		SitNum -= (SitNum / 140) * 14;

	return SitNum;
}


// ******************************************************************
// ******************************************************************

short PlaySituation::GetSitNum(char time, 
							   char down, 
							   char ytg,
							   char pos, 
							   char ptDif)
{
	short sitNum = (time * 560) + (down * 140) + (ytg * 35) +
			(pos * 7) + ptDif;

	if ((sitNum % 140) > 111)
		sitNum -= 7;

	if ((sitNum % 140) > 76)
		sitNum -= 7;

	if (sitNum / 140)
		sitNum -= (sitNum / 140) * 14;

	return sitNum;
}


// ******************************************************************
// ******************************************************************

short PlaySituation::SetSit(char time, 
							char down, 
							char ytg,
							char pos, 
							char ptDif)
{
	Time = time;
	Down = down;
	YTG = ytg;
	Pos = pos;
	PtDif = ptDif;

	return GetSitNum();
}


// ******************************************************************
// ******************************************************************

short PlaySituation::SetSitNum(short sitNum)
{
	SitNum = sitNum;

	if (SitNum > 2519)
		SitNum = 2519;

	if (SitNum < 0)
		SitNum = 0;

	if (SitNum / 126)
		SitNum += (SitNum / 126) * 14;

	if ((SitNum % 140) > 69)
		SitNum += 7;

	if ((SitNum % 140) > 104)
		SitNum += 7;

	Time = SitNum / 560;
	Down = (SitNum % 560) / 140;
	YTG = (SitNum % 140) / 35;
	Pos = (SitNum % 35) / 7;
	PtDif = SitNum % 7;

	return SitNum;
}


// PATPlaySituation member functions---------------------------------

// ******************************************************************
// ******************************************************************

short PATPlaySituation::GetSit(char *time, 
							   char *ptDif)
{
	if (time)
		*time = Time;

	if (ptDif)
		*ptDif = PtDif;

	return GetSitNum();
}


// ******************************************************************
// ******************************************************************

short PATPlaySituation::GetSitNum()
{
	SitNum = (Time * 15) + PtDif;

	return SitNum;
}


// ******************************************************************
// ******************************************************************

short PATPlaySituation::GetSitNum(char time, char ptDif)
{
	short sitNum = (time * 15) + ptDif;

	return sitNum;
}


// ******************************************************************
// ******************************************************************

short PATPlaySituation::SetSit(char time, char ptDif)
{
	Time = time;
	PtDif = ptDif;

	return GetSitNum();
}


// ******************************************************************
// ******************************************************************

short PATPlaySituation::SetSitNum(short sitNum)
{
	SitNum = sitNum;

	Time = SitNum / 15;
	PtDif = SitNum % 15;

	return SitNum;
}


// D_PROF private member functions-----------------------------------

// ******************************************************************
// ******************************************************************

short D_PROF::CountSpecific()
{
	short sitNum, playNum;
	NumSpecific = 0;

	for (sitNum=0; sitNum<NUM_SITUATIONS; sitNum++)
		for (playNum=0; playNum<3; playNum++)
			if (PlaysToCall[sitNum].Play[playNum].Num >= 128)
				NumSpecific++;

	for (sitNum=0; sitNum<NUM_PAT_SITS; sitNum++)
		for (playNum=0; playNum<3; playNum++)
			if (PATLogic[sitNum].Play[playNum].Num >= 128)
				NumSpecific++;

	return NumSpecific;
}


// ******************************************************************
// ******************************************************************

short D_PROF::GetGeneralType(char playCat)
{
	short playType = 0, offFlag = (playCat & 0x01),	passFlag = (playCat & 0x02);

	if (passFlag) // Pass
		playType = 5;

	if ((playCat & 0x30) == 0x30) // GL
		return playType;

	switch (playCat & 0x0c)
	{
		case 0x0c: // RD
			playType++;
			return playType;

		case 0x00: // Right
			if (offFlag || !passFlag)
				playType += 4;

			break;

		case 0x04: // Left
			if (offFlag || !passFlag)
				playType += 2;

			break;

		case 0x08: // Middle
			if (offFlag || !passFlag)
				playType += 3;

			break;
	}

	if (!passFlag)
		return playType;

	if (!offFlag)
		playType += 2;

	switch (playCat & 0x30)
	{
		case 0x00: // Short
			playType += 6;
			break;

		case 0x10: // Medium
			playType += 3;
			break;
	}

	return playType;
}


// D_PROF public member functions--------------------------------

// ******************************************************************
// ******************************************************************

D_PROF::D_PROF()
{
	Clear();
	ProfFile = new char[MAXPATH + 1];
	ProfName = new char[13];
	memset(password, 0, sizeof(password));
}


// ******************************************************************
// ******************************************************************

D_PROF::~D_PROF()
{
	if (ProfFile)
		delete []ProfFile;

	if (ProfName)
		delete []ProfName;
}


// ******************************************************************
// ******************************************************************

void far *D_PROF::operator new(size_t Size)
{
	return (void far *)vm_hmalloc(Size, MEMF_CLEAR);
}


// ******************************************************************
// ******************************************************************

void D_PROF::operator delete(void far *Ptr)
{
	vm_hfree((char huge *)Ptr);
}


// ******************************************************************
// ******************************************************************

short D_PROF::Load(InputFileHandler &InFile)
{
	OffFlag = !(TeamDirType & 1);

	InFile.Read(&Subs, sizeof(Subs));
	InFile.Read(PlaysToCall, sizeof(PlaysToCall));
	InFile.Read(&FGRange, sizeof(FGRange));
	InFile.Read(PATLogic, sizeof(PATLogic));

	CountSpecific();
	ActiveFlag = TRUE;
	EditedFlag = FALSE;

	return (TRUE);
}


// ******************************************************************
// ******************************************************************

short D_PROF::Save(OutputFileHandler &OutFile)
{
	OutFile.Write(&Subs, sizeof(Subs));
	OutFile.Write(PlaysToCall, sizeof(PlaysToCall));
	OutFile.Write(&FGRange, sizeof(FGRange));
	OutFile.Write(PATLogic, sizeof(PATLogic));

	EditedFlag = FALSE;

	return (TRUE);
}


// ******************************************************************
// ******************************************************************

void D_PROF::LoadInfo(InputFileHandler &InFile)
{
	ActiveFlag = TRUE;

	InFile.Read(&OffFlag, sizeof(OffFlag));
	InFile.Read(&NumSpecific, sizeof(NumSpecific));
	InFile.Read(&FGRange, sizeof(FGRange));
	InFile.Read(&PlanFlag, sizeof(short));

	if (InFile.Read(password, sizeof(password)) == sizeof(password))
	{
		for (int16 i = 0; i < sizeof(password); i++)
		{
			password[i] ^= int8(0x69 * i);
			password[i] = (((~password[i] << 4) & 0xf0) + ((password[i] >> 4) & 0x0f));
		}
	}
	else
		memset(password, 0, sizeof(password));
}


// ******************************************************************
// ******************************************************************

void D_PROF::SaveInfo(OutputFileHandler &OutFile,
					  short planFlag)
{
	OutFile.Write(&OffFlag, sizeof(OffFlag));
	OutFile.Write(&NumSpecific, sizeof(NumSpecific));
	OutFile.Write(&FGRange, sizeof(FGRange));
	OutFile.Write(&planFlag, sizeof(planFlag));

	if (password[0])
	{
		char temp[33];

		memcpy(temp, password, sizeof(password));

		for (int16 i = 0; i < sizeof(password) - 1; i++)
		{
			password[i] = (((~password[i] >> 4) & 0x0f) + ((password[i] << 4) & 0xf0));
			password[i] ^= int8(0x69 * i);
		}

		OutFile.Write(password, sizeof(password));

		memcpy(password, temp, sizeof(password));
	}
}


// ******************************************************************
// ******************************************************************

void D_PROF::Clear()
{
	short subPos, sitNum;
	ActiveFlag = FALSE;
	EditedFlag = FALSE;
	NumSpecific = 0;
	password[0] = 0;

	for (subPos = 0; subPos < 8; subPos++)
	{
		Subs.Percentage[subPos][0] = 70;
		Subs.Percentage[subPos][1] = 85;
	}

	memset(PlaysToCall, 0, sizeof(PlaysToCall));

	for (sitNum = 0; sitNum < NUM_SITUATIONS; sitNum++)
		PlaysToCall[sitNum].Play[0].Weight = 1;

	FGRange = 35;

	memset(PATLogic, 0, sizeof(PATLogic));

	for (sitNum = 0; sitNum < NUM_PAT_SITS; sitNum++)
	{
		PATLogic[sitNum].Play[0].Num = 16;
		PATLogic[sitNum].Play[0].Weight = 1;
	}
}


// ******************************************************************
// ******************************************************************

void D_PROF::Generalize(short specNum)
{
	if (!NumSpecific)
		return;

	char playCat;
	short sitNum, playNum, chosenPlay;
	PlayInPlan play;

	for (sitNum=0; sitNum<NUM_SITUATIONS; sitNum++)
	{
		for (playNum = 0; playNum < 3; playNum++)
		{
			chosenPlay = PlaysToCall[sitNum].Play[playNum].Num;
			chosenPlay -= 128;

			if ((specNum == -1) ? (chosenPlay >= 0) : (chosenPlay == specNum))
			{
				if (Gameplan->GetPlay(&play, chosenPlay))
				{
					playCat = play.Info.UserCat;

					if ((playCat == -1) || (playCat == -2))
						playCat = play.Info.PlayCat;

					PlaysToCall[sitNum].Play[playNum].Num = GetGeneralType(playCat);
				}
				else
					PlaysToCall[sitNum].Play[playNum].Num = 0;

				EditedFlag = TRUE;
			}
		}
	}

	for (sitNum=0; sitNum<NUM_PAT_SITS; sitNum++)
	{
		for (playNum=0; playNum<3; playNum++)
		{
			chosenPlay = PATLogic[sitNum].Play[playNum].Num;
			chosenPlay -= 128;

			if ((specNum == -1) ? (chosenPlay >= 0) :
					(chosenPlay == specNum))
			{
				if (Gameplan->GetPlay(&play, chosenPlay))
				{
					playCat = play.Info.UserCat;

					if ((playCat == -1) || (playCat == -2))
						playCat = play.Info.PlayCat;

					PATLogic[sitNum].Play[playNum].Num = GetGeneralType(playCat);
				}
				else
					PATLogic[sitNum].Play[playNum].Num = 0;

				EditedFlag = TRUE;
			}
		}
	}

	CountSpecific();
}


// ******************************************************************
// ******************************************************************

short D_PROF::Generalize(PlaySituation &sit, 
						 short playNum)
{
	if (!NumSpecific)
		return FALSE;

	char playCat;
	short chosenPlay;
	PlayInPlan play;
	short edited = FALSE;

	if (playNum == -1)
	{
		for (playNum = 0; playNum < 3; playNum++)
		{
			chosenPlay = PlaysToCall[sit.SitNum].Play[playNum].Num;
			chosenPlay -= 128;

			if (chosenPlay >= 0)
			{
				if (Gameplan->GetPlay(&play, chosenPlay))
				{
					playCat = play.Info.UserCat;

					if ((playCat == -1) || (playCat == -2))
						playCat = play.Info.PlayCat;

					PlaysToCall[sit.SitNum].Play[playNum].Num =
							GetGeneralType(playCat);
				}
				else
					PlaysToCall[sit.SitNum].Play[playNum].Num = 0;

				edited = TRUE;
			}
		}
	}
	else
	{
		chosenPlay = PlaysToCall[sit.SitNum].Play[playNum].Num;
		chosenPlay -= 128;

		if (chosenPlay >= 0)
		{
			if (Gameplan->GetPlay(&play, chosenPlay))
			{
				playCat = play.Info.UserCat;

				if ((playCat == -1) || (playCat == -2))
					playCat = play.Info.PlayCat;

				PlaysToCall[sit.SitNum].Play[playNum].Num =
						GetGeneralType(playCat);
			}
			else
				PlaysToCall[sit.SitNum].Play[playNum].Num = 0;

			edited = TRUE;
		}
	}

	if (edited)
	{
		CountSpecific();
		EditedFlag = TRUE;
	}

	return (edited);
}


// ******************************************************************
// ******************************************************************

short D_PROF::Generalize(PATPlaySituation &sit, 
						 short playNum)
{
	if (!NumSpecific)
		return FALSE;

	char playCat;
	short chosenPlay;
	PlayInPlan play;
	short edited = FALSE;

	if (playNum == -1)
	{
		for (playNum = 0; playNum < 3; playNum++)
		{
			chosenPlay = PATLogic[sit.SitNum].Play[playNum].Num;
			chosenPlay -= 128;

			if (chosenPlay >= 0)
			{
				if (Gameplan->GetPlay(&play, chosenPlay))
				{
					playCat = play.Info.UserCat;

					if ((playCat == -1) || (playCat == -2))
						playCat = play.Info.PlayCat;

					PATLogic[sit.SitNum].Play[playNum].Num =
							GetGeneralType(playCat);
				}
				else
					PATLogic[sit.SitNum].Play[playNum].Num = 0;

				edited = TRUE;
			}
		}
	}
	else
	{
		chosenPlay = PATLogic[sit.SitNum].Play[playNum].Num;
		chosenPlay -= 128;

		if (chosenPlay >= 0)
		{
			if (Gameplan->GetPlay(&play, chosenPlay))
			{
				playCat = play.Info.UserCat;

				if ((playCat == -1) || (playCat == -2))
					playCat = play.Info.PlayCat;

				PATLogic[sit.SitNum].Play[playNum].Num =
						GetGeneralType(playCat);
			}
			else
				PATLogic[sit.SitNum].Play[playNum].Num = 0;

			edited = TRUE;
		}
	}

	if (edited)
	{
		CountSpecific();
		EditedFlag = TRUE;
	}

	return (edited);
}


// ******************************************************************
// ******************************************************************

short D_PROF::Cycle(PlaySituation &sit, 
					short playNum)
{
	short chosenPlay;

	uchar newType[] =
	{
		0, 1, 3, 4, 23, 5, 6, 8, 9, 24, 11, 12, 25,
		14, 15, 26, 16, 17, 18, 19, 20, 21, 22, 2, 7, 10, 13
	};

	chosenPlay = PlaysToCall[sit.SitNum].Play[playNum].Num;

	if (chosenPlay > 26)
		return (FALSE);

	PlaysToCall[sit.SitNum].Play[playNum].Num = newType[chosenPlay];

	if (chosenPlay != newType[chosenPlay])
	{
		CountSpecific();
		return (TRUE);
	}

	return (FALSE);
}


// ******************************************************************
// ******************************************************************

short D_PROF::Cycle(PATPlaySituation &sit, 
					short playNum)
{
	short chosenPlay;

	uchar newType[] =
	{
		0, 1, 3, 4, 23, 5, 6, 8, 9, 24, 11, 12, 25,
		14, 15, 26, 16, 17, 18, 19, 20, 21, 22, 2, 7, 10, 13
	};

	chosenPlay = PATLogic[sit.SitNum].Play[playNum].Num;

	if (chosenPlay > 26)
		return (FALSE);

	PATLogic[sit.SitNum].Play[playNum].Num = newType[chosenPlay];

	if (chosenPlay != newType[chosenPlay])
	{
		CountSpecific();
		return (TRUE);
	}

	return (FALSE);
}


// ******************************************************************
// ******************************************************************

void D_PROF::ChangePlays(PlaysForSitEdit *plays, 
						 ushort sitNum)
{
	short playNum;

	if (sitNum < NUM_SITUATIONS)
	{
		for (playNum=0; playNum<3; playNum++)
		{
			if (PlaysToCall[sitNum].Play[playNum].Num >= 128)
				NumSpecific--;

			if (plays->Play[playNum].Num >= 128)
				NumSpecific++;

			PlaysToCall[sitNum].Play[playNum].Num = plays->Play[playNum].Num;
			PlaysToCall[sitNum].Play[playNum].Weight = plays->Play[playNum].Weight & 0x0f;
		}

		if (plays->StopClock)
			PlaysToCall[sitNum].Play[0].Weight |= 0x80;

		EditedFlag = TRUE;
	}
	else if ((sitNum - NUM_SITUATIONS) < NUM_PAT_SITS)
	{
		sitNum -= NUM_SITUATIONS;

		for (playNum = 0; playNum < 3; playNum++)
		{
			if (PATLogic[sitNum].Play[playNum].Num >= 128)
				NumSpecific--;

			if (plays->Play[playNum].Num >= 128)
				NumSpecific++;

			PATLogic[sitNum].Play[playNum].Num = plays->Play[playNum].Num;
			PATLogic[sitNum].Play[playNum].Weight = plays->Play[playNum].Weight & 0x0f;
		}

		if (plays->StopClock)
			PATLogic[sitNum].Play[0].Weight |= 0x80;

		EditedFlag = TRUE;
	}
}


// ******************************************************************
// ******************************************************************

void D_PROF::GetPlays(PlaysForSitEdit *plays, 
					  PlaySituation &sit)
{
	short playNum;
	ushort sitNum = sit.GetSitNum();

	if (sitNum >= NUM_SITUATIONS)
		return;

	for (playNum = 0; playNum < 3; playNum++)
	{
		plays->Play[playNum].Num = PlaysToCall[sitNum].Play[playNum].Num;
		plays->Play[playNum].Weight = PlaysToCall[sitNum].Play[playNum].Weight & 0x0f;
	}

	plays->StopClock = (PlaysToCall[sitNum].Play[0].Weight & 0x80) ? 1 : 0;
}


// ******************************************************************
// ******************************************************************

void D_PROF::GetPlays(PlaysForSitEdit *plays, 
					  PATPlaySituation &sit)
{
	short playNum;
	ushort sitNum = sit.GetSitNum();

	if (sitNum >= NUM_PAT_SITS)
		return;

	for (playNum = 0; playNum < 3; playNum++)
	{
		plays->Play[playNum].Num = PATLogic[sitNum].Play[playNum].Num;
		plays->Play[playNum].Weight = PATLogic[sitNum].Play[playNum].Weight & 0x0f;
	}
}


// ******************************************************************
// ******************************************************************

void D_PROF::GetSubs(SubstitutionTable *table)
{
	memcpy(table, &Subs, sizeof(SubstitutionTable));
}


// ******************************************************************
// ******************************************************************

void D_PROF::SetSubs(SubstitutionTable *table)
{
	memcpy(&Subs, table, sizeof(SubstitutionTable));
	EditedFlag = TRUE;
}


// ******************************************************************
// ******************************************************************

short PrnPaperOutCheck()
{
	while (Printer->Type && !Printer->Ready())
	{
		if (D_AutoReq(gpErrMsg(ERR_PRN_NOT_READY),
				"Retry\0Abort Printing\0") == 2)
			return (TRUE);
	}

	return (FALSE);
}


// ******************************************************************
// JDW [96-09-10] - rewrote this almost completely
// ******************************************************************

void D_PROF::Print(short titleFlag, 
				   BOOL printPAT, 
				   short totPages,
				   short totSits, 
				   short timeFlag[], 
				   short downFlag[],
				   short ytgFlag[], 
				   short posFlag[], 
				   short ptsFlag[])
{
	char headerStr[][15] =
	{
		"SIT#",
		"MINUTES",
		"DOWN",
		"YARDS",
		"FIELD POSITION",
		"POINT SPREAD ",
		"STOP",
		"PLAY1   ",
		"TYPE1",
		"WT1",
		"%1  ",
		"PLAY2   ",
		"TYPE2",
		"WT2",
		"%2  ",
		"PLAY3   ",
		"TYPE3",
		"WT3",
		"%3  ",
		"PAT#"
	};

	char PATheaderStr[][15] =
	{
		"PAT#",
		"MINUTES",
		"POINT SPREAD ",
		"PLAY1   ",
		"TYPE1",
		"WT1",
		"%1  ",
		"PLAY2   ",
		"TYPE2",
		"WT2",
		"%2  ",
		"PLAY3   ",
		"TYPE3",
		"WT3",
		"%3  "
	};


	char typeStr[][6] =
	{
		"GLRun", "GLRun", // 0
		"RDRun", "RDRun", // 1
		"RunLt", "RunLt", // 2
		"RunMd", "RunMd", // 3
		"RunRt", "RunRt", // 4
		"GLPas", "GLPas", // 5
		"RDPas", "RDPas", // 6
		"PsLLt", "PassL", // 7
		"PsLMd", "PassL", // 8
		"PsLRt", "PassL", // 9
		"PsMLt", "PassM", // 10
		"PsMMd", "PassM", // 11
		"PsMRt", "PassM", // 12
		"PsSLt", "PassS", // 13
		"PsSMd", "PassS", // 14
		"PsSRt", "PassS", // 15
		"FGPAT", "FGPAT", // 16
		"FkFGR", "FkFGR", // 17
		"FkFGP", "FkFGP", // 18
		"Punt ", "Punt ", // 19
		"FkPtR", "FkPtR", // 20
		"FkPtP", "FkPtP", // 21
		"RnClk", "RnClk", // 22
		"RnRnd", "RnRnd", // 23
		"PLRnd", "PLRnd", // 24
		"PMRnd", "PMRnd", // 25
		"PSRnd", "PSRnd"  // 26
	};

	int xHeader[20], xPATHeader[15];

	int xFactor[] = {
						  0,  38, 102, 140, 186, 301, 413, 449, 520, 566, 
						597, 638, 707, 753, 783, 824, 893, 939, 969
					};

	int xPATFactor[] =	{
							  0,  38, 102, 224, 296, 342, 372, 411, 
							482, 528, 559, 597, 668, 714, 745
						};

	short clockToPrint = FALSE, playToPrint = FALSE;
	short sitNum, sitsPerPage = SITS_PER_PAGE;
	char time, down, ytg, pos, pts;
	short page = 1, headerNum = 0, line = 0;
	short totWeight;
	short percentage[3], playNum, totPcnt, lowPcnt = 101, lowNum;
	char stopClockFlag;
	char playName[3][9];
	char playType[3][6];
	char playWeight[3][4];
	char pcntStr[3][5];
	char prnBuf[10];
	char ff[] = "\x0c";
	char sp[] = "\x20\x20\x20\x20\x20\x20\x20";
	char footerStr[80];
	PlaySituation playSit;
	PATPlaySituation patPlaySit;
	PlayInPlan *playPtr;
	PlaysForSit far *playsPtr;

	TEXTMETRIC	sTM;
	long		xFooter, yFooter;
	int			hPrinterRes, vPrinterRes;
	int			lCol, rCol;
	SIZE		sSizeStr;
	int			i;

#if 0
	if (Super)
	{
		short filter;

		do
		{
			filter = D_AutoReq("Filter by:", "Clock\0Play Type\0Specific Play\0Print\0Cancel\0");

			if (filter == 1)
			{
				clockToPrint = D_AutoReq("Print situations where...",
					"Clock is stopped\0Clock is not stopped\0Unfiltered\0");

				if (clockToPrint == 3)
					clockToPrint = FALSE;
			}
			else if (filter == 2)
			{
				playToPrint = D_AutoReq("Print situations with...",
						(OffFlag) ?
						"FG/PAT\0Fake FG Run\0Fake FG Pass\0\
						Punt\0Fake Punt Run\0Fake Punt Pass\0\
						Run Clock\0Unfiltered\0" :
						"FG/PAT\0Fake FG Run\0Fake FG Pass\0\
						Punt\0Fake Punt Run\0Fake Punt Pass\0\
						Unfiltered\0");

				if (playToPrint <= 6)
					playToPrint += 15;
				else if (playToPrint == 7)
				{
					if (OffFlag)
						playToPrint = 22;
					else
						playToPrint = 8;
				}

				if (playToPrint == 8)
					playToPrint = FALSE;
			}
			else if (filter == 3)
				D_OKBox("This does not work yet");
			else if (filter == 4)
			{
				char filterStr[80] = "Filter: ";

				if (clockToPrint)
					strcat(filterStr, (clockToPrint == 1) ? "[Stop clock]" : "[Don't stop clock]");

				if (playToPrint)
				{
					if (playToPrint == 16)
						strcat(filterStr, "[FG/PAT]");
					else if (playToPrint == 19)
						strcat(filterStr, "[Punt]");
					else if (playToPrint == 22)
						strcat(filterStr, "[Run Clock]");
				}

				if (!clockToPrint && !playToPrint)
					strcat(filterStr, "NONE");

				if (D_AutoReq(filterStr, OCButStr, TRUE) == 2)
					return;
			}
			else if (filter == 5)
				return;
		}
		while(filter != 4);
	}
#endif

	char fgRangeStr[3];
	itoa(FGRange, fgRangeStr, 10);
	strcpy(PosButStr[1], "DEF 5-");
	strcat(PosButStr[1], fgRangeStr);
	strncat(PosButStr[1], sp, (8 - strlen(fgRangeStr)));
	strcpy(PosButStr[2], "DEF ");
	strcat(PosButStr[2], fgRangeStr);
	strcat(PosButStr[2], "-OFF 35");
	strncat(PosButStr[2], sp, (3 - strlen(fgRangeStr)));

	// Print the title page
	if (Printer->TitlePage == IDC_RADIO_PRINT_TITLE_YES)
	{
		Printer->UpdateStatus(this);
		short specNum[] =
					{65, 73, 67, 72, 64, 68, 69, 0, 66, 70, 71};
		char offFlag = OffFlag;
		char szCat[25]; 

		Printer->LoadAltFont("Arial", 240, 0, FONT_BOLD, FALSE);
		Printer->SetAltFont();

		GetTextMetrics(Printer->GetPrinterDC(), &sTM);
		hPrinterRes = GetDeviceCaps(Printer->GetPrinterDC(), HORZRES);
		vPrinterRes = GetDeviceCaps(Printer->GetPrinterDC(), VERTRES);
		lCol = (hPrinterRes * 45) / 100;
		rCol = hPrinterRes >> 1;
		Printer->SetCurLine(2);

		strcpy(szCat, "Profile:");
		GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
		TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		sprintf(szCat, "%s",ProfName);
		TextOut( Printer->GetPrinterDC(), rCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		Printer->SetCurLine(Printer->GetCurLine()+1);
		
		strcpy(szCat, "Team Name:");
		GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
		TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		sprintf(szCat, "%s",Roster->TeamName);
		TextOut( Printer->GetPrinterDC(), rCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		Printer->SetCurLine(Printer->GetCurLine()+1);
		
		strcpy(szCat, "Nickname:");
		GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
		TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		sprintf(szCat, "%s",Roster->Nickname);
		TextOut( Printer->GetPrinterDC(), rCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		Printer->SetCurLine(Printer->GetCurLine()+1);
		
		strcpy(szCat, "Abbr:");
		GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
		TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		sprintf(szCat, "%s",Roster->Abbr);
		TextOut( Printer->GetPrinterDC(), rCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		Printer->SetCurLine(Printer->GetCurLine()+1);
		
		strcpy(szCat, "Coach:");
		GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
		TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		sprintf(szCat, "%s",Roster->CoachName);
		TextOut( Printer->GetPrinterDC(), rCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		Printer->SetCurLine(Printer->GetCurLine()+1);

		strcpy(szCat, "League:");
		GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
		TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		sprintf(szCat, "%s",Roster->LeagueName);
		TextOut( Printer->GetPrinterDC(), rCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		Printer->SetCurLine(Printer->GetCurLine()+1);

		if (Roster->ConfName) 
		{
			strcpy(szCat, "Conference:");
			GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
			TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
			sprintf(szCat, "%s",Roster->ConfName);
			TextOut( Printer->GetPrinterDC(), rCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
			Printer->SetCurLine(Printer->GetCurLine()+1);

			strcpy(szCat, "Division:");
			GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
			TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
			sprintf(szCat, "%s",Roster->DivName);
			TextOut( Printer->GetPrinterDC(), rCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
			Printer->SetCurLine(Printer->GetCurLine()+1);
		}
		else
		{
			strcpy(szCat, "Division:");
			GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
			TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
			sprintf(szCat, "%s",Roster->DivName);
			TextOut( Printer->GetPrinterDC(), rCol, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
			Printer->SetCurLine(Printer->GetCurLine()+1);
		}

		char		subStr[40];
		SIZE		sSizeStr;
		TEXTMETRIC	sTM;
		int			xPos, xOut, xIn;
		strcpy(subStr, "Field Goal Range: ");
		strcat(subStr, fgRangeStr);
		strcat(subStr, " yard line");
		Printer->SetCurLine(Printer->GetCurLine() + 3);
		Printer->LoadAltFont("Arial", 120, 0, FONT_NORMAL, FALSE);
		Printer->SetAltFont();
		GetTextMetrics(Printer->GetPrinterDC(), &sTM);
		GetTextExtentPoint32(Printer->GetPrinterDC(), subStr, strlen(subStr), &sSizeStr);
		TextOut( Printer->GetPrinterDC(), ((Printer->hPrinterRes - sSizeStr.cx)>>1), (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) subStr, strlen((PCHAR) subStr));
		Printer->SetCurLine(Printer->GetCurLine() + 3);
		sprintf(subStr, "Substitution Percentages");
		GetTextExtentPoint32(Printer->GetPrinterDC(), subStr, strlen(subStr), &sSizeStr);
		TextOut( Printer->GetPrinterDC(), ((Printer->hPrinterRes - sSizeStr.cx)>>1), (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) subStr, strlen((PCHAR) subStr));
		Printer->LineFeed();
		xPos = (Printer->hPrinterRes * 3)/10;
		xOut = (Printer->hPrinterRes * 55)/100;
		xIn = (Printer->hPrinterRes * 65)/100;
		TextOut( Printer->GetPrinterDC(), xPos, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) "POSITION", strlen((PCHAR) "POSITION"));
		TextOut( Printer->GetPrinterDC(), xOut, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) "OUT%", strlen((PCHAR) "OUT%"));
		TextOut( Printer->GetPrinterDC(), xIn, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) "IN%", strlen((PCHAR) "IN%"));
		Printer->LineFeed();

		short i;

		char *offPosStr[] =
		{
			"Off. Linemen",
			"Quarterbacks",
			"Running Backs",
			"Receivers",
			"Kickers"
		};

		char *defPosStr[] =
		{
			"Def. Linemen",
			"Linebackers",
			"Def. Backs"
		};

		for (i = 0; i < (OffFlag ? 5 : 3); i++)
		{
			if (OffFlag)
				strcpy(subStr, offPosStr[i]);
			else
				strcpy(subStr, defPosStr[i]);
			TextOut( Printer->GetPrinterDC(), xPos, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) subStr, strlen((PCHAR) subStr));

			pos = OffFlag ? ((i == 4) ? 7 : i) : (i + 4);
			itoa(Subs.Percentage[pos][0], subStr, 10);
			strcat(subStr, "%");
			TextOut( Printer->GetPrinterDC(), xOut, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) subStr, strlen((PCHAR) subStr));
			
			itoa(Subs.Percentage[pos][1], subStr, 10);
			strcat(subStr, "%");
			TextOut( Printer->GetPrinterDC(), xIn, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) subStr, strlen((PCHAR) subStr));
			
			Printer->LineFeed();
		}

		Printer->SetStandardFont();
		Printer->FormFeed();
		if (!Printer->PrintFile())
			return;
	}

	// set up Windows "tab stops"
	long	xPrintout = ((long)Printer->hPrinterRes * 95)/100;
	for (i = 0; i < 19; i++)
	{
		xHeader[i] = (int)((xPrintout * (long)xFactor[i])/1000L);
		if (i < 15)
			xPATHeader[i] = (int)((xPrintout * (long)xPATFactor[i])/1000L);
	}

	if (!totSits)
		return;

	playPtr = new PlayInPlan;

	// Print play calling logic
	for (sitNum = 0; sitNum < (NUM_SITUATIONS + NUM_PAT_SITS); sitNum++)
	{
		if (sitNum < NUM_SITUATIONS)
		{
			playSit.SetSitNum(sitNum);
			playSit.GetSit(&time, &down, &ytg, &pos, &pts);
			playsPtr = &PlaysToCall[sitNum];

			if (clockToPrint)
			{
				if (((playsPtr->Play[0].Weight & 0x80) >> 7) ^ (clockToPrint == 1))
					continue;
			}

			if (playToPrint)
			{
				if ((playsPtr->Play[0].Num != playToPrint) &&
						(playsPtr->Play[1].Num != playToPrint) &&
						(playsPtr->Play[2].Num != playToPrint))
					continue;
			}

			if (!timeFlag[time] || !downFlag[down] || !ytgFlag[ytg] || !posFlag[pos] || !ptsFlag[pts])
				continue;
		}
		else
		{
			if (!printPAT)
				continue;

			patPlaySit.SetSitNum(sitNum - NUM_SITUATIONS);
			patPlaySit.GetSit(&time, &pts);
			playsPtr = &PATLogic[sitNum - NUM_SITUATIONS];
		}

		if (line == sitsPerPage)
		{
			strcpy(footerStr, ProfName);
			strcat(footerStr, "    Page ");
			itoa(page, footerStr + strlen(footerStr), 10);
			strcat(footerStr, " of ");
			itoa(totPages, footerStr + strlen(footerStr), 10);
			Printer->LoadAltFont("Arial", 100, 0, FONT_BOLD, FALSE);
			Printer->SetAltFont();
			GetTextMetrics(Printer->GetPrinterDC(), &sTM);
			hPrinterRes = GetDeviceCaps(Printer->GetPrinterDC(), HORZRES);
			vPrinterRes = GetDeviceCaps(Printer->GetPrinterDC(), VERTRES);
			xFooter = sTM.tmAveCharWidth * strlen((PCHAR) footerStr);
			yFooter = sTM.tmHeight + sTM.tmInternalLeading;
			TextOut( Printer->GetPrinterDC(), (hPrinterRes - xFooter) >> 1, vPrinterRes - (yFooter << 1), (PCHAR) footerStr, strlen((PCHAR) footerStr));
			Printer->SetStandardFont();
			Printer->FormFeed();
			line = 0;
		}

		if ((line == 0) || (sitNum == NUM_SITUATIONS))
		{
			Printer->UpdateStatus(this, totPages + titleFlag, page + titleFlag);

			if (sitNum < NUM_SITUATIONS)
			{
				Printer->LoadAltFont("Arial Narrow", 80, 0, FONT_UNDERLINE, FALSE);
				Printer->SetAltFont();
				for (headerNum = 0; headerNum < 19; headerNum++)
				{
					TextOut( Printer->GetPrinterDC(), xHeader[headerNum], (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) headerStr[headerNum], strlen((PCHAR) headerStr[headerNum]));
				}
			}
			else
			{
				Printer->LoadAltFont("Arial Narrow", 80, 0, FONT_UNDERLINE, FALSE);
				Printer->SetAltFont();
				for (headerNum = 0; headerNum < 15; headerNum++)
				{
					TextOut( Printer->GetPrinterDC(), xPATHeader[headerNum], (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) PATheaderStr[headerNum], strlen((PCHAR) PATheaderStr[headerNum]));
				}
			}
			line++;
			Printer->LoadAltFont("Arial Narrow", 80, 0, FONT_NORMAL, FALSE);
			Printer->SetAltFont();
			Printer->LineFeed();
			Printer->SetStandardFont();
		}

		if (playsPtr->Play[0].Weight & 0x80)
			stopClockFlag = TRUE;
		else
			stopClockFlag = FALSE;

		totWeight = (playsPtr->Play[0].Weight & 0x0f) +
				playsPtr->Play[1].Weight +
				playsPtr->Play[2].Weight;
		sys_assert((totWeight > 0),
				"Odd... that shouldn't happen.");

		for (playNum = 0; playNum < 3; playNum++)
			percentage[playNum] = (playsPtr->Play[playNum].Weight & 0x0f) ?
								(((playsPtr->Play[playNum].Weight &
								0x0f) * 100) + (totWeight / 2)) /
								totWeight : 0;

		totPcnt = percentage[0] + percentage[1] + percentage[2];

		if (totPcnt != 100)
		{
			for (playNum = 0; playNum < 3; playNum++)
				if (percentage[playNum] &&
						(percentage[playNum] < lowPcnt))
				{
					lowPcnt = percentage[playNum];
					lowNum = playNum;
				}

			percentage[lowNum] += 100 - totPcnt;
		}

		for (playNum = 0; playNum < 3; playNum++)
		{
			itoa(playsPtr->Play[playNum].Weight & 0x0f,
					playWeight[playNum], 10);
			strncat(playWeight[playNum], sp,
					3 - strlen(playWeight[playNum]));
		}

		for (playNum = 0; playNum < 3; playNum++)
		{
			itoa(percentage[playNum], pcntStr[playNum], 10);
			strcat(pcntStr[playNum], "%");
			strncat(pcntStr[playNum], sp,
					4 - strlen(pcntStr[playNum]));
		}

		itoa((sitNum % NUM_SITUATIONS) + 1, prnBuf, 10);

		for (playNum = 0; playNum < 3; playNum++)
		{
			short num = playsPtr->Play[playNum].Num;

			if ((num >= 128) && (Gameplan->GetPlay(playPtr, num - 128)))
			{
				playPtr->GetPlayName(playName[playNum]);
				num = GetGeneralType(playPtr->Info.UserCat);
				strcpy(playType[playNum],
						typeStr[(num << 1) + !OffFlag]);
			}
			else
			{
				strcpy(playName[playNum], "--------");

				if (num >= 128)
					strcpy(playType[playNum], "-----");
				else
					strcpy(playType[playNum],
							typeStr[(num << 1) + !OffFlag]);
			}
		}

		if (sitNum < NUM_SITUATIONS)
		{
			HDC	hDC = Printer->GetPrinterDC();
			int	yLine = (Printer->GetCurLine() * Printer->GetLineHeight());

			Printer->LoadAltFont("Arial Narrow", 80, 0, FONT_NORMAL, FALSE);
			Printer->SetAltFont();
			TextOut( hDC, xHeader[0], yLine, (PCHAR) prnBuf, strlen((PCHAR) prnBuf));
			TextOut( hDC, xHeader[1], yLine, (PCHAR) TimeButStr[time], strlen((PCHAR) TimeButStr[time]));
			TextOut( hDC, xHeader[2], yLine, (PCHAR) DownButStr[down], strlen((PCHAR) DownButStr[down]));
			TextOut( hDC, xHeader[3], yLine, (PCHAR) YTGButStr[ytg], strlen((PCHAR) YTGButStr[ytg]));
			TextOut( hDC, xHeader[4], yLine, (PCHAR) PosButStr[pos], strlen((PCHAR) PosButStr[pos]));
			TextOut( hDC, xHeader[5], yLine, (PCHAR) PtsButStr[pts], strlen((PCHAR) PtsButStr[pts]));
			if (stopClockFlag == TRUE)
				TextOut( hDC, xHeader[6], yLine, (PCHAR) "Yes ", strlen((PCHAR) "Yes "));
			else
				TextOut( hDC, xHeader[6], yLine, (PCHAR) "No  ", strlen((PCHAR) "No  "));
			TextOut( hDC, xHeader[7], yLine, (PCHAR) playName[0], strlen((PCHAR) playName[0]));
			TextOut( hDC, xHeader[8], yLine, (PCHAR) playType[0], strlen((PCHAR) playType[0]));
			TextOut( hDC, xHeader[9], yLine, (PCHAR) playWeight[0], strlen((PCHAR) playWeight[0]));
			TextOut( hDC, xHeader[10], yLine, (PCHAR) pcntStr[0], strlen((PCHAR) pcntStr[0]));
			TextOut( hDC, xHeader[11], yLine, (PCHAR) playName[1], strlen((PCHAR) playName[1]));
			TextOut( hDC, xHeader[12], yLine, (PCHAR) playType[1], strlen((PCHAR) playType[1]));
			TextOut( hDC, xHeader[13], yLine, (PCHAR) playWeight[1], strlen((PCHAR) playWeight[1]));
			TextOut( hDC, xHeader[14], yLine, (PCHAR) pcntStr[1], strlen((PCHAR) pcntStr[1]));
			TextOut( hDC, xHeader[15], yLine, (PCHAR) playName[2], strlen((PCHAR) playName[2]));
			TextOut( hDC, xHeader[16], yLine, (PCHAR) playType[2], strlen((PCHAR) playType[2]));
			TextOut( hDC, xHeader[17], yLine, (PCHAR) playWeight[2], strlen((PCHAR) playWeight[2]));
			TextOut( hDC, xHeader[18], yLine, (PCHAR) pcntStr[2], strlen((PCHAR) pcntStr[2]));
			Printer->SetStandardFont();
		}
		else
		{
			HDC	hDC = Printer->GetPrinterDC();
			int	yLine = (Printer->GetCurLine() * Printer->GetLineHeight());

			Printer->LoadAltFont("Arial Narrow", 80, 0, FONT_NORMAL, FALSE);
			Printer->SetAltFont();
			TextOut( hDC, xPATHeader[0], yLine, (PCHAR) prnBuf, strlen((PCHAR) prnBuf));
			TextOut( hDC, xPATHeader[1], yLine, (PCHAR) TimeButStr[(time == 3) ? 5 : time], strlen((PCHAR) TimeButStr[(time == 3) ? 5 : time]));
			TextOut( hDC, xPATHeader[2], yLine, (PCHAR) PATPtsButStr[pts], strlen((PCHAR) PATPtsButStr[pts]));
			TextOut( hDC, xPATHeader[3], yLine, (PCHAR) playName[0], strlen((PCHAR) playName[0]));
			TextOut( hDC, xPATHeader[4], yLine, (PCHAR) playType[0], strlen((PCHAR) playType[0]));
			TextOut( hDC, xPATHeader[5], yLine, (PCHAR) playWeight[0], strlen((PCHAR) playWeight[0]));
			TextOut( hDC, xPATHeader[6], yLine, (PCHAR) pcntStr[0], strlen((PCHAR) pcntStr[0]));
			TextOut( hDC, xPATHeader[7], yLine, (PCHAR) playName[1], strlen((PCHAR) playName[1]));
			TextOut( hDC, xPATHeader[8], yLine, (PCHAR) playType[1], strlen((PCHAR) playType[1]));
			TextOut( hDC, xPATHeader[9], yLine, (PCHAR) playWeight[1], strlen((PCHAR) playWeight[1]));
			TextOut( hDC, xPATHeader[10], yLine, (PCHAR) pcntStr[1], strlen((PCHAR) pcntStr[1]));
			TextOut( hDC, xPATHeader[11], yLine, (PCHAR) playName[2], strlen((PCHAR) playName[2]));
			TextOut( hDC, xPATHeader[12], yLine, (PCHAR) playType[2], strlen((PCHAR) playType[2]));
			TextOut( hDC, xPATHeader[13], yLine, (PCHAR) playWeight[2], strlen((PCHAR) playWeight[2]));
			TextOut( hDC, xPATHeader[14], yLine, (PCHAR) pcntStr[2], strlen((PCHAR) pcntStr[2]));
			Printer->SetStandardFont();
		}
		//strncat(lineBuf, lf, lfLen);

		line++;
		Printer->LoadAltFont("Arial Narrow", 80, 0, FONT_NORMAL, FALSE);
		Printer->SetAltFont();
		Printer->LineFeed();
		short sitsThisPage = sitsPerPage;

		if (page == totPages)
			sitsThisPage = ((totSits - 1) % sitsPerPage) + 1;

		Printer->UpdateStatus(((long)line * 100) / sitsThisPage);
	}

	strcpy(footerStr, ProfName);
	strcat(footerStr, "    Page ");
	itoa(page, footerStr + strlen(footerStr), 10);
	strcat(footerStr, " of ");
	itoa(totPages, footerStr + strlen(footerStr), 10);
	Printer->LoadAltFont("Arial", 100, 0, FONT_BOLD, FALSE);
	Printer->SetAltFont();
	GetTextMetrics(Printer->GetPrinterDC(), &sTM);
	hPrinterRes = GetDeviceCaps(Printer->GetPrinterDC(), HORZRES);
	vPrinterRes = GetDeviceCaps(Printer->GetPrinterDC(), VERTRES);
	xFooter = sTM.tmAveCharWidth * strlen((PCHAR) footerStr);
	yFooter = sTM.tmHeight + sTM.tmInternalLeading;
	TextOut( Printer->GetPrinterDC(), (hPrinterRes - xFooter) >> 1, vPrinterRes - (yFooter << 1), (PCHAR) footerStr, strlen((PCHAR) footerStr));
	Printer->SetStandardFont();

	if (playPtr)
		delete playPtr;
}

// ******************************************************************
// ******************************************************************

short D_PROF::CopySit(short sitToCopy, 
					  short timeFlag[],
					  short downFlag[], 
					  short ytgFlag[], 
					  short posFlag[],
					  short ptsFlag[], 
					  short whatToCopy[])
{
	short sitNum, sitsCopied = 0;
	char srcClock, dstClock;
	char time, down, ytg, pos, pts;
	PlaySituation playSit;
	srcClock = PlaysToCall[sitToCopy].Play[0].Weight & 0x80;

	for (sitNum=0; sitNum<NUM_SITUATIONS; sitNum++)
	{
		Mouse.Idle();
		playSit.SetSitNum(sitNum);
		playSit.GetSit(&time, &down, &ytg, &pos, &pts);

		if (!timeFlag[time] || !downFlag[down] || !ytgFlag[ytg] ||
				!posFlag[pos] || !ptsFlag[pts] ||
				(sitNum == sitToCopy))
			continue;

		dstClock = PlaysToCall[sitNum].Play[0].Weight & 0x80;

		if (whatToCopy[0])
			memcpy(&PlaysToCall[sitNum], &PlaysToCall[sitToCopy], sizeof(PlaysForSit));

		PlaysToCall[sitNum].Play[0].Weight &= 0x0f;
		PlaysToCall[sitNum].Play[0].Weight |= (whatToCopy[1]) ? srcClock : dstClock;

		sitsCopied++;
		EditedFlag = TRUE;
	}

	if (EditedFlag)
		CountSpecific();

	Mouse.Active();
	return sitsCopied;
}


static char huge *savePtr = NULL;


// ******************************************************************
// ******************************************************************

void D_PROF::SavePlayCall()
{
	if (savePtr)
	{
		vm_hfree(savePtr);
		savePtr = NULL;
	}

	savePtr = vm_hmalloc(sizeof(PlaysToCall), MEMF_CLEAR);
	memcpy(savePtr, PlaysToCall, sizeof(PlaysToCall));
}


// ******************************************************************
// ******************************************************************

void D_PROF::RestorePlayCall(short restoreFlag)
{
	if (!savePtr)
		return;

	if (restoreFlag)
	{
		memcpy(PlaysToCall, savePtr, sizeof(PlaysToCall));
		CountSpecific();
	}

	vm_hfree(savePtr);
	savePtr = NULL;
}


// ******************************************************************
// ******************************************************************

void D_PROF::save_pat_logic()
{
	if (savePtr)
	{
		vm_hfree(savePtr);
		savePtr = NULL;
	}

	savePtr = vm_hmalloc(sizeof(PATLogic), MEMF_CLEAR);
	memcpy(savePtr, PATLogic, sizeof(PATLogic));
}


// ******************************************************************
// ******************************************************************

void D_PROF::restore_pat_logic(short restoreFlag)
{
	if (!savePtr)
		return;

	if (restoreFlag)
	{
		memcpy(PATLogic, savePtr, sizeof(PATLogic));
		CountSpecific();
	}

	vm_hfree(savePtr);
	savePtr = NULL;
}


// ******************************************************************
// ******************************************************************

void D_PROF::DrawInfoMem()
{
	vm_fill = TRUE;
	vm_fcolor = 9;
	DrawBox();

	if (!ActiveFlag)
		return;

	char infoStr[40];
	vm_fontselect((void *)MediumTeamFont);
	vm_fgcolor = 2;
	strcpy(infoStr, ProfName);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 6);
	strcpy(infoStr, (OffFlag) ? "Offense" : "Defense");
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 21);
	strcpy(infoStr, "Specific Plays: ");
	itoa(NumSpecific, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 36);
	strcpy(infoStr, "FG Range: ");
	itoa(FGRange, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 51);

	if (PlanFlag || NumSpecific)
	{
		strcpy(infoStr, "CONTAINS PLAN");
		vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 81);
	}

	vm_fgcolor = (OffFlag) ? 1 : 14;
	strcpy(infoStr, (OffFlag) ? "Offense" : "Defense");
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 20);
	vm_fgcolor = 15;
	strcpy(infoStr, ProfName);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 5);
	strcpy(infoStr, "Specific Plays: ");
	itoa(NumSpecific, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 35);
	strcpy(infoStr, "FG Range: ");
	itoa(FGRange, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 50);

	if (PlanFlag || NumSpecific)
	{
		strcpy(infoStr, "CONTAINS PLAN");
		vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 80);
	}
}


// ******************************************************************
// ******************************************************************

BOOL DrawProfInfoDisk(char *fileName)
{
	short saveBox = CurBox.num;
	D_PROF *tempProf = new D_PROF;
	InputFileHandler inProfFile;
	BOOL bValid = FALSE;

	if (inProfFile.Open(fileName))
	{
		if (inProfFile.FindFirstBlock(ProfInfoBlockStr))
			tempProf->LoadInfo(inProfFile);
		else if (inProfFile.FindFirstBlock(ProfBlockStr))
			tempProf->Load(inProfFile);

		inProfFile.GetFile(tempProf->Name());
		tempProf->DrawInfoMem();
		inProfFile.Close();
		bValid = TRUE;
	}

	delete tempProf;
	return (bValid);
}


// ******************************************************************
// ******************************************************************

void ClearProfInfo()
{
	vm_fill = TRUE;
	vm_fcolor = 9;
	DrawBox();
}
