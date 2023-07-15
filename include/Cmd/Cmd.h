#ifndef CMD_H
#define CMD_H

#ifdef __cplusplus
extern "C" {
#endif

#define BUFFER_SIZE (20)

typedef enum{
    CONTROL_ERROR,
    CONTROL_IDLE,
    CONTROL_ON,
    CONTROL_INIT,
    CONTROL_LEAD,
    CONTROL_PID,
    CONTROL_EE,
    CONTROL_STOP,
}ControlStatus;

char *CmdGet(char *cmd,char *search);
char *CmdSkipSpaces(char *cmd);
void CmdProcess(char *cmd);
char *CmdSkipNumbers(char *cmd);
ControlStatus CmdGetStateStart();


#ifdef __cplusplus
}
#endif


#endif