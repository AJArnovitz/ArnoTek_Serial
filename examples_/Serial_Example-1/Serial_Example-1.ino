//******************************************************************
//
//        A r n o T e k   S e r i a l   E x a m p l e
//
//        Serial Bus Input Utility
//
//------------------------------------------------------------------
//
//  Copyright 2026 Anthony J. Arnovitz  All rights reserved.
//
//******************************************************************


//          * * *   I n c l u d e   F i l e   D e f i n i t i o n s   * * *

#include <ArnoTek_HeartBeat.h>


// Define the buffer size for the incoming message. You can define a buffer length between 1 and 64
// bytes.  If not defined, the default is 64 bytes.  ("64" is the maximun Arduino serial buffer size) 

//#define ArnoTek_Serial_MaxMsgLen 10
#include <ArnoTek_Serial.h>



//                  * * *   C o n s t r u c t o r s   * * *

ArnoTek_HeartBeat HeartBeat((long unsigned)250);  // Define heartbeat object (quarter second timer)
ArnoTek_HeartBeat PrintSpacer((long unsigned)10000, 0, &SpacerFunctionCallback);  // Define object to space serial output

ArnoTek_Serial AT_Serial;                         // Define serial interface object                     



//                  * * *   D e f i n e   G l o b a l   V a r i a b l e s   * * *

char inMessage[ArnoTek_Serial_MaxMsgLen];       // Define a buffer for the incoming message.    



//                  * * *   S E T U P   * * *

void setup() 
{
  Serial.begin(115200);

  Serial.println("\n\n*** Starting setup ***\n");



  Serial.println("\t*** Hit enter to continue ***\n");

  do {} while (!AT_Serial.GetMessage(inMessage));  // Wait for user input to continue set up 
  Serial.println("\n*** Setup complete ***\n\n");
}


//                  * * *   L O O P   * * *

void loop() 
{
  HeartBeat.Toggle();
  PrintSpacer.Toggle();


  if(AT_Serial.GetMessage(inMessage))  // Check if a completed message was received  
  {            
    // Process completed received message (the completed message is in "inMessage") 
    int msgLen = 0;
    while (inMessage[msgLen++] != '\0') {}
    Serial.print("Incoming message: (");
    Serial.print(msgLen);
    Serial.print(" bytes) [");
    Serial.print(inMessage);
    Serial.println("]");
  } 

}


//                  * * *   C a l l b a c k   F u n c t i o n s   * * *

  // This function outputs a blank line at a previously specified time interval
  void SpacerFunctionCallback(ArnoTek_HeartBeat *myObject, long unsigned myCurrentTime)
  {   
    Serial.println("");
  }
