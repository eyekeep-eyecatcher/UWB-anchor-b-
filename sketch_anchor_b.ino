#include <DW1000Jang.hpp>
#include <DW1000JangCompileOptions.hpp>
#include <DW1000JangConfiguration.hpp>
#include <DW1000JangConstants.hpp>
#include <DW1000JangRTLS.hpp>
#include <DW1000JangRanging.hpp>
#include <DW1000JangRegisters.hpp>
#include <DW1000JangTime.hpp>
#include <DW1000JangUtils.hpp>
#include <SPIporting.hpp>
#include <deprecated.hpp>
#include <require_cpp11.hpp>

// connection pins
#if defined(ESP8266)
const uint8_t PIN_SS = 15;
#else
const uint8_t PIN_RST = 7;
const uint8_t PIN_SS = 10; // spi select pin
#endif

int16_t numReceived = 0; // todo check int type
String message;

const int speakerPin = 8;

device_configuration_t DEFAULT_CONFIG = {
    false,
    true,
    true,
    true,
    false,
    SFDMode::STANDARD_SFD,
    Channel::CHANNEL_5,
    DataRate::RATE_850KBPS,
    PulseFrequency::FREQ_16MHZ,
    PreambleLength::LEN_256,
    PreambleCode::CODE_3
};

frame_filtering_configuration_t ANCHOR_FRAME_FILTER_CONFIG = {
    false,
    false,
    true,
    false,
    false,
    false,
    false,
    false /* This allows blink frames */
};

void setup() {
    // DEBUG monitoring
    Serial.begin(9600);
    pinMode(speakerPin, OUTPUT);
    Serial.println(F("### DW1000Jang-arduino-ranging-anchorA ###"));
    // initialize the driver
    #if defined(ESP8266)
    DW1000Jang::initializeNoInterrupt(PIN_SS);
    #else
    DW1000Jang::initializeNoInterrupt(PIN_SS, PIN_RST);
    #endif
    Serial.println(F("DW1000Jang initialized ..."));
    // general configuration
    DW1000Jang::applyConfiguration(DEFAULT_CONFIG);
    DW1000Jang::enableFrameFiltering(ANCHOR_FRAME_FILTER_CONFIG);

    DW1000Jang::setPreambleDetectionTimeout(64);
    DW1000Jang::setSfdDetectionTimeout(273);
    DW1000Jang::setReceiveFrameWaitTimeoutPeriod(8000);

    DW1000Jang::setNetworkId(100);
    DW1000Jang::setDeviceAddress(2);
   
    DW1000Jang::setAntennaDelay(16436);
    
    Serial.println(F("Committed configuration ..."));
    // DEBUG chip info and registers pretty printed
    char msg[128];
    DW1000Jang::getPrintableDeviceIdentifier(msg);
    Serial.print("Device ID: "); Serial.println(msg);
    DW1000Jang::getPrintableExtendedUniqueIdentifier(msg);
    Serial.print("Unique ID: "); Serial.println(msg);
    DW1000Jang::getPrintableNetworkIdAndShortAddress(msg);
    Serial.print("Network ID & Device Address: "); Serial.println(msg);
    DW1000Jang::getPrintableDeviceMode(msg);
    Serial.print("Device mode: "); Serial.println(msg);    
}

void loop() {
    // Measure the distance and get the result
    RangeAcceptResult result = DW1000JangRTLS::Anchor_Distance_Response();
    // Check if the distance measurement was successful
    DW1000Jang::startReceive();
    DW1000Jang::clearReceiveStatus();
    numReceived++;
    DW1000Jang::getReceivedData(message);
    
    // Force reading data regardless of isReceiveDone()
    if (DW1000Jang::isReceiveDone() || message.length() > 0) {
        #if defined(ESP8266)
        yield();
        #endif
        DW1000Jang::clearReceiveStatus();
        // get data as string
        DW1000Jang::getReceivedData(message);
        if (char(message[0]) == 'H') {
            tone(speakerPin, 1000);
            delay(10); // 0.1초 동안 소리 출력
        } else if (char(message[0]) == 'L' || char(message[0]) == 'S') {
            noTone(speakerPin);
            delay(100); // 0.1초 동안 소리 중지
        }
    }
}
