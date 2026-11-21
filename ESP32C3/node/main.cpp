/**
  * @file enigmaiot_node.ino
  * @version 0.9.8-modified
  * @date 20/05/2026
  * @author Modified by Kilo
  * @brief Node: flood (water-level + ultrasonic) and rain intensity sensors
  */

#include <Arduino.h>
#include <EnigmaIOTNode.h>
#include <espnow_hal.h>
#include <CayenneLPP.h>
#include <WiFi.h>
#include <Update.h>
#include <driver/adc.h>
#include "esp_wifi.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "soc/adc_channel.h"
#include <ArduinoJson.h>
#include <esp_sleep.h>

constexpr auto RESET_PIN = 2;

// Non-blocking timer for periodic sensor data sending (not used by default)
unsigned long previousMillis = 0;
const unsigned long SENSOR_SEND_INTERVAL = 10000; // 10 seconds

// Pins
#define FLOOD_DETECT_PIN 10        // Digital water presence/level switch (active low assumed)
#define ULTRASONIC_TRIG_PIN 4      // HC-SR04 TRIG
#define ULTRASONIC_ECHO_PIN 3      // HC-SR04 ECHO

// Analog rain sensor (repurpose previous TEMT6000 ADC pin)
#define RAIN_SENSOR_PIN 0          // ADC channel for rain sensor (0)

// Battery voltage sense on ADC (using voltage divider)
#define VOLTAGE_SENSE_PIN 1
#define VOLTAGE_DIVIDER_RATIO 2.0

// Ultrasonic measurement timeout (microseconds)
#define ULTRASONIC_TIMEOUT_US 30000UL // 30 ms -> ~5 meters

// Rain sensor mapping
#define RAIN_MAX_PERCENT 100.0f

// Sensor startup delay (not used for ultrasonic)
#define SENSOR_POWER_ON_DELAY 10

// Sensor data structure
struct SensorData {
    bool floodDetected;      // Channel 2 - digital
    float rainIntensity;     // Channel 3 - 0..100 (arbitrary units)
    float waterDistanceCm;   // Channel 4 - ultrasonic measured distance in cm (or -1 if invalid)
    float batteryVoltage;    // Channel 7 - Battery Volts
    float power;             // Channel 6 - Power estimate
};

// Read ultrasonic distance in centimeters
static float measureUltrasonicCm() {
    // Ensure trigger is LOW
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2);
    // Send 10us pulse
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

    // Read pulse on echo pin (timeout in microseconds)
    unsigned long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, ULTRASONIC_TIMEOUT_US);
    if (duration == 0) {
        // timeout
        return -1.0f;
    }
    // Sound speed ~343 m/s => 0.0343 cm/us. Distance = duration(us) * 0.0343 / 2
    float distance_cm = (duration * 0.0343f) / 2.0f;
    return distance_cm;
}

// Debounced digital read (reads 3 times)
static bool debouncedDigitalRead(uint8_t pin) {
    int reads = 0;
    for (int i = 0; i < 3; ++i) {
        reads += digitalRead(pin);
        delay(5);
    }
    // Majority
    return (reads >= 2); // true if HIGH majority
}

SensorData readSensorPayload() {
    SensorData data;

    // Read flood detect (assume active LOW: 0 -> water present)
    pinMode(FLOOD_DETECT_PIN, INPUT_PULLUP);
    bool floodRaw = !debouncedDigitalRead(FLOOD_DETECT_PIN); // active LOW -> invert
    data.floodDetected = floodRaw;

    // If flood detected, activate ultrasonic measurement to get water level/distance
    if (data.floodDetected) {
        // Configure ultrasonic pins
        pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
        pinMode(ULTRASONIC_ECHO_PIN, INPUT);
        float dist = measureUltrasonicCm();
        data.waterDistanceCm = dist;
        Serial.printf("Ultrasonic distance: %.1f cm\n", dist);
    } else {
        data.waterDistanceCm = -1.0f; // invalid/not measured
    }

    // Read rain sensor (analog), map to 0..100 percent
    int rainRaw = analogRead(RAIN_SENSOR_PIN);
    data.rainIntensity = (rainRaw / 4095.0f) * RAIN_MAX_PERCENT;

    // Read battery voltage from ADC
    int voltageRaw = analogRead(VOLTAGE_SENSE_PIN);
    float adcVoltage = (voltageRaw / 4095.0f) * 3.3f;
    data.batteryVoltage = adcVoltage * VOLTAGE_DIVIDER_RATIO;

    // Approximate power using assumed 80 mA transmission current
    data.power = data.batteryVoltage * 0.08f;

    Serial.printf("Flood: %s, Rain: %.1f%%, Dist: %.1f cm, Volt: %.2f V, Power: %.2f W\n",
                  data.floodDetected ? "YES" : "NO",
                  data.rainIntensity,
                  data.waterDistanceCm,
                  data.batteryVoltage,
                  data.power);

    return data;
}

void buildCayenneLPPMessage(CayenneLPP& msg, const SensorData& data) {
    // Channel mapping
    // 2 = flood (digital)
    // 3 = rain intensity (analog %)
    // 4 = water distance (cm)
    // 6 = power
    // 7 = battery voltage

    // Flood as digital input (0 or 1)
    msg.addDigitalInput(2, data.floodDetected ? 1 : 0);
    // Rain intensity as analog input
    msg.addAnalogInput(3, data.rainIntensity);
    // Water distance as analog (in cm)
    msg.addAnalogInput(4, data.waterDistanceCm);
    // Power and battery
    msg.addPower(6, data.power);
    msg.addVoltage(7, data.batteryVoltage);
}

void sendSensorData() {
    CayenneLPP msg(MAX_DATA_PAYLOAD_SIZE);

    SensorData sensorData = readSensorPayload();
    buildCayenneLPPMessage(msg, sensorData);

    Serial.printf ("Trying to send: %s\n", printHexBuffer (msg.getBuffer (), msg.getSize ()));

    if (!EnigmaIOTNode.sendData (msg.getBuffer (), msg.getSize ())) {
        Serial.println ("---- Error sending data");
    } else {
        Serial.println ("---- Data sent");
    }
}

void connectEventHandler () {
    Serial.println ("Registered");
}

void disconnectEventHandler (nodeInvalidateReason_t reason) {
    Serial.printf ("Unregistered. Reason: %d\n", reason);
}

void processRxData (const uint8_t* mac, const uint8_t* buffer, uint8_t length, nodeMessageType_t command, nodePayloadEncoding_t payloadEncoding) {
    char macstr[ENIGMAIOT_ADDR_LEN * 3];
    String commandStr;
    uint8_t tempBuffer[MAX_MESSAGE_LENGTH];

    mac2str (mac, macstr);
    Serial.println ();
    Serial.printf ("Data from %s\n", macstr);
    if (command == nodeMessageType_t::DOWNSTREAM_DATA_GET)
        commandStr = "GET";
    else if (command == nodeMessageType_t::DOWNSTREAM_DATA_SET)
        commandStr = "SET";
    else
        return;

    Serial.printf ("Command %s\n", commandStr.c_str ());
    Serial.printf ("Data: %s\n", printHexBuffer (buffer, length));
    Serial.printf ("Encoding: 0x%02X\n", payloadEncoding);

    CayenneLPP lpp (MAX_DATA_PAYLOAD_SIZE);
    DynamicJsonDocument doc (1000);
    JsonArray root;
    memcpy (tempBuffer, buffer, length);

    switch (payloadEncoding) {
    case CAYENNELPP:
        root = doc.createNestedArray ();
        lpp.decode (tempBuffer, length, root);
        serializeJsonPretty (doc, Serial);
        break;
    case MSG_PACK:
        deserializeMsgPack (doc, tempBuffer, length);
        serializeJsonPretty (doc, Serial);
        break;
    default:
        DEBUG_WARN ("Payload encoding %d is not (yet) supported", payloadEncoding);
    }
}

void setup () {
    Serial.begin (115200); Serial.println (); Serial.println ();
    time_t start = millis ();

    // Turn-off the 'brownout detector' to avoid random restarts during wake up
    WRITE_PERI_REG (RTC_CNTL_BROWN_OUT_REG, 0);

    // Configure pins
    pinMode(FLOOD_DETECT_PIN, INPUT_PULLUP);
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);

    // ADC pins: nothing special here; analogRead will be used
    analogRead(VOLTAGE_SENSE_PIN);

    EnigmaIOTNode.setResetPin (RESET_PIN);
    EnigmaIOTNode.onConnected (connectEventHandler);
    EnigmaIOTNode.onDisconnected (disconnectEventHandler);
    EnigmaIOTNode.onDataRx (processRxData);

    EnigmaIOTNode.begin (&Espnow_hal);

    uint8_t macAddress[ENIGMAIOT_ADDR_LEN];
    if ((esp_wifi_get_mac (WIFI_IF_STA, macAddress) == ESP_OK))
    {
        EnigmaIOTNode.setNodeAddress (macAddress);
        DEBUG_DBG ("Node address set to %s", mac2str (macAddress));
    } else {
        DEBUG_WARN ("Node address error");
    }

    // Read and send sensors once, then sleep
    sendSensorData();

    Serial.printf ("Total time: %lu ms\n", millis () - start);

    EnigmaIOTNode.sleep ();
}

void loop () {
    EnigmaIOTNode.handle ();
}
