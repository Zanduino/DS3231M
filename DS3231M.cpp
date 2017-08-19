/*******************************************************************************************************************
** DS3231M class method definitions. See the DS3231M.h header file for program details and version information    **
**                                                                                                                **
** This program is free software: you can redistribute it and/or modify it under the terms of the GNU General     **
** Public License as published by the Free Software Foundation, either version 3 of the License, or (at your      **
** option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY     **
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   **
** GNU General Public License for more details. You should have received a copy of the GNU General Public License **
** along with this program.  If not, see <http://www.gnu.org/licenses/>.                                          **
**                                                                                                                **
*******************************************************************************************************************/
#include "DS3231M.h"                                                          // Include the header definition    //
                                                                              //                                  //
const uint8_t daysInMonth [] PROGMEM={31,28,31,30,31,30,31,31,30,31,30,31};   // Numbers of days in each month    //
/*******************************************************************************************************************
** function date2days returns the number of days from a given Y M D value                                         **
*******************************************************************************************************************/
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {                 //                                  //
  if (y >= 2000) y -= 2000;                                                   // Remove year offset               //
  uint16_t days = d;                                                          // Store numbers of days            //
  for (uint8_t i=1;i<m;++i) days += pgm_read_byte(daysInMonth + i - 1);       // Add number of days for each month//
  if (m > 2 && y % 4 == 0) ++days;                                            // Deal with leap years             //
  return days + 365 * y + (y + 3) / 4 - 1;                                    // Return computed value            //
} // of method date2days                                                      //                                  //
/*******************************************************************************************************************
** function time2long return the number of seconds for a D H M S value                                            **
*******************************************************************************************************************/
static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {       //                                  //
  return ((days * 24L + h) * 60 + m) * 60 + s;                                //                                  //
} // of method time2long                                                      //                                  //
/*******************************************************************************************************************
** Function con2d() converts character string representation to decimal                                           **
*******************************************************************************************************************/
static uint8_t conv2d(const char* p) {                                        //                                  //
  uint8_t v = 0;                                                              //                                  //
  if ('0' <= *p && *p <= '9')                                                 //                                  //
  v = *p - '0';                                                               //                                  //
  return 10 * v + *++p - '0';                                                 //                                  //
} // of method conv2d                                                         //                                  //
/*******************************************************************************************************************
** Class Constructor for DateTime instantiates the class. This is an overloaded class constructor so there are    **
** multiple definitions. This implementation ignores time zones and DST changes. It also ignores leap seconds, see**
** http://en.wikipedia.org/wiki/Leap_second                                                                       **
*******************************************************************************************************************/
DateTime::DateTime (uint32_t t) {                                             //                                  //
  t -= SECONDS_FROM_1970_TO_2000;                                             // bring to 2000 timestamp from 1970//
  ss = t % 60;                                                                //                                  //
  t /= 60;                                                                    //                                  //
  mm = t % 60;                                                                //                                  //
  t /= 60;                                                                    //                                  //
  hh = t % 24;                                                                //                                  //
  uint16_t days = t / 24;                                                     //                                  //
  uint8_t leap;                                                               //                                  //
  for (yOff = 0; ; ++yOff) {                                                  //                                  //
    leap = yOff % 4 == 0;                                                     //                                  //
    if (days < 365 + leap)                                                    //                                  //
    break;                                                                    //                                  //
    days -= 365 + leap;                                                       //                                  //
  } // of for-next each year                                                  //                                  //
  for (m = 1; ; ++m) {                                                        //                                  //
    uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);                //                                  //
    if (leap && m == 2)                                                       //                                  //
    ++daysPerMonth;                                                           //                                  //
    if (days < daysPerMonth)                                                  //                                  //
    break;                                                                    //                                  //
    days -= daysPerMonth;                                                     //                                  //
  } // of for-next each month                                                 //                                  //
  d = days + 1;                                                               //                                  //
} // of method DateTime()                                                     //----------------------------------//
DateTime::DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour,  //                                  //
                    uint8_t min, uint8_t sec) {                               //                                  //
  if (year >= 2000)                                                           //                                  //
  year -= 2000;                                                               //                                  //
  yOff = year;                                                                //                                  //
  m = month;                                                                  //                                  //
  d = day;                                                                    //                                  //
  hh = hour;                                                                  //                                  //
  mm = min;                                                                   //                                  //
  ss = sec;                                                                   //                                  //
} // of method DateTime()                                                     //----------------------------------//
DateTime::DateTime (const DateTime& copy):yOff(copy.yOff),m(copy.m),d(copy.d),//                                  //
                    hh(copy.hh),mm(copy.mm),ss(copy.ss) {}                    // of method DateTime()             //
/*******************************************************************************************************************
** Constructor for using "the compiler's time": DateTime now (__DATE__, __TIME__); NOTE: using F() would          **
** further reduce the RAM footprint, see below. The compiler date and time arrive in string format as follows:    **
** date = "Dec 26 2009", time = "12:34:56"                                                                        **
*******************************************************************************************************************/
DateTime::DateTime (const char* date, const char* time) {                     // User compiler time to see RTC    //
  yOff = conv2d(date + 9);                                                    // Compute the year offset          //
  switch (date[0]) {                                                          // Switch for month detection       //
    case 'J': m = (date[1] == 'a') ? 1 : ((date[2] == 'n') ? 6 : 7); break;   // Jan Feb Mar Apr May Jun Jul Aug  //
    case 'F': m = 2; break;                                                   // Sep Oct Nov Dec                  //
    case 'A': m = date[2] == 'r' ? 4 : 8; break;                              //                                  //
    case 'M': m = date[2] == 'r' ? 3 : 5; break;                              //                                  //
    case 'S': m = 9; break;                                                   //                                  //
    case 'O': m = 10; break;                                                  //                                  //
    case 'N': m = 11; break;                                                  //                                  //
    case 'D': m = 12; break;                                                  //                                  //
  } // of switch for the month                                                //                                  //
  d = conv2d(date + 4);                                                       // Compute the day                  //
  hh = conv2d(time);                                                          //                                  //
  mm = conv2d(time + 3);                                                      //                                  //
  ss = conv2d(time + 6);                                                      //                                  //
} // of method DateTime()                                                     //----------------------------------//
DateTime::DateTime (const __FlashStringHelper* date,                          //                                  //
                    const __FlashStringHelper* time) {                        //                                  //
  char buff[11];                                                              //                                  //
  memcpy_P(buff, date, 11);                                                   //                                  //
  yOff = conv2d(buff + 9);                                                    //                                  //
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec                          //                                  //
  switch (buff[0]) {                                                          //                                  //
    case 'J': m = (buff[1] == 'a') ? 1 : ((buff[2] == 'n') ? 6 : 7); break;   //                                  //
    case 'F': m = 2; break;                                                   //                                  //
    case 'A': m = buff[2] == 'r' ? 4 : 8; break;                              //                                  //
    case 'M': m = buff[2] == 'r' ? 3 : 5; break;                              //                                  //
    case 'S': m = 9; break;                                                   //                                  //
    case 'O': m = 10; break;                                                  //                                  //
    case 'N': m = 11; break;                                                  //                                  //
    case 'D': m = 12; break;                                                  //                                  //
  } // of switch for the month                                                //                                  //
  d = conv2d(buff + 4);                                                       //                                  //
  memcpy_P(buff, time, 8);                                                    //                                  //
  hh = conv2d(buff);                                                          //                                  //
  mm = conv2d(buff + 3);                                                      //                                  //
  ss = conv2d(buff + 6);                                                      //                                  //
} // of method DateTime()                                                     //                                  //
/*******************************************************************************************************************
** Function dayOfTheWeek() to return the day-of-week where Monday is day 1                                        **
*******************************************************************************************************************/
uint8_t DateTime::dayOfTheWeek() const {                                      // Compute the DOW                  //
  uint16_t day = date2days(yOff, m, d);                                       // compute the number of days       //
  return (day + 6) % 7;                                                       // Jan 1, 2000 is a Saturday, i.e. 6//
} // of method dayOfTheWeek()                                                 //                                  //
/*******************************************************************************************************************
** Function unixtime() to return the UNIX time, which is seconds since 1970-01-01 00:00:00                        **
*******************************************************************************************************************/
uint32_t DateTime::unixtime(void) const {                                     //                                  //
  uint32_t t;                                                                 // Declare return variable          //
  uint16_t days = date2days(yOff, m, d);                                      // Compute days                     //
  t = time2long(days, hh, mm, ss);                                            // Compute seconds                  //
  t += SECONDS_FROM_1970_TO_2000;                                             // Add seconds from 1970 to 2000    //
  return t;                                                                   //                                  //
} // of method unixtime()                                                     //                                  //
/*******************************************************************************************************************
** Function secondstime() to return the time, in seconds since 2000                                               **
*******************************************************************************************************************/
long DateTime::secondstime(void) const {                                      //                                  //
  long t;                                                                     //                                  //
  uint16_t days = date2days(yOff, m, d);                                      //                                  //
  t = time2long(days, hh, mm, ss);                                            //                                  //
  return t;                                                                   //                                  //
} // of method secondstime()                                                  //                                  //
/*******************************************************************************************************************
** Overloaded functions to allow math operations on the date/time                                                 **
*******************************************************************************************************************/
DateTime DateTime::operator+(const TimeSpan& span) {                          //                                  //
  return DateTime(unixtime()+span.totalseconds());                            //                                  //
} // of overloaded + function                                                 //                                  //
DateTime DateTime::operator-(const TimeSpan& span) {                          //                                  //
  return DateTime(unixtime()-span.totalseconds());                            //                                  //
} // of overloaded - function                                                 //                                  //
TimeSpan DateTime::operator-(const DateTime& right) {                         //                                  //
  return TimeSpan(unixtime()-right.unixtime());                               //                                  //
} // of overloaded - function                                                 //                                  //
/*******************************************************************************************************************
** Implementation of TimeSpan                                                                                     **
*******************************************************************************************************************/
TimeSpan::TimeSpan (int32_t seconds): _seconds(seconds) {}                    //                                  //
TimeSpan::TimeSpan (int16_t days,int8_t hours,int8_t minutes,int8_t seconds): //                                  //
_seconds((int32_t)days*86400L+(int32_t)hours*3600+(int32_t)minutes*60+seconds){}//                                //
TimeSpan::TimeSpan (const TimeSpan& copy): _seconds(copy._seconds) {}         //                                  //
TimeSpan TimeSpan::operator+(const TimeSpan& right) {                         //                                  //
  return TimeSpan(_seconds+right._seconds);                                   //                                  //
} // of overloaded add                                                        //                                  //
TimeSpan TimeSpan::operator-(const TimeSpan& right) {                         //                                  //
  return TimeSpan(_seconds-right._seconds);                                   //                                  //
} // of overloaded subtract                                                   //                                  //
DS3231M_Class::DS3231M_Class()  {}                                            // Unused class constructor         //
DS3231M_Class::~DS3231M_Class() {}                                            // Unused class destructor          //
/*******************************************************************************************************************
** Method begin starts I2C communications with the device, using a default address if one is not specified and    **
** return true if the device has been detected and false if it was not                                            **
*******************************************************************************************************************/
bool DS3231M_Class::begin() {                                                 // Start I2C communications         //
  Wire.begin();                                                               // Start I2C as master device       //
  Wire.beginTransmission(DS3231M_ADDRESS);                                    // Address the DS3231MM             //
  uint8_t errorCode = Wire.endTransmission();                                 // See if there's a device present  //
  if (errorCode == 0) {                                                       // If we have a DS3231MM            //
    writeByte(DS3231M_RTCHOUR,readByte(DS3231M_RTCHOUR)&B10111111);           // Force use of 24 hour clock       //
  } // of if-then device detected                                             //                                  //
  else return false;                                                          // return error if no device found  //
  return true;                                                                // return success                   //
} // of method begin()                                                        //                                  //
/*******************************************************************************************************************
** Method readByte reads 1 byte from the specified address                                                        **
*******************************************************************************************************************/
uint8_t DS3231M_Class::readByte(const uint8_t addr) {                         //                                  //
  Wire.beginTransmission(DS3231M_ADDRESS);                                    // Address the I2C device           //
  Wire.write(addr);                                                           // Send the register address to read//
  _TransmissionStatus = Wire.endTransmission();                               // Close transmission               //
  Wire.requestFrom(DS3231M_ADDRESS, (uint8_t)1);                              // Request 1 byte of data           //
  return Wire.read();                                                         // read it and return it            //
} // of method readByte()                                                     //                                  //
/*******************************************************************************************************************
** Method writeByte write 1 byte to the specified address                                                         **
*******************************************************************************************************************/
void DS3231M_Class::writeByte(const uint8_t addr, const uint8_t data) {       //                                  //
  Wire.beginTransmission(DS3231M_ADDRESS);                                    // Address the I2C device           //
  Wire.write(addr);                                                           // Send the register address to read//
  Wire.write(data);                                                           // Send the register address to read//
  _TransmissionStatus = Wire.endTransmission();                               // Close transmission               //
} // of method writeByte()                                                    //                                  //
/*******************************************************************************************************************
** Method bcd2dec converts a BCD encoded value into number representation                                         **
*******************************************************************************************************************/
uint8_t DS3231M_Class::bcd2int(const uint8_t bcd){                            // convert BCD digits to integer    //
  return ((bcd/16 * 10) + (bcd % 16));                                        //                                  //
} // of method bcd2int                                                        //                                  //
/*******************************************************************************************************************
** Method dec2bcd converts an integer to BCD encoding                                                             **
*******************************************************************************************************************/
uint8_t DS3231M_Class::int2bcd(const uint8_t dec){                            // convert BCD digits to integer    //
  return ((dec/10 * 16) + (dec % 10));                                        //                                  //
} // of method int2bcd                                                        //                                  //
/*******************************************************************************************************************
** Method adjust set the current date/time. This is an overloaded function, if called with no parameters then the **
** RTC is set to the date/time when the program was compiled and uploaded. Otherwise the values are set, but the  **
** oscillator is stopped during the process and needs to be restarted upon completion.                            **
*******************************************************************************************************************/
void DS3231M_Class::adjust() {                                                // Set the RTC date and Time        //
  adjust(DateTime(F(__DATE__), F(__TIME__)));                                 // Set to compile time              //
} // of method adjust                                                         /-----------------------------------//
void DS3231M_Class::adjust(const DateTime& dt) {                              // Set the RTC date and Time        //
  writeByte(DS3231M_RTCSEC,int2bcd(dt.second()));                             // Write seconds, keep device off   //
  writeByte(DS3231M_RTCMIN,int2bcd(dt.minute()));                             // Write the minutes value          //
  writeByte(DS3231M_RTCHOUR,int2bcd(dt.hour()));                              // Also re-sets the 24Hour clock on //
  writeByte(DS3231M_RTCWKDAY,dt.dayOfTheWeek());                              // Update the weekday               //
  writeByte(DS3231M_RTCDATE,int2bcd(dt.day()));                               // Write the day of month           //
  writeByte(DS3231M_RTCMTH,int2bcd(dt.month()));                              // Month, ignore century bit        //
  writeByte(DS3231M_RTCYEAR,int2bcd(dt.year()-2000));                         // Write the year                   //
  writeByte(DS3231M_STATUS,readByte(DS3231M_STATUS)&B01111111);               // Unset OSC flag bit if set        //
  writeByte(DS3231M_CONTROL,readByte(DS3231M_CONTROL)&B01111111);             // Unset EOSC flag bit if set       //
  _SetUnixTime = now().unixtime();                                            // Store time of last change        //
} // of method adjust                                                         //                                  //
/*******************************************************************************************************************
** Method now() returns the current date/time                                                                     **
** then the power down time is returned, if the timeType = 2 then the power up time is returned. These options are**
** not documented as the official calls are "getPowerOffDate" and "getPowerOnDate"                                **
*******************************************************************************************************************/
DateTime DS3231M_Class::now(){                                                // Return current date/time         //
  Wire.beginTransmission(DS3231M_ADDRESS);                                    // Address the I2C device           //
  Wire.write(DS3231M_RTCSEC);                                                 // Start at specified register      //
  _TransmissionStatus = Wire.endTransmission();                               // Close transmission               //
  Wire.requestFrom(DS3231M_ADDRESS, (uint8_t)7);                              // Request 7 bytes of data          //
  if(Wire.available()==7) {                                                   // Wait until the data is ready     //
    _ss = bcd2int(Wire.read() & 0x7F);                                        // Mask high bit in seconds         //
    _mm = bcd2int(Wire.read() & 0x7F);                                        // Mask high bit in minutes         //
    _hh = bcd2int(Wire.read() & 0x7F);                                        // Mask high bit in hours           //
    Wire.read();                                                              // Ignore Day-Of-Week register      //
    _d = bcd2int(Wire.read()  & 0x3F);                                        // Mask 2 high bits for day of month//
    _m = bcd2int(Wire.read()  & 0x1F);                                        // Mask 3 high bits for Month       //
    _y = bcd2int(Wire.read()) + 2000;                                         // Add 2000 to internal year        //
  } // of if-then there is data to be read                                    //                                  //
  return DateTime (_y, _m, _d, _hh, _mm, _ss);                                // Return class value               //
} // of method now                                                            //                                  //
/*******************************************************************************************************************
** Method temperature() returns the internal temperature in tenths of degrees Celsius (this avoids having to use  **
** floating point                                                                                                 **
*******************************************************************************************************************/
int16_t DS3231M_Class::temperature() {                                        // See if the crystal is running    //
  int32_t temp = readByte(DS3231M_TEMPERATURE)<<8;                            // read MSB                         //
  temp |= readByte(DS3231M_TEMPERATURE+1);                                    // get LSB                          //
  temp = temp>>6;                                                             // Shift over 6 bits                //
  if(temp&0x200) temp|=0xFC00;                                                // keep negative by setting bits    //
  temp = temp*10/4;                                                           // value is in .25C increments      //
  return temp;                                                                // Return computed temperature      //
} // of method temperature()                                                  //                                  //
/*******************************************************************************************************************
** Method setAlarm will set one of the 2 alarms. As the two alarms aren't identical in what will trigger them,    **
** this call chooses which alarm to set depending upon the "alarmType" parameter value.                           **
** will choose the actual alarm to set                                                                            **
*******************************************************************************************************************/
void DS3231M_Class::setAlarm(const uint8_t alarmType,                         // Alarm type enumeration, see above//
                             const DateTime dt,                               // Date/Time to set alarm from      //
                             const bool state ) {                             //                                  //
  if (alarmType>=UnknownAlarm) return;                                        // Don't do anything if out-of-range//
  if (alarmType<everyMinute) {                                                // These types go to alarm 1        //
    writeByte(DS3231M_ALM1SEC,int2bcd(dt.second()));                          // Set seconds value                //
    writeByte(DS3231M_ALM1MIN,int2bcd(dt.minute()));                          // Set minutes value                //
    writeByte(DS3231M_ALM1HOUR,int2bcd(dt.hour()));                           // Set hours value                  //
    if(alarmType==secondsMinutesHoursDateMatch)                               // Set either day of month or day   //
      writeByte(DS3231M_ALM1DATE,int2bcd(dt.day()));                          // of week depending on alarmType   //
    else                                                                      //                                  //
      writeByte(DS3231M_ALM1DATE,int2bcd(dt.dayOfTheWeek()));                 //                                  //
    if(alarmType<secondsMinutesHoursDateMatch)                                // Set the high-bit of ALM1DATE if  //
      writeByte(DS3231M_ALM1DATE,readByte(DS3231M_ALM1DATE)|0x80);            // the alarm bit needs setting      //
    if(alarmType<secondsMinutesHoursMatch)                                    // Set the high-bit of ALM1HOUR if  //
      writeByte(DS3231M_ALM1HOUR,readByte(DS3231M_ALM1HOUR)|0x80);            // the alarm bit needs setting      //
    if(alarmType<secondsMinutesMatch)                                         // Set the high-bit of ALM1MIN if   //
      writeByte(DS3231M_ALM1MIN,readByte(DS3231M_ALM1MIN)|0x80);              // the alarm bit needs setting      //
    if(alarmType==everySecond)                                                // Set the high-bit of ALM1SEC if   //
      writeByte(DS3231M_ALM1SEC,readByte(DS3231M_ALM1SEC)|0x80);              // the alarm bit needs setting      //
    if(alarmType==secondsMinutesHoursDayMatch)                                // Set bit 7 if the alarm bit needs //
      writeByte(DS3231M_ALM1DATE,readByte(DS3231M_ALM1DATE)|0x40);            // setting                          //
    if (state) writeByte(DS3231M_CONTROL,readByte(DS3231M_CONTROL)|1);        // Set A1IE enable to on            //
          else writeByte(DS3231M_CONTROL,readByte(DS3231M_CONTROL)&0xFE);     // Set A1IE enable to off           //
  } else {                                                                    //                                  //
    writeByte(DS3231M_ALM2MIN,int2bcd(dt.minute()));                          // Set minutes value                //
    writeByte(DS3231M_ALM2HOUR,int2bcd(dt.hour()));                           // Set hours value                  //
    if(alarmType==minutesHoursDateMatch)                                      // If we do a min-hh-date match     //
      writeByte(DS3231M_ALM2DATE,int2bcd(dt.day()));                          // Set day of month value           //
    else                                                                      //                                  //
      if (alarmType==minutesHoursDayMatch)                                    // if we have a min-hh-dow match    //
        writeByte(DS3231M_ALM2DATE,int2bcd(dt.dayOfTheWeek()|0x80));          // Set day of week value and switch //
    if(alarmType<minutesHoursDateMatch)                                       //                                  //
      writeByte(DS3231M_ALM2DATE,readByte(DS3231M_ALM2DATE)|0x80);            // the alarm bit needs setting      //
    if(alarmType<minutesHoursMatch)                                           //                                  //
      writeByte(DS3231M_ALM2HOUR,readByte(DS3231M_ALM2HOUR)|0x80);            // the alarm bit needs setting      //
    if(alarmType==everyMinute)                                                //                                  //
      writeByte(DS3231M_ALM2MIN,readByte(DS3231M_ALM2MIN)|0x80);              // the alarm bit needs setting      //
    if (state) writeByte(DS3231M_CONTROL,readByte(DS3231M_CONTROL)|2);        // Set A2IE enable to on            //
      else writeByte(DS3231M_CONTROL,readByte(DS3231M_CONTROL)&0xFD);         // Set A2IE enable to off           //
  } // of if-then-else use alarm 1 or 2                                       //                                  //
  clearAlarm();                                                               // Clear the alarm state            //
  return;                                                                     // return to caller                 //
} // of method setAlarm                                                       //                                  //
/*******************************************************************************************************************
** Method isAlarm will return whether either of the two alarms has been triggered                                 **
*******************************************************************************************************************/
bool DS3231M_Class::isAlarm() {                                               // Return alarm status              //
  return ( readByte(DS3231M_STATUS)&3);                                       // Alarm if either of 2 LSBits set  //
} // of method isAlarm()                                                      //                                  //
/*******************************************************************************************************************
** Method clearAlarm will clear a set alarm by re-writing the same contents back to the register                  **
*******************************************************************************************************************/
void DS3231M_Class::clearAlarm() {                                            // Clear the alarm bits             //
  writeByte(DS3231M_STATUS,readByte(DS3231M_STATUS)&0xFC);                    // Alarm if either of 2 LSBits set  //
} // of method clearAlarm()                                                   //                                  //
/*******************************************************************************************************************
** Method kHz32 will turn the 32.768kHz output on the 32Khz pin on or off                                         **
*******************************************************************************************************************/
void DS3231M_Class::kHz32(const bool state) {                                 // Turn 32kHz output on or off      //
  writeByte(DS3231M_STATUS,(readByte(DS3231M_STATUS)&0xF7)|(state<<3));       // Set bit 3 to state               //
} // of method kHz32()                                                        //                                  //
/*******************************************************************************************************************
** Method getAgingOffset() will return the aging offset value from the DS3231M                                    **
*******************************************************************************************************************/
int8_t DS3231M_Class::getAgingOffset() {                                      //                                  //
  return(readByte(DS3231M_AGING));                                            // Simply return the signed value   //
} // of method getAgingOffset()                                               //                                  //
/*******************************************************************************************************************
** Method setAgingOffset() will set the aging offset as a signed integer. Each value is 0.12ppm and positive      **
** values slows the time base                                                                                     **
*******************************************************************************************************************/
void DS3231M_Class::setAgingOffset(const int8_t agingOffset) {                //                                  //
  writeByte(DS3231M_AGING,agingOffset);                                       //                                  //
  return(readByte(DS3231M_AGING));                                            //                                  //
} // of method setAgingOffset()                                               //                                  //
/*******************************************************************************************************************
** Method weekdayRead will read the weekday number from the RTC. This number is user-settable and is incremented  **
** when the day shifts. It is set as part of the adjust() method where Monday is weekday 1                        **
*******************************************************************************************************************/
uint8_t DS3231M_Class::weekdayRead() {                                        // Read the DOW from the RTC        //
  uint8_t dow = readByte(DS3231M_RTCWKDAY) & B00000111;                       // no need to convert, values 1-7   //
  return dow;                                                                 // return the value                 //
} // of method weekdayRead()                                                  //                                  //
/*******************************************************************************************************************
** Method weekdaywrite will write the weekday (1-7) to the register and return the setting. If the input value is **
** out of range then nothing will be written and a 0 will be returned.                                            **
*******************************************************************************************************************/
uint8_t DS3231M_Class::weekdayWrite(const uint8_t dow) {                      // Write the DOW to the RTC         //
  uint8_t retval = 0;                                                         // assume we have an error          //
  if (dow>0 && dow<8) {                                                       // If parameter is in range, then   //
      writeByte(DS3231M_RTCWKDAY,dow);                                        // set the register                 //
      retval = dow;                                                           // set the return value             //
    } // of if-then we have a good DOW                                        //                                  //
  return dow;                                                                 // return the value                 //
} // of method weekdayWrite()                                                 //                                  //
/*******************************************************************************************************************
** Method pinAlarm() will set the control register flag to make the INT/SQW Pin get pulled up on an alarm         **
*******************************************************************************************************************/
void DS3231M_Class::pinAlarm() {                                              //                                  //
  writeByte(DS3231M_CONTROL,readByte(DS3231M_CONTROL)&~0x4);                  // Set bit 3 to on                  //
} // of method pinAlarm()                                                     //                                  //
/*******************************************************************************************************************
** Method pinSquareWave() will set the control register flag to make the INT/SQW Pin produce a 1Hz signal         **
*******************************************************************************************************************/
void DS3231M_Class::pinSquareWave() {                                         //                                  //
  writeByte(DS3231M_CONTROL,readByte(DS3231M_CONTROL)|0x4);                   // Set bit 3 to off                 //
} // of method pinSquareWave()                                                //                                  //
