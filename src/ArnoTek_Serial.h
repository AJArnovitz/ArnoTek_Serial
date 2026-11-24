//*******************************************************************************************
//                                                                                          *
//                               A r n o T e k _ S e r i a l                                *
//                                                                                          *
//                   Copyright Anthony J. Arnovitz 2026    All rights reserved.             *
//                                                                                          *
//------------------------------------------------------------------------------------------*
//                                                                                          *
//     This class defines an interface for easily getting input from the serial bus.        *
//                                                                                          *
//------------------------------------------------------------------------------------------*
//                                                                                          *
// How to use this code:                                                                    *
//     1. Copy this code to your Arduino library                                            *
//     2. Add "#include <ArnoTek_Serial.h>" to your source code                             *
//     3. Define a serial interface object before the setup routine                         *
//     4. Call a Serial.begin statement to set the serial interface's baud rate             *
//     5. Call the "GetMessage" method of the serial interface object obtain incoming text  *
//        The "GetMessage" method returns a true boolen value indicating if a completed     *
//        message is ready for processing.  If no completed message is available, "false"   *
//        is returned.  The incoming message is built up in an internal object buffer       *
//        and is copied to the user's buffer when the message is complete.                  *
//                                                                                          *
// Example:                                                                                 *
//                                                                                          *
//    Define the buffer size for the incoming message. You can define a buffer length       * 
//    between 1 and 64 bytes.  If not defined, the default is 64 bytes.  (64 is the         *
//    maximun Arduino serial buffer size)  If a buffer size is specified, it must be        *
//    defined BEFORE including the ArnoTek_Serial include file.                             *
//                                                                                          *
//    #define ArnoTek_Serial_MaxMsgLen 20                                                   *
//    #include <ArnoTek_Serial.h>                                                           *
//                                                                                          *
//    ArnoTek_Serial AT_Serial;       // Instantiate the ArnoTek serial interface object    *
//                                                                                          *
//    // Define the user's input buffer                                                     *
//    char inMessage[ArnoTek_Serial_MaxMsgLen];                                             *    
//                                                                                          *
//                                                                                          *
//    void setup()                                                                          *
//    {                                                                                     *
//       Serial.begin(115200);        // Initialize serial interface                        *
//                                                                                          *
//       Note that the ArnoTek_Serial object requires nothing in the setup function other   *
//       than Serial.begin needs to be called to initialize the serial interface.           *
//           .                                                                              *
//           .                                                                              *
//           .                                                                              *
//    }                                                                                     *
//                                                                                          *
//                                                                                          *
//    void loop()                                                                           *
//    {                                                                                     *
//       if(AT_Serial.GetMessage(inMessage))  // Check if a completed message was received  *
//       {                                                                                  *
//                                                                                          *
//         // Process completed received message (the completed message is in "inMessage")  *
//                                                                                          *
//         Serial.print("Incoming message: [");                                             *
//         Serial.print(inMessage);                                                         *
//         Serial.println("]");                                                             *
//       }                                                                                  *
//           .                                                                              *
//           .                                                                              *
//           .                                                                              *
//    }                                                                                     *
//                                                                                          *
//*******************************************************************************************


#ifndef ArnoTekSerial_H
#define ArnoTekSerial_H

// Define the max message length if it has not already been defined.  The buffer must 
// include a byte for the numeric 0 string termination charater.
//        NOTE: 64 is the Arduino's max length of an incoming serial message 
#ifndef ArnoTek_Serial_MaxMsgLen
#define ArnoTek_Serial_MaxMsgLen 64
#endif


//*************************************************************************************
//                          A r n o T e k _ S e r i a l
//
//-------------------------------------------------------------------------------------
//
// The ArnoTek_Serial class is used to instantiate an object that processes incoming
// text messages over the serial bus.
//*************************************************************************************

class ArnoTek_Serial
{
  private:
    int                    bufferIndex          = 0;                        // Current index of the next input buffer position
    int                    userBufferLength     = ArnoTek_Serial_MaxMsgLen; // Length of the user's buffer
    int                    waitTimeBetweenReads = 1;                        // Number of milliseconds to wait between successive reads of the serial input;
    bool                   isMessageInProgress  = false;                    // TRUE is a message is in progress coming in
    bool                   isMessageComplete    = false;                    // TRUE if the message is complete and ready to pass tp the user
    bool                   isPriorOverflow      = false;                    // TRUE if an end-of-line character was inserted on the previous message due to a user buffer overflow
    char                   incomingMsg_USB[ArnoTek_Serial_MaxMsgLen];       // Buffer for the incoming message

  public:
    //*************************************************************************************
    //                                 G e t M e s s a g e
    //
    // Gets the completed recieved message (if there is one)
    //
    //-------------------------------------------------------------------------------------
    // Parameters:
    //             ReturnBuffer - Buffer to place the completed message
    //    Returns:
    //             False if no message is available for processing
    //             True if a completed message was received and is ready for processing
    //
    // NOTE: The end of an incomming message is determined when end end-of-line character
    //       is received OR if the max length of the internal receive buffer is reached.
    //       If the message is longer than the specified buffer length but less than the
    //       Arduino maximum message length of 64, multiple messages will be returned
    //       until the entire incoming message has been returned.  Note that if the 
    //       Arduino maximum message length is exceeded, the results are unpredictable.
    //*************************************************************************************

    bool GetMessage(char* ReturnBuffer)
    {
      bool returnValue                      = false;          // Assume nothing is ready until proven otherwise

      //*****************************************************************************************************************************
      //                                   Note about the processing of "isPriorOverflow" boolean                                   *
      //----------------------------------------------------------------------------------------------------------------------------*
      //                                                                                                                            *
      //    The last byte of the message written to the user's buffer must always be a numeric 0 to indicate the end                *
      //    of the message.  This code will automatically change the end-of-line character received from the interface              *
      //    to a numeric 0 to indicate the end of the message.  If the user enters same number or more characters in the interface, *
      //    this code will automatically insert a numeric 0 in the last byte of the buffer to indicate the end of the               *
      //    message.  So if the user defined a 5 byte buffer and entered 6 characters in the interface, a 5 byte messsage           *
      //    will be returned of the first four characaters followed by a numeric 0 in the last byte of the buffer to                *
      //    indicate the end of the first message.  The following call to "GetMessage" will return the remaining entered            *
      //    two characters and the terminating newline character from the interface will be changed to a numeric 0 to               *
      //    indicate the end of the message.  So in this scenario, entering 6 characters when the buffer was defined as             *
      //    being 5 bytes long results in TWO messages being returned even though the user only entered one end-of-line character.  *
      //    Characters 1 through 4 will be returned in the first message and characters 5 through 6 will be returned in a           *
      //    second message.                                                                                                         *
      //                                                                                                                            *
      //    If the user entered 4 bytes (with a 5 byte buffer size being specified as described earlier), a numeric 0 is            *
      //    inserted into the last byte of the user buffer and a message is returned to the user even though the end-of-line        *
      //    character has not yet been received.  This situation is when the "isBufferOverflow" is set to TRUE so if the next       *
      //    character received is an end-of-line character, it is ignored.  So in this scenario, only one message will be returned  *
      //    consisting of characters 1 through 4 followed by a numeric 0 in the last byte of the buffer to indicate the end of      *
      //    the message.                                                                                                            *
      //                                                                                                                            *
      //    Note that if the incoming message is longer than the Arduino internal buffer length of 64 bytes, the results are        *
      //    unpredicable.                                                                                                           *
      //                                                                                                                            *
      //*****************************************************************************************************************************


      // Get the bytes of the incoming message when they are available as long as the message is not complete
      while (Serial.available() && !isMessageComplete)
      {
        isMessageInProgress                 = true;

        // There is data on the serial port - go get it
        incomingMsg_USB[bufferIndex] = Serial.read();

        // Is this the end of the message from the serial port? 
        if (incomingMsg_USB[bufferIndex] == '\n')    
        {
          if(isPriorOverflow)
          {
            isPriorOverflow                 = false;  // If we are in a buffer overflow situation on the next characater, just ignore it
          } 
          else
          {
            // Yes, the entire message has been received and the last message was not a buffer overflow situation
            incomingMsg_USB[bufferIndex]    = '\0';   // Mark the end of the message (Overwrite last used byte in buffer)
            isMessageComplete               = true;   // Mark the message complete and ready to pass to the user
          }
        }
        else 
        {
          // Process the next character that is not and-of-line
          isPriorOverflow                   = false;  // Don't worry about a prior buffer overflow situation since the next byte is not an end-of-line

          // Are we at the end of the user's buffer (leaving room for the string terminator character)
          if (bufferIndex >= (userBufferLength -2) )  // The "2" is because the buffer index is relative 0 plus we need a byte for the string terminator
          {
            isPriorOverflow                 = true;   // The message may be longer than the user's buffer
            incomingMsg_USB[++bufferIndex]  = '\0';   // Mark the end of the message (first byte AFTER the last character received)
            isMessageComplete               = true;   // Mark the message complete and ready to pass to the user
          }        
          else // Not the end of the message - just bump the index to get things ready for the next incoming byte
          {
            bufferIndex++;
          }
        }  
      }

      // Is the received message ready to pass back to the user?
      if(isMessageComplete)
      {          
          // Copy the entire input buffer to the user's buffer and clear the input buffer
          for(int i = 0; i < userBufferLength; i++)
          {
            ReturnBuffer[i]               = incomingMsg_USB[i];
            incomingMsg_USB[i]            = '\0';    // Clear the entire buffer for the next time around in case the next message is shorter that the max buffer length
          }

          // Reset the control variables for the next message and mark that there is a message available
          bufferIndex                     = 0;
          isMessageInProgress             = false;
          isMessageComplete               = false;
          returnValue                     = true;
      }

      return returnValue;
    }
};     // End of ArnoTek_Serial class

#endif