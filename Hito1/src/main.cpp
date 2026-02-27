#include <Arduino.h>
#include <WiFi.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <ping/ping_sock.h>

const char *ssid = "Redmi Note 14 5G";
const char *password = "daniTestAp";

// Función que se ejecuta cuando recibimos respuesta del Ping
void on_ping_success(esp_ping_handle_t hdl, void *args)
{
  uint32_t elapsed_time, recv_len;
  uint8_t ttl;
  ip_addr_t target_addr;

  esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));

  Serial.printf("Respuesta desde %s: bytes=%u tiempo=%u ms TTL=%u\n",
                ipaddr_ntoa(&target_addr), recv_len, elapsed_time, ttl);
}

// Función que se ejecuta si el ping falla (timeout)
void on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
  Serial.println("Error: Tiempo de espera agotado (Timeout)");
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");

  // --- CONFIGURACIÓN DEL PING ---
  ip_addr_t target_addr;
  struct hostent *host = gethostbyname("www.google.com");
  if (host == NULL)
  {
    Serial.println("Error: No se pudo resolver DNS");
    return;
  }
  target_addr.u_addr.ip4.addr = *(uint32_t *)(host->h_addr_list[0]);
  target_addr.type = IPADDR_TYPE_V4;

  esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
  ping_config.target_addr = target_addr; // Dirección destino
  ping_config.count = 5;                 // Número de pings a enviar

  // Definir los callbacks (qué hacer cuando hay respuesta o error)
  esp_ping_callbacks_t cbs = {
      .on_ping_success = on_ping_success,
      .on_ping_timeout = on_ping_timeout,
      .on_ping_end = [](esp_ping_handle_t hdl, void *args)
      {
        Serial.println("Sesión de ping finalizada.");
      }};

  // Crear la sesión de ping e iniciarla
  esp_ping_handle_t ping_handle;
  esp_ping_new_session(&ping_config, &cbs, &ping_handle);
  esp_ping_start(ping_handle);
}

void loop() {}