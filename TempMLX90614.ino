#include <Wire.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
void setup() {
Serial.begin(115200);
Serial.println("MLX90614 (GY-906) with ESP32 38-pin board");
// Initialize I2C with ESP32 pins
Wire.begin(21, 22); // SDA, SCL
if (!mlx.begin()) {
  **Serial.println("Error connecting to MLX90614. Check wiring!");**
\*\*while (1);\*\*

}
}
void loop() {
float ambient = mlx.readAmbientTempC();
float object = mlx.readObjectTempC();
Serial.print("Ambient Temp: ");
Serial.print(ambient);
Serial.print(" °C | Object Temp: ");
Serial.print(object);
Serial.println(" °C");
delay(1000);
}
