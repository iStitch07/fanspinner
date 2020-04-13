#include <iostream>
#include <string>
#include <fstream>
#include <wiringPi.h>
#include <softPwm.h>
#include <unistd.h>

#define PIN        7U

#define RANGE_MAX           100
#define RANGE_MIN            35

#define TEMPERATURE_MAX      55
#define TEMPERATURE_MIN      45

using namespace std;

static int getTemperature() {
  static fstream myfile;
  int temperature = 0;
  myfile.open("/sys/devices/virtual/thermal/thermal_zone0/temp", ios_base::in);
  myfile >> temperature;
  myfile.close();
  return temperature / 1000;
}

static int map(int x, int inMin, int inMax, int outMin, int outMax) {
  if (x < inMin) {
    return outMin;
  } else if (x > inMax) {
    return outMax;
  }
  return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

int main() {
  int temperature;
  int pwmValue;
  bool pwmStopped = true;

  try {
    if (wiringPiSetup() == 0) {
      while (1) {
        temperature = getTemperature();
        pwmValue = map(temperature, TEMPERATURE_MIN, TEMPERATURE_MAX, RANGE_MIN, RANGE_MAX);

        if (temperature >= TEMPERATURE_MIN && pwmStopped) {
          softPwmCreate(PIN, pwmValue, RANGE_MAX);
          pwmStopped = false;
        } else if (temperature >= (TEMPERATURE_MIN - 5) && !pwmStopped) {
          softPwmWrite(PIN, pwmValue);
        } else {
          if (!pwmStopped) {
            softPwmWrite(PIN, 0);
            softPwmStop(PIN);
          }
          pwmStopped = true;
        }
        // cout << "temp: " << temperature << " pwmValue: " << pwmValue << " pwmStoped: " << pwmStopped << endl;
        usleep(1000 * 1000);
      }
    }
  } catch (exception& e) {
    cerr << e.what() << endl;
  }
  return 0;
}
