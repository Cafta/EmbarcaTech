#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <ArduinoJson.h>  // A mensagem criptografada será enviada como JSON
extern "C" {
  #include "OLED.h"  // Biblioteca personalizada para OLED, que contém as funções de inicialização e desenho
}

const char* ssid = "CAFTA"; //"CaftaS9";
const char* password = "hahehihohu";
const char* mqtt_server = "mqtt.eclipseprojects.io"; // servidor MQTT
const char* client_id = "bitdog2"; // ID do cliente MQTT
const char* mqtt_user = "aluno"; // Usuário do MQTT
const char* mqtt_password = "senha123"; // Senha do MQTT  
const char* topico = "escola/sala1/temperatura";
const char* topico2 = "escola/sala1/temperatura_criptografada";

WiFiClient espClient;
PubSubClient client(espClient);

void xor_decrypt(const uint8_t *imput, uint8_t *output, size_t length, uint8_t key);
void xor_encrypt(const uint8_t *imput, uint8_t *output, size_t length, uint8_t key);
void callback(char* topic, byte* message, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(115200);
  delay(100); // Aguarda a inicialização do Serial
  oled_init(); // Inicializa o OLED
  WiFi.begin(ssid, password);

  //clear_ssd1306_i2c(); // Limpa o OLED
  print_oled(0, 16, "  Connecting (WiFi)  ", false); // Exibe mensagem de conexão no OLED

  // Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  print_oled(0, 24, "  Conectado ao WiFi  ", false); // Exibe mensagem de conexão no OLED
  Serial.println("\nConectado ao WiFi");
}

bool pos_reconnection = false;

void loop() {
  if (!client.connected()) {
    reconnect();
    pos_reconnection = true; // Define a flag de reconexão
  }
  if (pos_reconnection) {
    pos_reconnection = false; // Reseta a flag de reconexão
    print_background(); // Exibe o fundo no OLED
  }
  client.loop();
}

void xor_encrypt(const uint8_t *imput, uint8_t *output, size_t length, uint8_t key) {
  for (size_t i = 0; i < length; i++) {
    output[i] = imput[i] ^ key;
  }
}

void xor_decrypt(const uint8_t *imput, uint8_t *output, size_t length, uint8_t key) {
  xor_encrypt(imput, output, length, key); // XOR é simétrico, então a mesma função serve para criptografar e descriptografar
} 

void reconnect() {
  clear_ssd1306_i2c(); // Limpa o OLED
  print_oled(0, 40, " MQTT Connecting...  ", false); // Exibe mensagem de conexão no OLED
  while (!client.connected()) {
    Serial.print("\nTentando conectar ao MQTT...");
    if (client.connect(client_id, mqtt_user, mqtt_password)) {
    // if (client.connect(client_id)) {
      Serial.println("Conectado!");
      print_background(); // Limpa o OLED e exibe o fundo
      client.subscribe(topico);
      client.subscribe(topico2);
    } else {
      Serial.print("Falhou, rc=");
      print_oled(0, 40, "  MQTT  ->  Fail     ", false); // Exibe mensagem no OLED
      Serial.print(client.state());
      delay(2000);
      print_oled(0, 40, " MQTT Connecting...  ", false); // Exibe mensagem de conexão no OLED
    }
  }
  delay(500); // Aguarda um segundo após a conexão
  print_background(); // Limpa o OLED e exibe o fundo
}

void callback(char* topic, byte* message, unsigned int length) {
  float valor = 0.0;

  if (strcmp(topic, topico2) == 0) {
    // Mensagem criptografada
    uint8_t decrypted[length];
    xor_decrypt(message, decrypted, length, 42);

    char json[length + 1];
    for (unsigned int i = 0; i < length; i++) {
      json[i] = (char)decrypted[i];
    }
    json[length] = '\0';

    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, json) == DeserializationError::Ok) {
      valor = doc["valor"] | 0.0;
    }

    char linha6[22];
    snprintf(linha6, sizeof(linha6), "     %.1f oC", valor);
    print_oled(0, 6 * 8, linha6, false); // linha 6 muda

  } else if (strcmp(topic, topico) == 0) {
    // Mensagem sem criptografia
    char msg[length + 1];
    for (unsigned int i = 0; i < length; i++) {
      msg[i] = (char)message[i];
    }
    msg[length] = '\0';

    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, msg) == DeserializationError::Ok) {
      valor = doc["valor"] | 0.0;
    }

    char linha3[22];
    snprintf(linha3, sizeof(linha3), "     %.1f oC", valor);
    print_oled(0, 3 * 8, linha3, false); // linha 3 muda
  }
}

