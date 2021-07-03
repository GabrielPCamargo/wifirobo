#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Ultrasonic.h>

//Constante de definição de comandos pelo websocket
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define UP_LEFT 5
#define UP_RIGHT 6
#define DOWN_LEFT 7
#define DOWN_RIGHT 8
#define TURN_LEFT 9
#define TURN_RIGHT 10
#define OBSTACLE_AVOIDER 11
#define STOP 0

//Configurações de motor e direção
#define RIGHT_MOTOR 0
#define LEFT_MOTOR 1

#define FORWARD 1
#define BACKWARD -1

//Definição de pinos
//Ultrassom
#define TRIGGER 13
#define ECHO 12


//Estrutura para motores
struct MOTOR_PINS
{
  int pinIN1;
  int pinIN2;    
};

//Vetor da estrutura de motores
std::vector<MOTOR_PINS> motorPins = 
{
  {33, 32},  //RIGHT_MOTOR
  {26, 25},  //LEFT_MOTOR  
};

const char* ssid     = "MyWiFiCar";
const char* password = "12345678";

AsyncWebServer server(80); //Declara um servidor com porta 80
AsyncWebSocket ws("/ws"); //Declara um metodo para socket cliente

//Criando objeto ultrasonic e definindo as portas digitais
Ultrasonic UltrasonicSensor(TRIGGER, ECHO);

//Página root do HTML do servidor
const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
      body {
        height: 100vh;
      }

     .flex {
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100%;
     }
    .arrows {
      font-size:50px;
      color:red;
    }
    .circularArrows {
      font-size:50px;
      color:blue;
    }
    tr {
      height: 20px;
      width: 100%;
    }
    .alternative tr{
      height: 20px;
      width: 25px;
    } 

    td {
      background-color:black;
      border-radius:25%;
      box-shadow: 5px 5px #888888;
      width: 60px;
    }
    td:active {
      transform: translate(5px,5px);
      box-shadow: none; 
    }

    .noselect {
      -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
         -khtml-user-select: none; /* Konqueror HTML */
           -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
                user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
    }
    </style>
  </head>
  <body class="noselect" align="center" style="background-color:white">

    <div class=flex>
      <table id="mainTable" style="display: inline-block;margin:auto;table-layout:fixed;text-align: center">
        <tr>
          <td ontouchstart='onTouchStartAndEnd("5")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows" >&#11017;</span></td>
          <td ontouchstart='onTouchStartAndEnd("1")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows" >&#8679;</span></td>
          <td ontouchstart='onTouchStartAndEnd("6")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows" >&#11016;</span></td>
        </tr>
        
        <tr>
          <td ontouchstart='onTouchStartAndEnd("3")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows" >&#8678;</span></td>
          <td></td>    
          <td ontouchstart='onTouchStartAndEnd("4")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows" >&#8680;</span></td>
        </tr>
        
        <tr>
          <td ontouchstart='onTouchStartAndEnd("7")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows" >&#11019;</span></td>
          <td ontouchstart='onTouchStartAndEnd("2")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows" >&#8681;</span></td>
          <td ontouchstart='onTouchStartAndEnd("8")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows" >&#11018;</span></td>
        </tr>
      
        <tr>
          <td ontouchstart='onTouchStartAndEnd("9")' ontouchend='onTouchStartAndEnd("0")'><span class="circularArrows" >&#8634;</span></td>
          <td style="background-color:white;box-shadow:none"></td>
          <td ontouchstart='onTouchStartAndEnd("10")' ontouchend='onTouchStartAndEnd("0")'><span class="circularArrows" >&#8635;</span></td>
        </tr>
      </table>
  
      <table id="mainTable" style=";display:inline-block;margin:auto;table-layout:fixed">
        <tr>
          <th>Alternative buttons</th>
        </tr>
        <tr class="alternative">
          <td ontouchstart='onTouchStartAndEnd("11")' class="arrows">&#8622;</td>
        </tr>
      </table>
    </div>
    
    <script>
      var webSocketUrl = "ws:\/\/" + window.location.hostname + "/ws";
      var websocket;
      
      function initWebSocket() 
      {
        websocket = new WebSocket(webSocketUrl);
        websocket.onopen    = function(event){};
        websocket.onclose   = function(event){setTimeout(initWebSocket, 2000);};
        websocket.onmessage = function(event){};
      }

      function onTouchStartAndEnd(value) 
      {
        websocket.send(value);
      }
          
      window.onload = initWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault()
      });      
    </script>
    
  </body>
</html> 

)HTMLHOMEPAGE";

void handleAvoider() {

  float DistanciaemCM = UltrasonicSensor.convert(UltrasonicSensor.timing(), Ultrasonic::CM);
  Serial.print(DistanciaemCM);
  Serial.println(" cm");

  if (DistanciaemCM <= 40) {// Se a distância lida pelo sensor for menor ou igual que 40 centimetros

    rotateMotor(RIGHT_MOTOR, STOP);
    rotateMotor(LEFT_MOTOR, STOP);
    delay(250);
    // Motor lado esquerdo para trás
    rotateMotor(LEFT_MOTOR, BACKWARD);

    // Motor lado direito para trás
    rotateMotor(RIGHT_MOTOR, BACKWARD);

    delay(700);// Tempo que ficará indo para trás
    rotateMotor(RIGHT_MOTOR, STOP);
    rotateMotor(LEFT_MOTOR, STOP);
    delay(250);

    // Motor lado esquerdo para frente
    rotateMotor(LEFT_MOTOR, FORWARD);
    // Motor lado direito para trás
    rotateMotor(RIGHT_MOTOR, BACKWARD);

    delay(250);// Tempo que ficará indo para o lado direito
  
  }else{

          // Motor lado esquerdo para frente
    rotateMotor(RIGHT_MOTOR, FORWARD);

    // Motor lado direito para frente
    rotateMotor(LEFT_MOTOR, FORWARD);
  }
}

//Gira o motor de acordo com o comando - frente, atrás ou parar
void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);    
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);     
  }
  else
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);       
  }
}

//Movimenta o carro de acordo com o valor recebido pelo websocket
void processCarMovement(String inputValue)
{
  Serial.printf("Got value as %s %d\n", inputValue.c_str(), inputValue.toInt());
  boolean keepAvoiding =  inputValue.toInt() == OBSTACLE_AVOIDER; 
  switch(inputValue.toInt())
  {

    case UP:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);                
      break;
  
    case DOWN:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);
       
      break;
  
    case LEFT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, STOP);  
      break;
  
    case RIGHT:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, FORWARD);
      break;
  
    case UP_LEFT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case UP_RIGHT:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);  
      break;
  
    case DOWN_LEFT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);   
      break;
  
    case DOWN_RIGHT:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);   
      break;
  
    case TURN_LEFT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case TURN_RIGHT:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);   
      break;
  
    case STOP:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);   
      break;

    case OBSTACLE_AVOIDER:
      while(keepAvoiding){
        handleAvoider();
        delay(250);
      }
      break;
    
  
    default:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);   
      break;
  }
}

void handleRoot(AsyncWebServerRequest *request) //Para a raiz do servidor executa o HTML
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request)  //Para página não encontrada, escreve "arquivo não encontrada"
{
    request->send(404, "text/plain", "Arquivo não encontrado");
}


//Seleciona o tipo de evento do websocket e executa de acordo.
void onWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len){                      
  switch (type) 
  {
    case WS_EVT_CONNECT: //Caso conecte
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      //client->text(getRelayPinsStatusJson(ALL_RELAY_PINS_INDEX));
      break;
    case WS_EVT_DISCONNECT: //Caso desconecte
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      processCarMovement("0");
      break;
    case WS_EVT_DATA: //Caso envie dados
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
      {
        std::string myData = "";
        myData.assign((char *)data, len);
        processCarMovement(myData.c_str());       
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void setUpPinModes() //Configura os pinos dos motores
{
  for (int i = 0; i < motorPins.size(); i++)
  {
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);  
    rotateMotor(i, STOP);  
  }
}


void setup(void) 
{
  setUpPinModes();
  Serial.begin(115200);

  WiFi.softAP(ssid, password); //Cria uma rede wifi
  IPAddress IP = WiFi.softAPIP(); //Descobre o IP
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot); //Caso o cliente entre na rota raiz, executa handleRoot
  server.onNotFound(handleNotFound);//Caso o cliente entre fora da rota raiz, executa handleNotFound
  
  ws.onEvent(onWebSocketEvent); //Quando houver uma mudança no wesocket executa a função de callback onWebSocketEvent
  server.addHandler(&ws); //Adiciona websocket ao servidor web
  
  server.begin(); //Inicia o servidor wev
  Serial.println("HTTP server started");
}

void loop() 
{
  ws.cleanupClients(); //Limpa os clientes, caso exceda o número máximo
}
