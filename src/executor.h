#include"semanticParser.h"
#pragma once

void executeCommand();

void executeCLEAR();
void executeCROSS();
void executeDISTINCT();
void executeEXPORT();
void executeINDEX();
void executeJOIN();
void executeLIST();
void executeLOAD();
void executePRINT();
void executePROJECTION();
void executeRENAME();
void executeSELECTION();
void executeSORT();
void executeSOURCE();
void executeLOADMATRIX();
void executePRINTMATRIX();
void executeCHECKANTISYM();
void executeROTATE();
void executeCROSSTRANSPOSE();
void executeEXPORTMATRIX();
void executeORDERBY();
void executeGROUPBY();
void executeSEARCH();
void executeDELETE();
void executeUPDATE();
void executeINSERT();
// bool semanticParseLOADMATRIX();
// bool syntacticParseLOADMATRIX();


bool evaluateBinOp(int value1, int value2, BinaryOperator binaryOperator);
void printRowCount(int rowCount);