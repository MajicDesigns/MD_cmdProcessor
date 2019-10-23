
#include <MD_cmdProcessor.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

// handler function prototypes
void handlerHelp(char* param);
void handleA1(char* param);
void handlerZ9(char* param);

const MD_cmdProcessor::cmdItem_t PROGMEM cmdTable[] =
{
  { "?",  handlerHelp, "",    "Help" },
  { "a1", handlerA1,   "123", "Command A1" },
  { "Z9", handlerZ9,   "456", "Command Z9" },
};

MD_cmdProcessor CP(Serial, cmdTable, ARRAY_SIZE(cmdTable));

// handler functions
void handlerHelp(char* param)
{
  Serial.print(F("\nHelp\n===="));
  CP.help();
  Serial.print(F("\n"));
}

void handlerA1(char *param)
{
  Serial.print("\nA1 param:");
  Serial.print(param);
}

void handlerZ9(char *param)
{
  Serial.print("\nZ9 param:");
  Serial.print(param);
}

void setup(void) 
{
  Serial.begin(57600);
  Serial.print(F("\nEnter command. Ensure line ending set to newline."));
  CP.begin();
}

void loop(void) 
{
  CP.run();
}
