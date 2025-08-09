/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
  PARTICULAR PURPOSE.

  Copyright (C) 1993-1996  Sierra On-Line.  All Rights Reserved.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
// PLAN.CPP
// Routines for loading and saving game plans.

#include "all.h"
#include "resource.h"
INCLUDE_ASSERT
#pragma warning (disable : 4244) // disable conversion from short to char and long to short warnings

// PlayInPlan public member functions----------------------------

void PlayInPlan::GetPlayName(char *playName)
{
	if (Info.StockFlag) 
	{
		*playName = 0;
		strncat(playName, Stock.PlayName, 8);
	}
	else 
	{
		char *dotPtr, *namePtr = strrchr(Cust.PlayName, '\\');

		if (!namePtr)
			namePtr = Cust.PlayName;
		else
			namePtr++;

		if (dotPtr = strrchr(namePtr, '.'))
			*dotPtr = 0;

		strcpy(playName, namePtr);
		if (dotPtr)
			*dotPtr = '.';
	}
}


// D_PLAN public member functions--------------------------------

D_PLAN::D_PLAN()
{
	Clear();
	PlanActive = FALSE;
	EditedFlag = FALSE;
	PlanFile = new char[MAXPATH + 1];
	PlanName = new char[13];
}


D_PLAN::~D_PLAN()
{
	if (PlanFile)
		delete []PlanFile;
	if (PlanName)
		delete []PlanName;
}


void far *D_PLAN::operator new(size_t Size)
{
	return ((void far *)vm_hmalloc(Size, MEMF_CLEAR));
}


void D_PLAN::operator delete(void far *Ptr)
{
	vm_hfree((char huge *)Ptr);
}


void D_PLAN::Clear()
{
	PlanActive = FALSE;
	EditedFlag = FALSE;
	short audNum;

	memset(NormalPlays, -1, sizeof(NormalPlays));
	memset(SpecialPlays, -1, sizeof(SpecialPlays));
	memset(StockSpecialPlays, -1, sizeof(StockSpecialPlays));
	memset(ClockPlays, -1, sizeof(ClockPlays));
	memset(NotFoundPlays, 0, sizeof(NotFoundPlays));

	for (audNum = 0; audNum < 4; audNum++)
		Audible[audNum] = audNum;

	NumNormPlays = 0;
	NumSpecPlays = 0;
	NumStockPlays = 0;
	NumCustPlays = 0;
}


void D_PLAN::InitSpec()
{
	char *offSpecName[] = {
				"FGPAT", "KICKOFF", "PUNT", "ONSDKICK", 
				"FKFGRUN", "FKFGPASS", "FKPNTRUN", "FKPNTPAS",
				"FREEKICK", "SQUIB", "RUNCLOCK", "STOPCLOK"};
	char *defSpecName[] = {
				"FGPATD", "KICKRET", "PUNTRET", "ONSIDRET",
				"FKFGRUND", "FKFGPASD", "FKPNTRND", "FKPNTPSD",
				"FREERET", "SQUIBRET"};

	short i;
	PlayInPlan *playPtr = new PlayInPlan;

	if (playPtr) 
	{
		for (i=0; i<NUM_SPEC_PLAYS; i++) 
		{
			memcpy(playPtr, &StockSpecialPlays[i], sizeof(PlayInPlan));
			playPtr->Stock.StockFlag = 1;
			playPtr->Stock.PlayCat = playPtr->Stock.UserCat = OffFlag;
			playPtr->Stock.SpecCat = i + 1;
			strncpy(playPtr->Stock.PlayName, (OffFlag) ? offSpecName[i] : defSpecName[i], 8);
			playPtr->Stock.Offset = StockPlays->GetStockOffset(playPtr->Stock.PlayName, &playPtr->Stock.Size);
			memcpy(&StockSpecialPlays[i], playPtr, sizeof(PlayInPlan));
		}

		for (i=0; (i<NUM_CLOCK_PLAYS) && OffFlag; i++) 
		{
			memcpy(playPtr, &ClockPlays[i], sizeof(PlayInPlan));
			playPtr->Stock.StockFlag = playPtr->Stock.PlayCat = playPtr->Stock.UserCat = 1;
			playPtr->Stock.SpecCat = i + 11;
			strncpy(playPtr->Stock.PlayName, offSpecName[i + NUM_SPEC_PLAYS], 8);
			playPtr->Stock.Offset = StockPlays->GetStockOffset(	playPtr->Stock.PlayName, &playPtr->Stock.Size);
			memcpy(&ClockPlays[i], playPtr, sizeof(PlayInPlan));
		}
		delete playPtr;
	}
}


short D_PLAN::Load(InputFileHandler &inPlanFile)
{
	short slotNum;//, numNotFound = 0;
	short *fileOff;
	char fileStr[MAXPATH + 1];
	char *data, *dataPtr, *playPtr;//, far *slotPtr;
	PlayInPlan far *slotPtr;
	NumNormPlays = 0;
	NumSpecPlays = 0;
	NumStockPlays = 0;
	NumCustPlays = 0;
	InputFileHandler inPlayFile;

	data = new char[slotNum = inPlanFile.BlockSize];
	sys_assert(data, "Not enough memory to load gameplan.");
	inPlanFile.Read(data, slotNum);
	Clear();
	OffFlag = !(TeamDirType & 1);

	for (slotNum = 0; slotNum < 4; slotNum++)
		Audible[slotNum] = *(data + slotNum);

	dataPtr = data + 4;
	fileOff = (short *) (dataPtr);

	for (slotNum = 0; slotNum < NUM_NORM_PLAYS; slotNum++) 
	{
		slotPtr = &NormalPlays[slotNum];
		if (*fileOff) 
		{
			playPtr = dataPtr + *fileOff;
			fileStr[0] = 0;
			if (*(playPtr + 5) != ':') 
			{
				strcpy(fileStr, peDirStr);
				if (fileStr[strlen(fileStr) - 1] != '\\')
					strcat(fileStr, "\\");
			}

			strcat(fileStr, playPtr + 4);
			if (*playPtr) 
			{
				memcpy(slotPtr, playPtr, sizeof(StockInPlan));
				NumStockPlays++;
				NumNormPlays++;
			}
			else if (inPlayFile.Open(fileStr) && inPlayFile.FindFirstBlock(PlayBlockStr)) 
			{
				inPlayFile.MoveForward(22l);
				inPlayFile.Read(&slotPtr->Cust.PlayCat, 3);
 				if (!slotPtr->Cust.SpecCat && ((slotPtr->Cust.PlayCat & 1) == OffFlag)) 
				{
					memcpy(slotPtr, playPtr, strlen(playPtr + 4) + 5);
					inPlayFile.FindFirstBlock(PlayBlockStr);
					inPlayFile.MoveForward(22l);
					inPlayFile.Read(&slotPtr->Cust.PlayCat, 3);
					slotPtr->Cust.StockFlag = 0;
					NumNormPlays++;
					NumCustPlays++;
				}
				else 
				{
					if (Profile->Active()) 
					{
						memcpy(slotPtr, playPtr, strlen(playPtr + 4) + 5);
						Profile->Generalize(slotNum);
					}
					NotFoundPlays[slotNum] = TRUE;
					slotPtr->Info.StockFlag = -1;
				}
			}
			else 
			{
				if (Profile->Active()) 
				{
					memcpy(slotPtr, playPtr, strlen(playPtr + 4) + 5);
					Profile->Generalize(slotNum);
				}
				NotFoundPlays[slotNum] = TRUE;
				slotPtr->Info.StockFlag = -1;
			}
			inPlayFile.Close();
		}
		else
			slotPtr->Info.StockFlag = -1;

		fileOff++;
	}

	for (slotNum = 0; slotNum < (NUM_SPEC_PLAYS * 2); slotNum++) 
	{
		if (slotNum % 2)
			slotPtr = &StockSpecialPlays[slotNum / 2];
		else
			slotPtr = &SpecialPlays[slotNum / 2];

		if (*fileOff) 
		{
			playPtr = dataPtr + *fileOff;
			fileStr[0] = 0;
			if (*(playPtr + 5) != ':') 
			{
				strcpy(fileStr, peDirStr);
				if (fileStr[strlen(fileStr) - 1] != '\\')
					strcat(fileStr, "\\");
			}

			strcat(fileStr, playPtr + 4);
			if (*playPtr)
				memcpy(slotPtr, playPtr, sizeof(StockInPlan));
			else if (inPlayFile.Open(fileStr) && inPlayFile.FindFirstBlock(PlayBlockStr)) 
			{
				inPlayFile.MoveForward(22l);
				inPlayFile.Read(&slotPtr->Cust.PlayCat, 3);
				if (slotPtr->Cust.SpecCat == *(playPtr + 2) && ((slotPtr->Cust.PlayCat & 1) == OffFlag)) 
				{
					memcpy(slotPtr, playPtr, strlen(playPtr + 4) + 5);
					slotPtr->Cust.StockFlag = 0;
					NumSpecPlays++;
				}
				else 
				{
					NotFoundPlays[NUM_NORM_PLAYS + (slotNum >> 1)] = TRUE;
					slotPtr->Info.StockFlag = -1;
				}
			}
			else 
			{
				NotFoundPlays[NUM_NORM_PLAYS + (slotNum >> 1)] = TRUE;
				slotPtr->Info.StockFlag = -1;
			}
			inPlayFile.Close();
		}
		else
			slotPtr->Info.StockFlag = -1;

		fileOff++;
	}
	if (data)
		delete []data;

	InitSpec();
	EditedFlag = FALSE;
	PlanActive = TRUE;
	return TRUE;
}


short D_PLAN::OldLoad(InputFileHandler &inPlanFile,
					  short halfFlag)
{
	short slotNum, play;//, numNotFound = 0;
	short *fileOff;
	short updateFlag = TRUE;
	char fileStr[MAXPATH + 1];
	char *data, *dataPtr, *playPtr; //, far *slotPtr;

	PlayInPlan far *slotPtr;
	InputFileHandler inPlayFile;
	data = new char[slotNum = inPlanFile.BlockSize];
	sys_assert(data, "Not enough memory to load gameplan.");
	inPlanFile.Read(data, slotNum);

	if (halfFlag == -1) 
	{
		Clear();
		halfFlag++;
		updateFlag = FALSE;
	}
	else if (OffFlag != !(TeamDirType & 1))
		Clear();
	for (slotNum = 0; slotNum < 4; slotNum++)
		Audible[slotNum] = *(data + slotNum);

	dataPtr = data + 4;
	fileOff = (short *)(dataPtr);

	for (slotNum = 0; slotNum < 32; slotNum++) 
	{
		play = (halfFlag) ? (slotNum + 32) : (slotNum);
		slotPtr = &NormalPlays[play];

		if (slotPtr->Cust.StockFlag != -1) 
		{
			if (!slotPtr->Cust.StockFlag)
				NumCustPlays--;
			else
				NumStockPlays--;
			NumNormPlays--;
			slotPtr->Cust.StockFlag = -1;
		}

		if (*fileOff) 
		{
			playPtr = dataPtr + *fileOff;
			fileStr[0] = 0;
			if (*(playPtr + 1) != ':') 
			{
				strcpy(fileStr, peDirStr);
				if (fileStr[strlen(fileStr) - 1] != '\\')
					strcat(fileStr, "\\");
			}
			strcat(fileStr, playPtr);
			if (inPlayFile.Open(fileStr)) 
			{
				if (!updateFlag) 
				{
					if (inPlayFile.FindFirstBlock(PlayBlockStr)) 
					{
						NumNormPlays++;
						NumCustPlays++;
					}
				}
				else 
				{
					if (inPlayFile.FindFirstBlock(PlayBlockStr)) 
					{
						memcpy(slotPtr->Cust.PlayName, playPtr,
									strlen(playPtr) + 1);
						inPlayFile.MoveForward(22l);
						inPlayFile.Read(&slotPtr->Cust.PlayCat, 2);
						if ((slotPtr->Cust.PlayCat & 1) == OffFlag) 
						{
							slotPtr->Cust.StockFlag = 0;
							slotPtr->Cust.UserCat =	slotPtr->Cust.PlayCat;
							NumNormPlays++;
							NumCustPlays++;
						}
						else 
						{
							NotFoundPlays[play] = TRUE;
							slotPtr->Cust.StockFlag = -1;
						}
					}
				}
				inPlayFile.Close();
			}
			else 
			{
				NotFoundPlays[play] = TRUE;
				slotPtr->Cust.StockFlag = -1;
			}
		}
		fileOff++;
	}

	if (data)
		delete [] data;

	OffFlag = !(TeamDirType & 1);
	InitSpec();
	EditedFlag = FALSE;
	PlanActive = TRUE;
	return TRUE;
}


short D_PLAN::Save(OutputFileHandler &outPlanFile)
{
	short i;
	short fileOff[TOT_NUM_PLAYS], *currOff;
	char far *slotPtr;
	long int tablePos;

	outPlanFile.Write(Audible, sizeof(Audible));
	tablePos = outPlanFile.GetPosition();
	outPlanFile.MarkSpot();
	outPlanFile.Write(fileOff, sizeof(fileOff));
	currOff = fileOff;

	for (i = 0; i < NUM_NORM_PLAYS; i++) 
	{
		slotPtr = (char far *)&NormalPlays[i];
		if (*slotPtr != -1) 
		{
			*currOff = outPlanFile.GetPosition() - tablePos;
			if (*slotPtr)
				outPlanFile.Write(slotPtr, sizeof(StockInPlan));
			else
				outPlanFile.Write(slotPtr, strlen(slotPtr + 4) + 5);
		}
		else
			*currOff = 0;
		currOff++;
	}

	for (i = 0; i < (NUM_SPEC_PLAYS * 2); i++) 
	{
		if (i % 2)
			slotPtr = (char far *) &StockSpecialPlays[i / 2];
		else
			slotPtr = (char far *) &SpecialPlays[i / 2];
		if (*slotPtr != -1) 
		{
			*currOff = outPlanFile.GetPosition() - tablePos;
			if (*slotPtr)
				outPlanFile.Write(slotPtr, sizeof(StockInPlan));
			else
				outPlanFile.Write(slotPtr, strlen(slotPtr + 4) + 5);
		}
		else
			*currOff = 0;
		currOff++;
	}

	for (i = 0; i < NUM_CLOCK_PLAYS; i++) 
	{
		slotPtr = (char far *)&ClockPlays[i];
		if ((*slotPtr != -1) && OffFlag) 
		{
			*currOff = outPlanFile.GetPosition() - tablePos;
			outPlanFile.Write(slotPtr, sizeof(StockInPlan));
		}
		else
			*currOff = 0;
		currOff++;
	}

	outPlanFile.GotoSpot();
	outPlanFile.Write(fileOff, sizeof(fileOff));
	outPlanFile.DeleteSpot();
	EditedFlag = FALSE;
	return TRUE;
}


void D_PLAN::LoadInfo(InputFileHandler &inPlanFile)
{
	PlanActive = TRUE;
	inPlanFile.Read(&OffFlag, sizeof(OffFlag));
	inPlanFile.Read(&NumCustPlays, sizeof(NumCustPlays));
	inPlanFile.Read(&NumStockPlays, sizeof(NumStockPlays));
	inPlanFile.Read(&NumSpecPlays, sizeof(NumSpecPlays));
}


void D_PLAN::SaveInfo(OutputFileHandler &outPlanFile)
{
	outPlanFile.Write(&OffFlag, sizeof(OffFlag));
	outPlanFile.Write(&NumCustPlays, sizeof(NumCustPlays));
	outPlanFile.Write(&NumStockPlays, sizeof(NumStockPlays));
	outPlanFile.Write(&NumSpecPlays, sizeof(NumSpecPlays));
}


void D_PLAN::NewPlan(short offFlag)
{
	Clear();
	OffFlag = offFlag;
	InitSpec();
	EditedFlag = FALSE;
	PlanActive = TRUE;
}


short D_PLAN::SlotFilled(short playNum)
{
	if (!Valid(playNum))
		return FALSE;

	if (playNum < NUM_NORM_PLAYS)
		return (NormalPlays[playNum].Info.StockFlag != -1);

	playNum -= NUM_NORM_PLAYS;
	return (!SpecialPlays[playNum].Info.StockFlag);
}


char D_PLAN::PlayNotFound(short playNum)
{
	if (!Valid(playNum) || SlotFilled(playNum))
		return FALSE;
	return NotFoundPlays[playNum];
}


void D_PLAN::AddPlay(PlayInPlan *play, short playNum)
{
	if (!play || !Valid(playNum))
		return;

	NotFoundPlays[playNum] = FALSE;

	if (playNum < NUM_NORM_PLAYS) 
	{
		memcpy(&NormalPlays[playNum], play, sizeof(PlayInPlan));
		if (play->Info.StockFlag)
			NumStockPlays++;
		else
			NumCustPlays++;
		NumNormPlays++;
	}
	else 
	{
		if (play->Info.StockFlag)
			return;
		playNum -= NUM_NORM_PLAYS;
		memcpy(&SpecialPlays[playNum], play, sizeof(PlayInPlan));
		NumSpecPlays++;
	}
	EditedFlag = TRUE;
}


short D_PLAN::GetPlay(PlayInPlan *play, short playNum)
{
	if (!play || !Valid(playNum) || !SlotFilled(playNum))
		return FALSE;

	if (playNum < NUM_NORM_PLAYS)
		memcpy(play, &NormalPlays[playNum], sizeof(PlayInPlan));
	else 
	{
		playNum -= NUM_NORM_PLAYS;
		memcpy(play, &SpecialPlays[playNum], sizeof(PlayInPlan));
	}
	return TRUE;
}


void D_PLAN::RemovePlay(short playNum)
{
	if (!Valid(playNum))
		return;

	NotFoundPlays[playNum] = FALSE;

	if (playNum < NUM_NORM_PLAYS) 
	{
		if (NormalPlays[playNum].Info.StockFlag == 0)
			NumCustPlays--;
		if (NormalPlays[playNum].Info.StockFlag == 1)
			NumStockPlays--;

		NormalPlays[playNum].Info.StockFlag = -1;
		NumNormPlays--;
	}
	else 
	{
		playNum -= NUM_NORM_PLAYS;
		SpecialPlays[playNum].Info.StockFlag = -1;
		NumSpecPlays--;
	}
	EditedFlag = TRUE;
}


#if 0
void D_PLAN::Print(short titleFlag)
{
	if (!titleFlag && !NumNormPlays)
		return;
	short drawPlayNum, planPage, totalPages, pagePlayNum;
	short startPlayNum = 0;
	InputFileHandler inFile;
	PlayInPlan *playPtr;
	D_PLAY *tempPlay;
	char saveDisk;
	char saveDir[MAXPATH + 1];
	char footerStr[80];
	short playsPerPage;
	if (Printer->Text == PRN_NOTEXT)
	{
		if (Printer->Diag == PRN_FULL)
			playsPerPage = 2;
		else if (Printer->Diag == PRN_MINI)
			playsPerPage = 32;
		else
			playsPerPage = 64;
	}
	else 
	{
		if (Printer->Diag == PRN_FULL)
			playsPerPage = 1;
		else
			playsPerPage = 2;
	}
	totalPages = 64 / playsPerPage;
	if (playsPerPage == 32) 
	{
		totalPages = 0;
		for (drawPlayNum = 32; drawPlayNum < 64; drawPlayNum++)
		{
			if (SlotFilled(drawPlayNum)) 
			{
				startPlayNum = 32;
				totalPages++;
				break;
			}
		}
		for (drawPlayNum = 0; drawPlayNum < 32; drawPlayNum++)
		{
			if (SlotFilled(drawPlayNum)) 
			{
				startPlayNum = 0;
				totalPages++;
				break;
			}
		}
	}
	if (playsPerPage == 2)
		totalPages = (NumNormPlays + 1) / 2;
	if (playsPerPage == 1)
		totalPages = NumNormPlays;
	SaveDir(saveDir, &saveDisk);
	RestoreDir(peDirStr, peDisk);
	playPtr = new PlayInPlan;
	tempPlay = new D_PLAY;
	if (!Printer->Type) 
	{
		char fileBuf[80];
		char bufLen;
		OutputFileHandler outFile;
		outFile.AddToFile("PRINT.OUT");
		outFile.Write("\x0d\x0a", 2);
		outFile.Write(PlanName, strlen(PlanName));
		outFile.Write("\x0d\x0a", 2);
		for (drawPlayNum = 0; drawPlayNum < NUM_NORM_PLAYS; drawPlayNum++) 
		{
			if ((drawPlayNum < 36) && !(drawPlayNum % 4))
				outFile.Write(" ", 1);
			PlayNumToRowColStr(drawPlayNum, fileBuf);
			strcat(fileBuf, " - ");
			if (GetPlay(playPtr, drawPlayNum)) 
			{
				if (playPtr->Info.StockFlag)
					/*_f*/ strncat(fileBuf, playPtr->Info.PlayName, 8);
				else if (inFile.Open(playPtr->Cust.PlayName)) 
				{
					if (inFile.FindFirstBlock(PlayBlockStr))
						playPtr->GetPlayName(fileBuf + strlen(fileBuf));
					else
						/*_f*/ strcat(fileBuf, "Empty");
 					inFile.Close();
				}
				else
					/*_f*/ strcat(fileBuf, "Empty");
			}
			else
				/*_f*/ strcat(fileBuf, "Empty");
			bufLen = strlen(fileBuf);
			outFile.Write(fileBuf, bufLen);
			bufLen = 20 - bufLen;
			while (bufLen--)
				outFile.Write(" ", 1);
			if ((drawPlayNum % 4) == 3)
				outFile.Write("\x0d\x0a", 2);
		}
		outFile.Close();
		delete tempPlay;
		delete playPtr;
		RestoreDir(saveDir, saveDisk);
		return;
	}
	if (titleFlag) 
	{
		Printer->OpenPrnFile(PlanName, totalPages + titleFlag, 1);
		Printer->UpdateStatus(this);
		short specNum[] =
					{65, 73, 67, 72, 64, 68, 69, 0, 66, 70, 71};
		char offFlag = OffFlag;
		pgSetTextStyle(pgPRINT);
		pgSetTextJustify(pgCENTER, pgCENTER);
		pgSetColor(1);
		pgSetTextScaling(4, 1, 4, 1);
		pgSetTextJustify(pgRIGHT, pgCENTER);
		pgDrawTextXY(700, 100, "Game Plan:");
		pgDrawTextXY(700, 300, "Team Name:");
		pgDrawTextXY(700, 500, "Nickname:");
		pgDrawTextXY(700, 700, "Abbr:");
		pgDrawTextXY(700, 900, "Coach:");
		pgDrawTextXY(700, 1100, "League:");
		if (Roster->ConfName) 
		{
			pgDrawTextXY(700, 1300, "Conference:");
			pgDrawTextXY(700, 1500, "Division:");
		}
		else
			pgDrawTextXY(700, 1300, "Division:");
		pgSetTextJustify(pgLEFT, pgCENTER);
		pgDrawTextXY(800, 100, PlanName);
		pgDrawTextXY(800, 300, Roster->TeamName);
		pgDrawTextXY(800, 500, Roster->Nickname);
		pgDrawTextXY(800, 700, Roster->Abbr);
		pgDrawTextXY(800, 900, Roster->CoachName);
		pgDrawTextXY(800, 1100, Roster->LeagueName);
		if (Roster->ConfName) 
		{
			pgDrawTextXY(800, 1300, Roster->ConfName);
			pgDrawTextXY(800, 1500, Roster->DivName);
		}
		else
			pgDrawTextXY(800, 1300, Roster->DivName);
		pgSetTextJustify(pgCENTER, pgCENTER);
		pgSetTextScaling(2, 1, 2, 1);
		char audibleStr[40];
		pgDrawTextXY(999, 2099, "Audibles");
		strcpy(audibleStr, UpStr);
		strcat(audibleStr, " - ");
		strcat(audibleStr, AudibleText[!OffFlag][Audible[0]]);
		pgDrawTextXY(999, 2199, audibleStr);
		strcpy(audibleStr, LeftStr);
		strcat(audibleStr, " - ");
		strcat(audibleStr, AudibleText[!OffFlag][Audible[2]]);
		pgDrawTextXY(599, 2299, audibleStr);
		strcpy(audibleStr, RightStr);
		strcat(audibleStr, " - ");
		strcat(audibleStr, AudibleText[!OffFlag][Audible[3]]);
		pgDrawTextXY(1399, 2299, audibleStr);
		strcpy(audibleStr, DownStr);
		strcat(audibleStr, " - ");
		strcat(audibleStr, AudibleText[!OffFlag][Audible[1]]);
		pgDrawTextXY(999, 2399, audibleStr);
		pgDrawTextXY(999, 2929, "Special Plays");
		short saveDiag = Printer->Diag, saveText = Printer->Text;
		Printer->Diag = PRN_NODIAG;
		Printer->Text = PRN_NOTEXT;
		for (drawPlayNum = 0; drawPlayNum < 11; drawPlayNum++) 
		{
			if (drawPlayNum == 7)
				drawPlayNum++;
			strcpy(tempPlay->Name(), "Stock");
			if (GetPlay(playPtr, specNum[drawPlayNum])) 
			{
				if (inFile.Open(playPtr->Cust.PlayName)) 
				{
					if (inFile.FindFirstBlock(PlayBlockStr))
						inFile.GetFile(tempPlay->Name());
 					inFile.Close();
				}
			}
			tempPlay->Special(specNum[drawPlayNum] - 63);
			tempPlay->UserCat((unsigned char*) &offFlag);
			tempPlay->Print(drawPlayNum + 64);
		}
		Printer->Diag = saveDiag;
		Printer->Text = saveText;
		if (!Printer->PrintFile()) 
		{
			delete tempPlay;
			delete playPtr;
			RestoreDir(saveDir, saveDisk);
			return;
		}
	}
	drawPlayNum = startPlayNum;
	for (planPage = 0; (planPage < totalPages) && NumNormPlays; planPage++) 
	{
		Printer->OpenPrnFile(PlanName, totalPages + titleFlag,
					planPage + 1 + titleFlag);
		Printer->UpdateStatus(this);
		if ((Printer->Diag == PRN_NODIAG) && (Printer->Text == PRN_NOTEXT)) 
		{
			pgSetTextStyle(pgPRINT);
			pgSetTextJustify(pgCENTER, pgCENTER);
			pgSetTextScaling(2, 1, 2, 1);
			pgSetColor(1);
			pgDrawTextXY(999, 49, "Regular Plays");
		}
		for (pagePlayNum = 0; pagePlayNum < playsPerPage; pagePlayNum++, drawPlayNum++) 
		{
			while (!SlotFilled(drawPlayNum) &&
						((Printer->Diag == PRN_FULL) ||
						(Printer->Text != PRN_NOTEXT)) &&
						(drawPlayNum < 64))
				drawPlayNum++;
			if (drawPlayNum == 64) // End of printing.
				break;
			if (GetPlay(playPtr, drawPlayNum)) 
			{
				if (playPtr->Info.StockFlag) 
				{
					if (inFile.Open(StockPlays->StockDatFile)) 
					{
						inFile.JumpTo(playPtr->Stock.Offset);
						tempPlay->Load(inFile, playPtr->Stock.Size, TRUE);
						inFile.Close();
						*tempPlay->Name() = 0;
						strncat(tempPlay->Name(),
									playPtr->Stock.PlayName, 8);
						if ((Printer->Diag == PRN_FULL) ||
									(Printer->Text != PRN_NOTEXT))
							tempPlay->InitTeam();
						tempPlay->Print(pagePlayNum, drawPlayNum);
						tempPlay->Erase();
					}
				}
				else if (inFile.Open(playPtr->Cust.PlayName)) 
				{
					if (inFile.FindFirstBlock(PlayBlockStr)) 
					{
						tempPlay->Load(inFile, inFile.BlockSize, TRUE);
						inFile.GetFile(tempPlay->Name());
						if ((Printer->Diag == PRN_FULL) ||
									(Printer->Text != PRN_NOTEXT))
							tempPlay->InitTeam();
						tempPlay->Print(pagePlayNum, drawPlayNum);
						tempPlay->Erase();
					}
 					inFile.Close();
				}
			}
			else 
			{
				strcpy(tempPlay->Name(), "Empty");
				tempPlay->Print(pagePlayNum, drawPlayNum);
			}
		}
		strcpy(footerStr, PlanName);
		strcat(footerStr, "    Page ");
		itoa(planPage + 1, footerStr + strlen(footerStr), 10);
		strcat(footerStr, " of ");
		itoa(totalPages, footerStr + strlen(footerStr), 10);
		pgSetTextJustify(pgCENTER, pgCENTER);
		pgSetTextScaling(2, 1, 2, 1);
		pgSetColor(1);
		pgDrawTextXY(999, 4049, footerStr);
		if (!Printer->PrintFile())
			planPage = totalPages;
	}
	delete tempPlay;
	delete playPtr;
	RestoreDir(saveDir, saveDisk);
}
#else
// JDW - 96-08-15 - Rewrote this function
void D_PLAN::Print(short titleFlag)
{
	TEXTMETRIC	sTM;
	long		xFooter, yFooter;
	int			hPrinterRes, vPrinterRes;
	int			lCol, rCol;
	SIZE		sSizeStr;

	if (!titleFlag && !NumNormPlays)
		return;
	short drawPlayNum, planPage, totalPages, pagePlayNum;
	short startPlayNum = 0;
	InputFileHandler inFile;
	PlayInPlan *playPtr;
	D_PLAY *tempPlay;
	char saveDisk;
	char saveDir[MAXPATH + 1];
	char footerStr[80];
	short playsPerPage;
	if (Printer->Text == IDC_RADIO_PRINT_NONE2)
	{
		if (Printer->Diag == IDC_RADIO_PRINT_FULL_SIZE)
			playsPerPage = 2;
		else if (Printer->Diag == IDC_RADIO_PRINT_MINI)
			playsPerPage = 32;
		else
			playsPerPage = 64;
	}
	else 
	{
		if (Printer->Diag == IDC_RADIO_PRINT_FULL_SIZE)
			playsPerPage = 1;
		else
			playsPerPage = 2;
	}
	totalPages = 64 / playsPerPage;
	if (playsPerPage == 32) 
	{
		totalPages = 0;
		for (drawPlayNum = 32; drawPlayNum < 64; drawPlayNum++)
		{
			if (SlotFilled(drawPlayNum)) 
			{
				startPlayNum = 32;
				totalPages++;
				break;
			}
		}
		for (drawPlayNum = 0; drawPlayNum < 32; drawPlayNum++)
		{
			if (SlotFilled(drawPlayNum)) 
			{
				startPlayNum = 0;
				totalPages++;
				break;
			}
		}
	}
	if (playsPerPage == 2)
		totalPages = (NumNormPlays + 1) / 2;
	if (playsPerPage == 1)
		totalPages = NumNormPlays;
	SaveDir(saveDir, &saveDisk);
	RestoreDir(peDirStr, peDisk);
	playPtr = new PlayInPlan;
	tempPlay = new D_PLAY;
#if 0
	if (!Printer->Type) 
	{
		char fileBuf[80];
		char bufLen;
		OutputFileHandler outFile;
		outFile.AddToFile("PRINT.OUT");
		outFile.Write("\x0d\x0a", 2);
		outFile.Write(PlanName, strlen(PlanName));
		outFile.Write("\x0d\x0a", 2);
		for (drawPlayNum = 0; drawPlayNum < NUM_NORM_PLAYS; drawPlayNum++) 
		{
			if ((drawPlayNum < 36) && !(drawPlayNum % 4))
				outFile.Write(" ", 1);
			PlayNumToRowColStr(drawPlayNum, fileBuf);
			strcat(fileBuf, " - ");
			if (GetPlay(playPtr, drawPlayNum)) 
			{
				if (playPtr->Info.StockFlag)
					/*_f*/ strncat(fileBuf, playPtr->Info.PlayName, 8);
				else if (inFile.Open(playPtr->Cust.PlayName)) 
				{
					if (inFile.FindFirstBlock(PlayBlockStr))
						playPtr->GetPlayName(fileBuf + strlen(fileBuf));
					else
						/*_f*/ strcat(fileBuf, "Empty");
 					inFile.Close();
				}
				else
					/*_f*/ strcat(fileBuf, "Empty");
			}
			else
				/*_f*/ strcat(fileBuf, "Empty");
			bufLen = strlen(fileBuf);
			outFile.Write(fileBuf, bufLen);
			bufLen = 20 - bufLen;
			while (bufLen--)
				outFile.Write(" ", 1);
			if ((drawPlayNum % 4) == 3)
				outFile.Write("\x0d\x0a", 2);
		}
		outFile.Close();
		delete tempPlay;
		delete playPtr;
		RestoreDir(saveDir, saveDisk);
		return;
	}
#endif
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

		strcpy(szCat, "Game Plan:");
		GetTextExtentPoint32(Printer->GetPrinterDC(), szCat, strlen(szCat), &sSizeStr);
		TextOut( Printer->GetPrinterDC(), lCol - sSizeStr.cx, (Printer->GetCurLine() * Printer->GetLineHeight()), (PCHAR) szCat, strlen((PCHAR) szCat));
		sprintf(szCat, "%s",PlanName);
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
#if 0	// CC doesn't have this completed yet.
		short saveDiag = Printer->Diag, saveText = Printer->Text;
		Printer->Diag = IDC_RADIO_PRINT_NONE;
		Printer->Text = IDC_RADIO_PRINT_NONE2;
		for (drawPlayNum = 0; drawPlayNum < 11; drawPlayNum++) 
		{
			if (drawPlayNum == 7)
				drawPlayNum++;
			strcpy(tempPlay->Name(), "Stock");
			if (GetPlay(playPtr, specNum[drawPlayNum])) 
			{
				if (inFile.Open(playPtr->Cust.PlayName)) 
				{
					if (inFile.FindFirstBlock(PlayBlockStr))
						inFile.GetFile(tempPlay->Name());
 					inFile.Close();
				}
			}
			tempPlay->Special(specNum[drawPlayNum] - 63);
			tempPlay->UserCat((unsigned char*) &offFlag);
			tempPlay->Print(drawPlayNum + 64);
		}
		Printer->Diag = saveDiag;
		Printer->Text = saveText;
#endif
		Printer->SetStandardFont();
		Printer->FormFeed();
		if (!Printer->PrintFile()) 
		{
			delete tempPlay;
			delete playPtr;
			RestoreDir(saveDir, saveDisk);
			return;
		}
	}

	drawPlayNum = startPlayNum;

	for (planPage = 0; (planPage < totalPages) && NumNormPlays; planPage++) 
	{
		Printer->OpenPrnFile(PlanName, totalPages + titleFlag, planPage + 1 + titleFlag);
		Printer->UpdateStatus(this);

		if ((Printer->Diag == IDC_RADIO_PRINT_NONE) && (Printer->Text == IDC_RADIO_PRINT_NONE2)) 
		{
			Printer->LoadAltFont("Arial", 150, 0, FONT_BOLD, FALSE);
			Printer->SetAltFont();
			GetTextExtentPoint32(Printer->GetPrinterDC(), "Regular Plays", strlen("Regular Plays"), &sSizeStr);
			TextOut(Printer->GetPrinterDC(), (Printer->hPrinterRes - sSizeStr.cx)>>1, (Printer->vMarginList - (sSizeStr.cy << 1)), (PCHAR) "Regular Plays", strlen((PCHAR) "Regular Plays"));
			Printer->SetStandardFont();
		}

		for (pagePlayNum = 0; pagePlayNum < playsPerPage; pagePlayNum++, drawPlayNum++) 
		{
			while (!SlotFilled(drawPlayNum) &&
						((Printer->Diag == IDC_RADIO_PRINT_FULL_SIZE) ||
						(Printer->Text != IDC_RADIO_PRINT_NONE2)) &&
						(drawPlayNum < 64))
				drawPlayNum++;
			if (drawPlayNum == 64) // End of printing.
				break;

			if (GetPlay(playPtr, drawPlayNum)) 
			{
				if (playPtr->Info.StockFlag) 
				{
					if (inFile.Open(StockPlays->StockDatFile)) 
					{
						inFile.JumpTo(playPtr->Stock.Offset);
						tempPlay->Load(inFile, playPtr->Stock.Size, TRUE);
						inFile.Close();
						*tempPlay->Name() = 0;
						strncat(tempPlay->Name(), playPtr->Stock.PlayName, 8);
						if ((Printer->Diag == IDC_RADIO_PRINT_FULL_SIZE) || (Printer->Text != IDC_RADIO_PRINT_NONE2))
							tempPlay->InitTeam();
						tempPlay->Print(pagePlayNum, drawPlayNum);
						tempPlay->Erase();
					}
				}
				else if (inFile.Open(playPtr->Cust.PlayName)) 
				{
					if (inFile.FindFirstBlock(PlayBlockStr)) 
					{
						tempPlay->Load(inFile, inFile.BlockSize, TRUE);
						inFile.GetFile(tempPlay->Name());
						if ((Printer->Diag == IDC_RADIO_PRINT_FULL_SIZE) ||
									(Printer->Text != IDC_RADIO_PRINT_NONE2))
							tempPlay->InitTeam();
						tempPlay->Print(pagePlayNum, drawPlayNum);
						tempPlay->Erase();
					}
 					inFile.Close();
				}
			}
			else 
			{
				strcpy(tempPlay->Name(), "Empty");
				tempPlay->Print(pagePlayNum, drawPlayNum);
			}
		}
		strcpy(footerStr, PlanName);
		strcat(footerStr, "    Page ");
		itoa(planPage + 1, footerStr + strlen(footerStr), 10);
		strcat(footerStr, " of ");
		itoa(totalPages, footerStr + strlen(footerStr), 10);
		Printer->LoadAltFont("Arial", 100, 0, FONT_BOLD, FALSE);
		Printer->SetAltFont();
		GetTextMetrics(Printer->GetPrinterDC(), &sTM);
		hPrinterRes = GetDeviceCaps(Printer->GetPrinterDC(), HORZRES);
		vPrinterRes = GetDeviceCaps(Printer->GetPrinterDC(), VERTRES);
		xFooter = sTM.tmAveCharWidth * strlen((PCHAR) footerStr);
		yFooter = sTM.tmHeight + sTM.tmInternalLeading;
		TextOut( Printer->GetPrinterDC(), (hPrinterRes - xFooter) >> 1, vPrinterRes - (yFooter << 1), (PCHAR) footerStr, strlen((PCHAR) footerStr));
		Printer->SetStandardFont();

		if (planPage < (totalPages - 1))	// No need to do this on last page
			Printer->FormFeed();
	}
	delete tempPlay;
	delete playPtr;
	RestoreDir(saveDir, saveDisk);
}
#endif


void D_PLAN::PlayNumToRowColStr(short playNum, char *rowColStr)
{
	if (!rowColStr)
		return;
	short playRow, playCol, i = 0;
	short specNum[] = {2, 10, 4, 9, 1, 5, 6, 0, 3, 7, 8};
	if (playNum > 63) 
	{
		playCol = playNum - 64;
		rowColStr[i++] = 'S';
		rowColStr[i++] = 0;
		strcat(rowColStr, "-");
		if (specNum[playCol] / 10)
			rowColStr[i++] = (specNum[playCol] / 10) + '0';
		rowColStr[i++] = (specNum[playCol] % 10) + '0';
		rowColStr[i] = 0;
	}
	else 
	{
		playRow = (playNum / 4) + 1;
		playCol = (playNum % 4) + 1;
		if (playRow / 10)
			rowColStr[i++] = (playRow / 10) + '0';
		rowColStr[i++] = (playRow % 10) + '0';
		rowColStr[i++] = 0;
		strcat(rowColStr, "-");
		rowColStr[i++] = playCol + '0';
		rowColStr[i] = 0;
	}
}


void D_PLAN::DrawInfoMem()
{
	vm_fill = TRUE;
	vm_fcolor = 9;
	DrawBox();
	if (!PlanActive)
		return;

	char infoStr[40];
	vm_fontselect((void *)MediumTeamFont);
	vm_fgcolor = 2;
	strcpy(infoStr, PlanName);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 6);
	strcpy(infoStr, (OffFlag) ? "Offense" : "Defense");
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 21);
	strcpy(infoStr, "Custom Plays: ");
	itoa(NumCustPlays, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 36);
	strcpy(infoStr, "Stock Plays: ");
	itoa(NumStockPlays, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 51);
	strcpy(infoStr, "Special Plays: ");
	itoa(NumSpecPlays, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 66);
	vm_fgcolor = (OffFlag) ? 1 : 14;
	strcpy(infoStr, (OffFlag) ? "Offense" : "Defense");
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 20);
	vm_fgcolor = 15;
	strcpy(infoStr, PlanName);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 5);
	strcpy(infoStr, "Custom Plays: ");
	itoa(NumCustPlays, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 35);
	strcpy(infoStr, "Stock Plays: ");
	itoa(NumStockPlays, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 50);
	strcpy(infoStr, "Special Plays: ");
	itoa(NumSpecPlays, infoStr + strlen(infoStr), 10);
	vm_hstr(infoStr, CurBox.x + 5, CurBox.y + 65);
}


// PLAN.CPP functions--------------------------------------------

BOOL DrawPlanInfoDisk(char *fileName)
{
	short saveBox = CurBox.num;
	D_PLAN *tempPlan = new D_PLAN;
	InputFileHandler inPlanFile;
	BOOL bValid = FALSE;

	if (inPlanFile.Open(fileName)) 
	{
		if (inPlanFile.FindFirstBlock(PlanInfoBlockStr))
			tempPlan->LoadInfo(inPlanFile);
		else if (inPlanFile.FindFirstBlock(PlanBlockStr))
			tempPlan->Load(inPlanFile);

		inPlanFile.GetFile(tempPlan->Name());
		tempPlan->DrawInfoMem();
		inPlanFile.Close();
		bValid = TRUE;
	}
	delete tempPlan;
	return bValid;
}


void ClearPlanInfo()
{
	vm_fill = TRUE;
	vm_fcolor = 9;
	DrawBox();
}
