#include <stdio.h>
#include "Cmd.h"
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>

extern ControlStatus state;
extern float K[2];
extern float nbar;
extern float ref;
ControlStatus startState = CONTROL_EE;
extern float ik[2];
extern float kp,ki;
extern float kd;


ControlStatus CmdGetStateStart(){
    return startState;
}

char *CmdGet(char * cmd,char *search){
    char *pt = cmd;
    if(strncmp(pt,search,strlen(search)) != 0)
        return NULL;
    pt += strlen(search);
    pt = CmdSkipSpaces(pt);
    return pt;
}


char *CmdSkipSpaces(char *cmd){
    while(*cmd == ' ' || *cmd == '=' || *cmd =='[' || *cmd == ',')
        cmd++;
    return cmd;
}

char *CmdSkipNumbers(char *cmd){
    while(*cmd != ' ' || *cmd != ',')
        cmd++;
    return cmd;
}

void CmdProcess(char *cmd){
    char *pt = cmd;
    char *ptAux;
    float x;
    if (CmdGet(cmd,"stop") || CmdGet(cmd,"STOP"))
        state = CONTROL_STOP;
    if (CmdGet(cmd,"lead") || CmdGet(cmd,"LEAD"))
        state = CONTROL_LEAD;
    if (CmdGet(cmd,"pid") || CmdGet(cmd,"PID"))
        state = CONTROL_PID;
    if (CmdGet(cmd,"ee") || CmdGet(cmd,"EE"))
        state = CONTROL_EE;
    if (CmdGet(cmd,"initEE")){
        state = CONTROL_INIT;
        startState = CONTROL_EE;
    }
    if (CmdGet(cmd,"initLead") ){
        state = CONTROL_INIT;
        startState = CONTROL_LEAD;
    }
    if (CmdGet(cmd,"initPID") ){
        state = CONTROL_INIT;
        startState = CONTROL_PID;
    }
    
    if (CmdGet(cmd,"onEE")){
        state = CONTROL_ON;
        startState = CONTROL_EE;
    }
    if (CmdGet(cmd,"onLead")){
        state = CONTROL_ON;
        startState = CONTROL_LEAD;
    }
    if (CmdGet(cmd,"onPID")){
        state = CONTROL_ON;
        startState = CONTROL_PID;
    }
    pt = CmdGet(cmd,"K");
    if (pt != NULL){
        K[0] = strtof(pt,&ptAux);
        pt = CmdSkipSpaces(ptAux);
        K[1] = strtof(pt,&ptAux);
    }
    pt = CmdGet(cmd,"Nbar");
    if (pt != NULL){
        nbar = strtof(pt,&ptAux);
    }
    pt = CmdGet(cmd,"ref");
    if (pt != NULL){
        ref = strtof(pt,&ptAux);
    }
    pt = CmdGet(cmd,"Kp");
    if (pt != NULL){
        kp = strtof(pt,&ptAux);
    }
    pt = CmdGet(cmd,"Ki");
    if (pt != NULL){
        ki = strtof(pt,&ptAux);
    }
    pt = CmdGet(cmd,"Kd");
    if (pt != NULL){
        kd = strtof(pt,&ptAux);
    }
    //x = strtof(pt,&aux);
    //printf("%f\n",x);
}