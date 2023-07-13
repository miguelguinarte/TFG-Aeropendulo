/* 
  PRUEBA DE CONCEPTO DE UN ALGORITMO DE CONTROL PARA EL ESP32
  
  Autor: Diego García Pérez
  Fecha: 21/06/22
  
  Descripción: 
    El determinismo temporal del algoritmo temporal se garantiza mediante una interrupción por desbordamiento
    de timer, la cual libera un semaforo asociado a la rutina de control.

    Para la transimisión de datos contamos con otra tarea, también sincronizada mediante un semáforo, 
    que envía los datos del control tras N_TICKS__TRANSMISSION ejecuciones de la rutina de control. 

    Estas dos tareas se ejecutan de manera independiente de forma que la transmisión de datos no compromete
    el periodo de muestreo de la rutina de control.

  Pruebas realizadas:
    -> Probar una interrupción y semáforo.                    ***
    -> Probar interrupción + semáforo + wifi                  ***
    -> Comprobar determinismo temporal                        *** 
    -> Probar en dos nucleos                                  *** (ojo el loop también afecta, parece que se ejecuta en el núcleo 0)
    -> Interrupción + semáforo + wifi + gestión de arrays     ***   
    -> Probar la compilación separada                         ***
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h> // ESP32Servo library installed by Library Manager
#include <ESC.h> // RC_ESP library installed by Library Manager
#include <aeropendulo.h>
#include <interprete.h>


#include "regStruct.h"
#include <controlDigital.h>


// CONSTANTES DEL CONTROL Y TRANSMISIÓN
#define N_TICKS_TRANSMISSION 100
#define MOTOR1_ESC_PIN 13 //
#define MOTOR2_ESC_PIN 12 //
// Valores de calibración de los ESC configurados con el sofware BLHeliSuite (windows + arduino UNO) 
#define MOTORS_MIN_SPEED 1012 
#define MOTORS_MAX_SPEED 1996
#define MOTORS_MIN_SPEED_CTRL 1200 
#define MOTORS_MAX_SPEED_CTRL 1600
#define SPEED_CTRL 1400
#define MOTORS_ARM_SPEED 500 




// VARIABLES GLOBALES


float volatile _uk[4],_ik[4],_dk,_pk, _ek[4], _yk,_Tm = 0.001; 
float volatile _Kp=7, _Ki = 0 , _Kd = 0, _Ref=0,_vM1_ms,_vM2_ms;
char _msg[100];
char *_msg_rcv;
int _v_ref_ms = SPEED_CTRL;

// Lead + lag compensator
float volatile a[4] = {19.087280700364204, -38.124842511645390, 19.037584953067615, 0};
float volatile b[4] = {1, -1.991951875709832, 0.991952513134534, 0};

// Lead compensator
// float volatile a[4] = {12.863274596182084, -12.848325991189427 0, 0};
// float volatile b[4] = {1, -0.997063142437592,0 0};


float volatile _M1_ms = 1400, _M2_ms = 1400;
float _val_leido;

// Estados máquina de estados
enum _estados { ST_STOP, ST_PID, ST_REG,ST_TEST};
int _estado = ST_STOP;


// Handle para un hardware timer 
hw_timer_t * timer = NULL;

// Mutex para proteger las variables compartidas entre procesos.
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

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
ESC _esc_M1 (MOTOR1_ESC_PIN, 1000, 2000, MOTORS_ARM_SPEED); 
ESC _esc_M2 (MOTOR2_ESC_PIN, 1000, 2000, MOTORS_ARM_SPEED); 

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

          // Configuración control posición
          if (GetValorComando(_msg_rcv, CMD_REF) != NULL)
          {
          
            pt_value = GetValorComando(_msg_rcv, CMD_REF);
            if (pt_value != NULL)
              if (GetValueFloat(&pt_value, &_val_leido) != -1)
                _Ref = _val_leido;

          }

          if (GetValorComando(_msg_rcv, CMD_KP) != NULL)
          {
               
            pt_value = GetValorComando(_msg_rcv, CMD_KP);
            if (pt_value != NULL)
              if (GetValueFloat(&pt_value, &_val_leido) != -1)
                _Kp = _val_leido;

          }
          if (GetValorComando(_msg_rcv, CMD_KI) != NULL)
          {
               
            pt_value = GetValorComando(_msg_rcv, CMD_KI);
            if (pt_value != NULL)
              if (GetValueFloat(&pt_value, &_val_leido) != -1)
                _Ki = _val_leido;

          }

          if (GetValorComando(_msg_rcv, CMD_KD) != NULL)
          {
               
            pt_value = GetValorComando(_msg_rcv, CMD_KD);
            if (pt_value != NULL)
              if (GetValueFloat(&pt_value, &_val_leido) != -1)
                _Kd = _val_leido;

          }

          if (GetValorComando(_msg_rcv, CMD_VREF) != NULL)
          {
               
            pt_value = GetValorComando(_msg_rcv, CMD_VREF);
            if (pt_value != NULL)
              if (GetValueFloat(&pt_value, &_val_leido) != -1)
                _v_ref_ms =  _val_leido;

          }

          if (GetValorComando(_msg_rcv, CMD_CTRL) != NULL)
          {
               
            pt_value = GetValorComando(_msg_rcv, CMD_CTRL);
            if (strstr(pt_value,"REG")!=NULL)
              _estado = ST_REG;
            else if (strstr(pt_value,"PID")!=NULL)
              _estado = ST_PID;
            else
              _estado = ST_STOP;

          }

          if (GetComando(_msg_rcv, CMD_STOP) != NULL)
          {
               
              if(_estado != ST_STOP){
                _estado = ST_STOP;
                delay(1000);
                _esc_M1.speed(1250);
                _esc_M1.speed(1250);
                _esc_M1.stop();
                _esc_M2.stop();
              }
          }


          if (GetComando(_msg_rcv, CMD_START) != NULL)
          {
               
              if(_estado == ST_STOP){
                digitalWrite(LED_BUILTIN, HIGH);
                _esc_M1.arm(); // armado de los motores
                _esc_M2.arm();
                delay(1000); // Espera para el armado
                digitalWrite(LED_BUILTIN, LOW);

                  // the following loop turns on the motor slowly, so get ready
                for (int i=0; i<350; i++){ // run speed from 840 to 1190
                  _esc_M1.speed(MOTORS_MIN_SPEED-200+i); // motor starts up about half way through loop
                  _esc_M2.speed(MOTORS_MIN_SPEED-200+i); // motor starts up about half way through loop
                  delay(10);
                }

                _estado = ST_REG;
              }
          }


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

void tareaControl(void* parameters){

  for (;;)
  {
    xSemaphoreTake( xBinarySemaphoreControl, portMAX_DELAY);
    // Aumentamos contador para la transmisión wifi
    cont_ticks_TM ++;
    //digitalWrite(LED_BUILTIN, HIGH);
    // Ejecutamos el control
    desplazartabla(_ik, 4);
    desplazartabla(_ek, 4);
    desplazartabla(_uk, 4);
    _yk = lecturaPosicionAngular(PIN_POT);
    _ek[0] = _Ref - _yk;
    
    switch (_estado)
    {
    case ST_STOP:
      _uk[0] = 0;
      _ek[0] = 0;
      break;

    case ST_PID:
        // Acción Proporcional
        _pk = _ek[0] * _Kp;
        // Acción integral
        _ik[0] = 0.5*_Kp * _Ki *_Tm * (_ek[0] + _ek[1]) + _ik[1];

        // anti-Windup (pendiente)
        _ik[0] = Clip(_ik[0], 300, -300); // Limitada a mano de momento

        // Acción diferencial
        _dk = (_Kp * _Kd / _Tm) * (_ek[0] - _ek[1]);

        _uk[0] = _pk + _ik[0] + _dk;
        break;
    case ST_REG:
      //_ek[0] = 0;
      _uk[0] = productoescalar(a, _ek, 4) - productoescalar(b + 1, _uk + 1, 3);

      break;
    default:
      break;
    }
    
    // Escritura de la acción de control en los motores
    _vM1_ms = _v_ref_ms + _uk[0];
    _vM1_ms = Clip(_vM1_ms,MOTORS_MAX_SPEED_CTRL,MOTORS_MIN_SPEED_CTRL);
    
    _vM2_ms = _v_ref_ms - _uk[0];
    _vM2_ms = Clip(_vM2_ms,MOTORS_MAX_SPEED_CTRL,MOTORS_MIN_SPEED_CTRL);

    if (_estado != ST_STOP){
      _esc_M1.speed((int) _vM1_ms);
      _esc_M2.speed((int) _vM2_ms);
    }

    //digitalWrite(LED_BUILTIN, LOW);
    // Al alcanzar contador N_TICKS_TRANSMISSION lazamos transmisión
    if (cont_ticks_TM >= N_TICKS_TRANSMISSION)
    {
      cont_ticks_TM = 0;
      xSemaphoreGive(xBinarySemaphoreTransmision);
    }
  }
}


void tareaTransmision(void* parameters){
  for (;;){
    xSemaphoreTake(xBinarySemaphoreTransmision, portMAX_DELAY);

    sprintf(_msg, "%d,%3.1f,%2.3f,%2.3f,%2.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.0f",_estado, _Ref,_Kp,_Ki,_Kd,_uk[0], _ek[0], _yk,_vM1_ms,_vM2_ms,_v_ref_ms);
    //Serial.println(_msg);
    webSocket.broadcastTXT(_msg);
    }

}



void setup() {
  Serial.begin(115200);
  delay(1000);
  // Configuramos el LED 
  pinMode(LED_BUILTIN, OUTPUT);

  // INICIALIZACIÓN DEL CONTROL
  VaciarTabla(_ek, 4);
  VaciarTabla(_ik, 4);
  VaciarTabla(_uk, 4);

  // motores
  pinMode(MOTOR1_ESC_PIN, OUTPUT);
  pinMode(MOTOR2_ESC_PIN, OUTPUT);

  
  // ARMADO PROVISIONAL
   digitalWrite(LED_BUILTIN, HIGH);
  
   _esc_M1.arm(); // armado de los motores
   _esc_M2.arm();
   delay(10000); // Espera para el armado
   digitalWrite(LED_BUILTIN, LOW);

     // the following loop turns on the motor slowly, so get ready
     for (int i=0; i<350; i++){ // run speed from 840 to 1190
     _esc_M1.speed(MOTORS_MIN_SPEED-200+i); // motor starts up about half way through loop
     _esc_M2.speed(MOTORS_MIN_SPEED-200+i); // motor starts up about half way through loop
      delay(10);
     }

  

  //inicializarReguladores();

  // Creamos el semáforo
  xBinarySemaphoreControl = xSemaphoreCreateBinary();
  xBinarySemaphoreTransmision = xSemaphoreCreateBinary();

  // Configuración de la interrupción por timner
  timer = timerBegin(1, 80, true); // Iniciamos el reloj con un pre-scaler de 80 --> F_conteo= 1Mz (el reloj es de 80MHz)
  timerAttachInterrupt(timer, &onTimer_1s, true);  // Indicamos la ISR
  timerAlarmWrite(timer, 1000, true);     // Configuramos el desbordamiento a 1s 
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
        tareaControl,           /* Task function. */
        "tareaControl",        /* name of task. */
        1000,                    /* Stack size of task */
        NULL,                     /* parameter of the task */
        6,                        /* priority of the task */
        NULL,
        1); 


  // PARA QUE NO INTERRUPA LA EJECUCIÓN DE LA RUTINA DE CONTROL HAY QUE TENER EN CUENTA QUE EL LOOP SE EJECUTA EN NÚCLEO 0, PARA ELLO DEJAR EL CONTROL EN EL NÚCLEO 1
  xTaskCreatePinnedToCore(
        tareaTransmision,           /* Task function. */
        "tareaTransmision",        /* name of task. */
        10000,                    /* Stack size of task */
        NULL,                     /* parameter of the task */
        2,                        /* priority of the task */
        NULL,
        0); 


}

void loop() {
  webSocket.loop();
  
}