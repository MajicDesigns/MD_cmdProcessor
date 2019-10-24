#pragma once

class MD_cmdProcessor
{
/*
Manages a command line processor for the application
Reads input from Stream passed as a parameter
Writes to Stream output

A command is defined as 
<command_line> = <cmd>|<cmd><space><parameters>[<separator><command_line>]<eoln>
<cmd> = <string>
<parameters> = <string>
<string> = <character><string>
<space> = ' '
<separator> = ';'
<eoln> = '\n'
<character> = any ASCII except for <space>, <separator> or <newline>

Valid commands are defined in a table the calling application. Commands are
case insensitive (ie, "A" and "a" are the same). The table should be located
in flash memory (PROGMEM).
Handlers (callback functions) are also defined to handle each command.
This class will manage the user input and call the relevant handler with 
the parameters passed for that command.
*/

public:
  /*
  Command Table item
  The command table is declared in the user code and a read-only reference
  is passed to the class for processing.
  */
  static const uint8_t CMD_TXT_SIZE = 2;   // allowed command max text length in characters
  static const uint8_t CMD_PARAM_SIZE = 8; // allowed length in characters for parameter help text
  static const uint8_t CMD_HELP_SIZE = 50; // allowed length in characters for help text
  typedef void (*cmdHandler_t)(char* param);

  struct cmdItem_t
  {
    char cmd[CMD_TXT_SIZE+1];         // the actual command string
    cmdHandler_t f;                     // address of the function to handle this command
    char helpParam[CMD_PARAM_SIZE+1]; // format of the parameters following SPACE for help output
    char helpText[CMD_HELP_SIZE+1];   // text for this command's help
  };

  // Constructor/destructor
  MD_cmdProcessor(Stream& S, const cmdItem_t* cmdTable, uint16_t size) :
    _S(S), _cmdTable(cmdTable), _cmdSize(size)
  {}

  ~MD_cmdProcessor(void) {}

  // Methods
  void begin(void) {}

  void process(char* cmd)
  /*
  Process the command line. 
  This may consist of multiple command separated by the SEPARATOR. A command not found
  causes an error to be printed to the output stream without stopping the rest of the
  command line.
  Normally invoked from get() but can be called from user code with a command string.
  */
  {
    char *psz = cmd;   // working pointer
    char *pcmd;        // current command
    char *pparam;      // current parameters

    if (cmd == nullptr) // do nothing but avoid crashing
      return;

    while (*psz != EOSTR) // process all of what we were given to the end of the string
    {
      uint16_t idx;     // generic indexing variable

      // Tokenize at SPACE and the SEPARATOR to create pcmd 
      // and pparam for this time round the while loop
      pcmd = psz;
      while ((*psz != SPACE) && (*psz != SEPARATOR) && (*psz != EOSTR)) psz++;
      if (*psz == SPACE) *psz++ = EOSTR;
      pparam = psz;
      while ((*psz != SEPARATOR) && (*psz != EOSTR)) psz++;
      if (*psz == SEPARATOR) *psz++ = EOSTR;
      
      // Look for the command in the command table
      for (idx = 0; idx < _cmdSize; idx++)
        if (strcasecmp_P(pcmd, _cmdTable[idx].cmd) == 0)
          break;

      // Process the command if we found it
      if (idx == _cmdSize)
      {
        // Not found :(
        _S.print(F("\nInvalid cmd: "));
        _S.print(pcmd);
      }
      else
      {
        // Found! 
        // Copy the handler address as it does not seem to work from flash.
        cmdHandler_t handler;

        memcpy_P(&handler, &(_cmdTable[idx].f), sizeof(cmdHandler_t));
        if (handler != nullptr)
          handler(pparam);   // call the handler function
      }
    }
  }

  void run(void)
  /*
  Get the command line
  Buffer up all the incoming characters from the input stream until EOLN, 
  at which time it is processed.
  */
  {
    while (_S.available())
    {
      char c = _S.read();

      if (c == EOLN)  // end of line
      {
        process(_cmdInput);
        _cmdIdx = 0; // reset for the next run
      }
      else if (_cmdIdx < CMD_BUF_SIZE - 1)  // don't overflow
      {
        _cmdInput[_cmdIdx++] = c;
        _cmdInput[_cmdIdx] = EOSTR;
      }
    }
  }

  void help(void)
  /*
  Print out the table as formatted help text.
  Useful for user code to maintain consistency but not grouped in any special way.
  */
  {
    for (uint8_t i = 0; i < _cmdSize; i++)
    {
      cmdItem_t temp;

      memcpy_P((void *)&temp, (void *)(_cmdTable + i), sizeof(cmdItem_t));
      _S.print(F("\n"));
      _S.print(temp.cmd);
      _S.write(SPACE);
      _S.print(temp.helpParam);
      _S.print(F("\t"));
      _S.print(temp.helpText);
    }
  }

private:
  // Constants
  static const uint8_t CMD_BUF_SIZE = 40;  // allowed command buffer size in characters

  static const char SPACE = ' ';      // command space character
  static const char SEPARATOR = ';';  // command separator character
  static const char EOLN = '\n';      // command end of line character
  static const char EOSTR = '\0';     // command end of string character

  // Input/Output 
  char _cmdInput[CMD_BUF_SIZE + 1]; // character buffer
  uint16_t _cmdIdx = 0;             // next character store index
  Stream& _S;                       // I/O stream object

  // The command table and size of table (number of items)
  const cmdItem_t *_cmdTable;
  uint16_t _cmdSize;
};
