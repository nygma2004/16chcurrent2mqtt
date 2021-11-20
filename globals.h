

char msg[50];
String mqttStat = "";
String message = "";
unsigned long lastTick, uptime, seconds, lastWifiCheck, lastRGB;
bool ledoff = false;
char newclientid[80];
char buildversion[12]="v0.9";
int result;

int currentdata[56];
