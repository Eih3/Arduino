// Explanation and more code www.timzaman.com
#include <RTTY.h>

void setup()
{
  RTTY.attach(2,3);
}

void loop()
{
  RTTY.tx("Hello World!");
}