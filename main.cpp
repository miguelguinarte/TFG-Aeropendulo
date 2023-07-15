#include <stdio.h>
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TaskMgr.h"
#include "Accelerometer.h"
#include "Gyro.h"
#include <unistd.h> 
#include "I2c.h"
#include "driver/i2c.h"
#include "driver/timer.h"
#include "esp_err.h"
#include "Timer.h"
#include "Wire.h"
#include "Esc.h"
#include "freertos/semphr.h"
#include "Potenciometer.h"
#include "math.h"
#include "MedianFilter.h"
#include "Cmd.h"
#include "rtc_wdt.h"
#include <esp_task_wdt.h>
#include "driver/uart.h"
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "WebSocketsServer.h"



// Transmission constant
#define N_TICKS_TRANSMISSION 50

// Global variables
ControlStatus state = CONTROL_IDLE;
static float angleAcc;
static float angleGyro;
static float angleCompFilter = 0;
static float angularVel;
int volatile counter = 0;
float ref = 0;
float ek[2] = {0,0};
float yk[6] ;
float ykpot = 0;
float ik[2] = {0,0};
float kp = 7, ki = 0, kd = 0, dk,pk,ts=3e-3; // PID variables
int uk[2] = {0,0};
int uk1 = 0;
int uk2 = 0;
float x1 = 0;
float x2[6];
float w_filt = 0;
float nbar = 2.3;
float K[2] = {2.66,1.93};
float vel_filt[2] = {0,0};
char _msg[100];
char *_msg_rcv;


// Handle for hardware timer 
hw_timer_t * timer = NULL;


// SEMÁFOROS
// Semáforo binario para sincronizar la interrupción por timer y la tarea de control.
SemaphoreHandle_t xBinarySemaphoreControl;
SemaphoreHandle_t xBinarySemaphoreTransmision;

int volatile cont_ticks_TM = 0;


// Variables transmisión WiFi
const char* ssid     = "ESPWebsocket";
const char* password = "diegodie";
;

WebSocketsServer webSocket = WebSocketsServer(81);

// Variables motores
ESC escM1 (MOTOR1_ESC_PIN, 1000, 2000, MOTORS_ARM_SPEED); 
ESC escM2 (MOTOR2_ESC_PIN, 1000, 2000, MOTORS_ARM_SPEED); 

//  FUNCIONES
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    char *pt_value = NULL;
    switch(type) {
        
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

				        // send message to client
			      	  webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
          
            _msg_rcv = (char *)payload;
            Serial.printf("get Text: %s\n", _msg_rcv);
            // Procesar la cadena
            CmdProcess(_msg_rcv);
        break;
        case WStype_BIN:  break;
		    case WStype_ERROR:	 break;	
		    case WStype_FRAGMENT_TEXT_START:  break;
		    case WStype_FRAGMENT_BIN_START:  break;
		    case WStype_FRAGMENT: break;
        case WStype_PING:  break;
        case WStype_PONG:  break;
		    case WStype_FRAGMENT_FIN:
			  break;
    }

}




void IRAM_ATTR onTimer_1s(){
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR( xBinarySemaphoreControl, &xHigherPriorityTaskWoken);
}

// Callbacks TAREAS

void controlTask(void* parameters){
  for (;;){
    xSemaphoreTake( xBinarySemaphoreControl, portMAX_DELAY);
    switch(state){
      case(CONTROL_ON):
        // Motots arm
        digitalWrite(LED_BUILTIN, HIGH);
        escM1.arm();
        escM2.arm();
        delay(10000); // Waiting for the arm
        digitalWrite(LED_BUILTIN, LOW);
        state = CONTROL_INIT;
        break;
      case(CONTROL_INIT):
          // the following loop turns on the motor slowly, so get ready
          for (int i=0; i<350; i++){ // run speed from 840 to 1190
          escM1.speed(MOTORS_MIN_SPEED-200+i); // motor starts up about half way through loop
          escM2.speed(MOTORS_MIN_SPEED-200+i); // motor starts up about half way through loop
          delay(10);
          }
          state = CmdGetStateStart();
          break;
      case(CONTROL_LEAD):
          angleAcc = AccelerometerGetAngle();
          angularVel = GyroGetAngularVel();
          angleCompFilter = (0.98)*(angleCompFilter + (angularVel - ANGULAR_VEL_OFFSET) * DT) + (0.02)*angleAcc;
          //yk = PotenciometerRead(PIN_POT);
          yk[0] = angleCompFilter;
          ek[0] = ref - yk[0];
          uk[0] = (int) (0.9704 * uk[1] + 24.65* ek[0] - 24.61*ek[1]);
          uk1 = constrain(SPEED_CTRL+uk[0],MOTORS_MIN_SPEED_CTRL,MOTORS_MAX_SPEED_CTRL);
          uk2 = constrain(SPEED_CTRL-uk[0],MOTORS_MIN_SPEED_CTRL,MOTORS_MAX_SPEED_CTRL);
          escM1.speed(uk1);
          escM2.speed(uk2);
          //printf("%.2f,%.2f,%.2f\n",yk[0],ref,ek[0]);
          printf("estoy en lead\n");
          break;
      case(CONTROL_PID):
          //angleAcc = AccelerometerGetAngle();
          //angularVel = GyroGetAngularVel();
          //angleCompFilter = (0.98)*(angleCompFilter + (angularVel - ANGULAR_VEL_OFFSET) * DT) + (0.02)*angleAcc;
          //yk = PotenciometerRead(PIN_POT);
          yk[0] = PotenciometerRead(PIN_POT);
          ek[0] = ref - yk[0];
          // Acción Proporcional
          pk = ek[0] * kp;
          // Acción integral
          ik[0] = 0.5*kp * ki * ts * (ek[0] + ek[1]) + ik[1];

          // anti-Windup
          ik[0] = constrain(ik[0], -200, 200); // Limitada a mano de momento

          // Acción diferencial
          dk = (kp * kd / ts) * (ek[0] - ek[1]);

          uk[0] = pk + ik[0] + dk;
          uk1 = constrain(SPEED_CTRL+uk[0],MOTORS_MIN_SPEED_CTRL,MOTORS_MAX_SPEED_CTRL);
          uk2 = constrain(SPEED_CTRL-uk[0],MOTORS_MIN_SPEED_CTRL,MOTORS_MAX_SPEED_CTRL);
          escM1.speed(uk1);
          escM2.speed(uk2);
          printf("%.2f,%.2f,%.2f\n",kp,ki,kd);
        break;
      case(CONTROL_EE):
          //ref = 20*sin(PI/10*(counter/3.33e2));
          angleAcc = AccelerometerGetAngle();
          angularVel = GyroGetAngularVel();
          angleCompFilter = (0.98)*(angleCompFilter + (angularVel - ANGULAR_VEL_OFFSET) * DT) + (0.02)*angleAcc;
          yk[0] = PotenciometerRead(PIN_POT);
          // Adaptive sensing 
          if(yk[0] > -45 && yk[0] < 34)
              yk[0] = angleCompFilter;
          else if(yk[0] >= 34) 
              yk[0] +=  6.5;
          else if(yk[0] <= -45)
              yk[0] -=  6.5; 
          // Position filtering for differenciating
          for(int i = 0; i < 6 ; i++){
                vel_filt[0] += yk[i];
          }
          vel_filt[0] = vel_filt[0]/6;
          // Velocity filtering
          x2[0] = angularVel;
          for(int i = 0; i < 6 ; i++){
                w_filt += x2[i];
          }
          w_filt = w_filt / 6;
          if (counter % 8 == 0){  
              // angular vel by derivative
              x1 = (vel_filt[0]-vel_filt[1])/(DT*8);                 
              //x2 = angularVel;
              uk[0] = (int) (-K[1]*yk[0]-K[0]*w_filt+nbar*ref);
              uk1 = constrain(SPEED_CTRL+uk[0],MOTORS_MIN_SPEED_CTRL,MOTORS_MAX_SPEED_CTRL);
              uk2 = constrain(SPEED_CTRL-uk[0],MOTORS_MIN_SPEED_CTRL,MOTORS_MAX_SPEED_CTRL);
              escM1.speed(uk1);
              escM2.speed(uk2);
              vel_filt[1] = vel_filt[0];
              //printf("%.2f,%.2f\n",yk[0],x1);
              //printf("%.2f,%.2f,%.2f\n",yk[0],nbar,ref);
            }
          break;
          case(CONTROL_STOP):
              escM1.stop();
              escM2.stop();
              state = CONTROL_IDLE;
              break;
          case(CONTROL_IDLE):
              printf("IDLE\n");
              rtc_wdt_feed();
              break;
          case(CONTROL_ERROR):
          default:
              break; 
    }
    uk[1] = uk[0];
    ek[1] = ek[0];
    ik[1] = ik[0];
    ArrayDisplacement(yk,6);
    ArrayDisplacement(x2,6);
    // Counter update for WiFi transmission
    counter++;
    // Al alcanzar contador N_TICKS_TRANSMISSION lazamos transmisión
    if (counter >= N_TICKS_TRANSMISSION){
      counter = 0;
      xSemaphoreGive(xBinarySemaphoreTransmision);
    }
  }
}


void txTask(void* parameters){
  for (;;){
    xSemaphoreTake(xBinarySemaphoreTransmision, portMAX_DELAY);
    sprintf(_msg, "%3.1f,%2.2f",ref,yk[0]);
    //Serial.println(_msg);
    webSocket.broadcastTXT(_msg);
    }

}



void setup() {
  Wire.begin();
  Wire.setClock(400e3);
  TaskMgrInit();  
  Serial.begin(115200);
  delay(1000);
  // LED config
  gpio_set_direction(GPIO_NUM_25,GPIO_MODE_OUTPUT); 

  // motors config
  gpio_set_direction(MOTOR1_ESC_PIN,GPIO_MODE_OUTPUT);
  gpio_set_direction(MOTOR2_ESC_PIN,GPIO_MODE_OUTPUT);
  

  // Creamos el semáforo
  xBinarySemaphoreControl = xSemaphoreCreateBinary();
  xBinarySemaphoreTransmision = xSemaphoreCreateBinary();

  // Configuración de la interrupción por timner
  timer = timerBegin(1, 80, true); // Iniciamos el reloj con un pre-scaler de 80 --> F_conteo= 1Mz (el reloj es de 80MHz)
  timerAttachInterrupt(timer, &onTimer_1s, true);  // Indicamos la ISR
  timerAlarmWrite(timer, 3000, true);     // Configuramos el desbordamiento a 3ms
  timerAlarmEnable(timer);  

  // Creando el punto de acceso WiFi
  Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password,11);
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());
  delay(40);
  // Iniciando el servidor websocket
  webSocket.begin();
  // Asignado callback asíncrona
  webSocket.onEvent(webSocketEvent);


  /// TAREAS
  // Creamos la tarea de control
  xTaskCreatePinnedToCore(
        controlTask,           /* Task function. */
        "controlTask",        /* name of task. */
        8000,                    /* Stack size of task */
        NULL,                     /* parameter of the task */
        6,                        /* priority of the task */
        NULL,
        1); 


  // PARA QUE NO INTERRUPA LA EJECUCIÓN DE LA RUTINA DE CONTROL HAY QUE TENER EN CUENTA QUE EL LOOP SE EJECUTA EN NÚCLEO 0, PARA ELLO DEJAR EL CONTROL EN EL NÚCLEO 1
  xTaskCreatePinnedToCore(
        txTask,           /* Task function. */
        "txTask",        /* name of task. */
        10000,                    /* Stack size of task */
        NULL,                     /* parameter of the task */
        2,                        /* priority of the task */
        NULL,
        0); 


}

void loop() {
  webSocket.loop();
  
}