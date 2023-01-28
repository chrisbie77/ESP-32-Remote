// void checkButtons()
// {
//   /*
//    * Created by ArduinoGetStarted.com
//    *
//    * This example code is in the public domain
//    *
//    * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-button-library
//    *
//    * This example:
//    *   + uses debounce for a button.
//    *   + reads state of a button
//    *   + detects the pressed and released events of a button
//    */
//   button1.loop();
//   button1State = button1.getState();
//   if (button1.isPressed())
//   {
//     // myPublish("b1:pressed");

//     if (sendMqtt == true)
//     {
//       myPublish(motorTopic, "b1:d");
//     }
//   }
//   if (button1.isReleased())
//   {
//     // myPublish("b1:released");
//   }
//   button2.loop();
//   button2State = button2.getState();
//   if (button2.isPressed())
//   {

//     if (sendMqtt == true)
//     {
//       myPublish(motorTopic, "b2:d");
//     }
//   }
//   if (button2.isReleased())
//   {
//     // myPublish(buttonsTopic, "b2:released");
//   }
//   button3.loop();
//   button3State = button3.getState();
//   if (button3.isPressed())
//   {

//     if (sendMqtt == true)
//     {
//       myPublish(motorTopic, "b3:d");
//     }
//   }
//   button4.loop();
//   button4State = button4.getState();
//   if (button4.isPressed())
//   {
//     if (sendMqtt == true)
//     {
//       myPublish(motorTopic, "b4:d");
//     }
//   }
//   button5.loop();
//   button5State = button5.getState();
//   if (button5.isPressed())
//   {
//     if (sendMqtt == true)
//     {
//       myPublish(motorTopic, "b5:d");
//     }
//   }
// }

// //

// void readStick1()
// {
//   // Define the number of samples to keep track of. The higher the number, the
//   // more the readingsX1 will be smoothed, but the slower the output will respond to
//   // the input. Using a constant rather than a normal variable lets us use this
//   // value to determine the size of the readingsX1 array.
//   // subtract the last reading:
//   totalX1 = totalX1 - readingsX1[XreadIndex1];
//   totalY1 = totalY1 - readingsY1[YreadIndex1];
//   // read from the sensor:
//   readingsX1[XreadIndex1] = analogRead(stick1PinX);
//   readingsY1[YreadIndex1] = analogRead(stick1PinY);
//   // add the reading to the total:
//   totalX1 = totalX1 + readingsX1[XreadIndex1];
//   totalY1 = totalY1 + readingsY1[YreadIndex1];
//   // advance to the next position in the array:
//   XreadIndex1 = XreadIndex1 + 1;
//   YreadIndex1 = YreadIndex1 + 1;
//   // if we're at the end of the array...
//   if (XreadIndex1 >= numreadingsX1)
//   {
//     // ...wrap around to the beginning:
//     XreadIndex1 = 0;
//   }
//   if (YreadIndex1 >= numreadingsY1)
//   {
//     YreadIndex1 = 0;
//   }
//   // calculate the average:
//   averageX1 = totalX1 / numreadingsX1;
//   averageY1 = totalY1 / numreadingsY1;
//   // delay(100); // delay in between reads for stability
// }

// void readStick2()
// {
//   // Define the number of samples to keep track of. The higher the number, the
//   // more the readingsX1 will be smoothed, but the slower the output will respond to
//   // the input. Using a constant rather than a normal variable lets us use this
//   // value to determine the size of the readingsX1 array.
//   // subtract the last reading:
//   totalX2 = totalX2 - readingsX2[XreadIndex2];
//   totalY2 = totalY2 - readingsY2[YreadIndex2];
//   // read from the sensor:
//   readingsX2[XreadIndex2] = analogRead(stick2PinX);
//   readingsY2[YreadIndex2] = analogRead(stick2PinY);
//   // add the reading to the total:
//   totalX2 = totalX2 + readingsX2[XreadIndex2];
//   totalY2 = totalY2 + readingsY2[YreadIndex2];
//   // advance to the next position in the array:
//   XreadIndex2 = XreadIndex2 + 1;
//   YreadIndex2 = YreadIndex2 + 1;
//   // if we're at the end of the array...
//   if (XreadIndex2 >= numreadingsX2)
//   {
//     // ...wrap around to the beginning:
//     XreadIndex2 = 0;
//   }
//   if (YreadIndex2 >= numreadingsY2)
//   {
//     YreadIndex2 = 0;
//   }
//   // calculate the average:
//   averageX2 = totalX2 / numreadingsX2;
//   averageY2 = totalY2 / numreadingsY2;
//   // delay(100); // delay in between reads for stability
// }

// void sendSticks()
// {

//   // stickValueX1 = averageX1;
//   // stickValueX2 = averageX2;
//   // stickValueY1 = averageY1;
//   // stickValueY2 = averageY2;
//   if (sendX1 == true)
//   {
//     if ((stickValueX1 - lastStickValueX1) > 3 || (lastStickValueX1 - stickValueX1) > 3)
//     {
//       lastStickValueX1 = stickValueX1;
//       mappedX1 = map(stickValueX1, 0, 512, 0, 100);
//       if ((mappedX1 - lastMappedX1 > 2) || (lastMappedX1 - mappedX1 > 2))
//       {
//         mappedStick = String(mappedX1);
//         if (sendMqtt == true)
//         {
//           mqttMessage = "x1:" + mappedStick;
//           myPublish(motorTopic, mqttMessage);
//         }
//         lastMappedX1 = mappedX1;
//       }
//     }
//   }
//   if (sendX2 == true)
//   {
//     if ((stickValueX2 - lastStickValueX2) > 2 || (lastStickValueX2 - stickValueX2) > 2)
//     {
//       lastStickValueX2 = stickValueX2;
//       mappedX2 = map(stickValueX2, 0, 512, 0, 100);
//       if ((mappedX2 - lastMappedX2 > 2) || (lastMappedX2 - mappedX2 > 2))
//       {
//         mappedStick = String(mappedX2);
//         if (sendMqtt == true)
//         {
//           mqttMessage = "x2:" + mappedStick;
//           myPublish(motorTopic, mqttMessage);
//         }
//         lastMappedX2 = mappedX2;
//       }
//     }
//   }
//   if ((stickValueY1 - lastStickValueY1) > 2 || (lastStickValueY1 - stickValueY1) > 2)
//   {
//     lastStickValueY1 = stickValueY1;
//     mappedY1 = map(stickValueY1, 0, 512, 0, 100);
//     if ((mappedY1 - lastMappedY1 > 2) || (lastMappedY1 - mappedY1 > 2))
//     {
//       mappedStick = String(mappedY1);
//       if (sendMqtt == true)
//       {
//         mqttMessage = "y1:" + mappedStick;
//         myPublish(motorTopic, mqttMessage);
//       }
//       lastMappedY1 = mappedY1;
//     }
//   }
//   if ((stickValueY2 - lastStickValueY2) > 2 || (lastStickValueY2 - stickValueY2) > 2)
//   {
//     lastStickValueY2 = stickValueY2;
//     mappedY2 = map(stickValueY2, 512, 0, 0, 100);
//     if ((mappedY2 - lastMappedY2 > 2) || (lastMappedY2 - mappedY2 > 2))
//     {
//       mappedStick = String(mappedY2);
//       if (sendMqtt == true)
//       {
//         mqttMessage = "y2:" + mappedStick;
//         myPublish(motorTopic, mqttMessage);
//       }
//       lastMappedY2 = mappedY2;
//     }
//   }
// }
// //

// void initSticks()
// {
//   for (int thisReadingX = 0; thisReadingX < numreadingsX1; thisReadingX++)
//   {
//     readingsX1[thisReadingX] = 0;
//   }
//   for (int thisReadingY = 0; thisReadingY < numreadingsY1; thisReadingY++)
//   {
//     readingsY1[thisReadingY] = 0;
//   }
//   for (int thisReadingX = 0; thisReadingX < numreadingsX2; thisReadingX++)
//   {
//     readingsX2[thisReadingX] = 0;
//   }
//   for (int thisReadingY = 0; thisReadingY < numreadingsY2; thisReadingY++)
//   {
//     readingsY2[thisReadingY] = 0;
//   }
// }

// void sendSticksUdp()
//  {
//    // int mappedX1 = 0;
//    // int mappedY1 = 0;
//    // int mappedX2 = 0;
//    // int mappedY2 = 0;
//    // stickValueX1 = averageX1;
//    // stickValueX2 = averageX2;
//    // stickValueY1 = averageY1;
//    // stickValueY2 = averageY2;

//   if ((stickValueX1 - lastStickValueX1) > 3 || (lastStickValueX1 - stickValueX1) > 3)
//   {
//     lastStickValueX1 = stickValueX1;
//     mappedX1 = map(stickValueX1, 0, 512, 0, 100);
//     mappedStick = String(mappedX1);
//     udpOutMessage = "x1:" + mappedStick;
//     udpOutMessage.toCharArray(udpOutMessageChar, 16);
//     //udpSocket.broadcastTo(udpOutMessageChar, 4444);
//     udpSocket.connect(targetIP, targetPort);
//     AsyncUDPMessage udpMsg;
//     udpMsg.println(udpOutMessageChar);
//     udpSocket.send(udpMsg);
//     udpSocket.close();
//   }
//   if ((stickValueY1 - lastStickValueY1) > 2 || (lastStickValueY1 - stickValueY1) > 2)
//   {
//     lastStickValueY1 = stickValueY1;
//     mappedY1 = map(stickValueY1, 0, 500, 0, 100);
//     mappedStick = String(mappedY1);
//     udpOutMessage = "y1:" + mappedStick;
//     udpOutMessage.toCharArray(udpOutMessageChar, 16);
//     //udpSocket.broadcastTo(udpOutMessageChar, 4444);
//     udpSocket.connect(targetIP, targetPort);
//     AsyncUDPMessage udpMsg;
//     udpMsg.println(udpOutMessageChar);
//     udpSocket.send(udpMsg);
//     udpSocket.close();
//   }
//   if ((stickValueX2 - lastStickValueX2) > 2 || (lastStickValueX2 - stickValueX2) > 2)
//   {
//     lastStickValueX2 = stickValueX2;
//     mappedX2 = map(stickValueX2, 0, 512, 0, 100);
//     mappedStick = String(mappedX2);
//     mqttMessage = "x2:" + mappedStick;
//     udpOutMessage.toCharArray(udpOutMessageChar, 16);
//     //udpSocket.broadcastTo(udpOutMessageChar, 4444);
//     udpSocket.connect(targetIP, targetPort);
//     AsyncUDPMessage udpMsg;
//     udpMsg.println(udpOutMessageChar);
//     udpSocket.send(udpMsg);
//     udpSocket.close();
//   }
//   if ((stickValueY2 - lastStickValueY2) > 2 || (lastStickValueY2 - stickValueY2) > 2)
//   {
//     lastStickValueY2 = stickValueY2;
//     mappedY2 = map(stickValueY2, 500, 0, 0, 100);
//     mappedStick = String(mappedY2);
//     udpOutMessage = "y2:" + mappedStick;
//     udpOutMessage.toCharArray(udpOutMessageChar, 16);
//     //udpSocket.broadcastTo(udpOutMessageChar, 4444);
//     udpSocket.connect(targetIP, targetPort);
//     AsyncUDPMessage udpMsg;
//     udpMsg.println(udpOutMessageChar);
//     udpSocket.send(udpMsg);
//     udpSocket.close();
//   }
//   lastPackAcked = false;
// }
// void recvWithEndMarker()
// {
//   static byte ndx = 0;
//   char endMarker = '\n';
//   char rc;
//   while (Serial.available() > 0 && newData == false)
//   {
//     rc = Serial.read();
//     if (rc != endMarker)
//     {
//       receivedChars[ndx] = rc;
//       ndx++;
//       if (ndx >= numChars)
//       {
//         ndx = numChars - 1;
//       }
//     }
//     else
//     {
//       receivedChars[ndx] = '\0'; // terminate the string
//       ndx = 0;
//       newData = true;
//     }
//   }
// }

// void showNewData()
// {
//   if (newData == true)
//   {
//     myPublish("/robots/remote1/status", receivedChars); // publish: arduino -> mqtt bus
//     newData = false;
//   }
// }

// void startudpSocket()
//  {
//    if (udpSocket.listen(udpPort))
//    {
//      Serial.print("UDP Listening on IP: ");
//      Serial.println(WiFi.localIP());
//      udpSocket.onPacket([](AsyncUDPPacket packet)
//                         {
//                           char *udpMsgChar = (char *)malloc(packet.length() + 1);
//                           memcpy(udpMsgChar, packet.data(), packet.length());
//                           udpMsgChar[packet.length()] = '\0';
//                           udpInMessage = String(udpMsgChar);
//                           free(udpMsgChar);
//                           udpInMessage.trim();
//                           if (udpCommandPrefix == "ak")
//                           {
//                             lastPackAcked = true;
//                           }
//                           else if (udpCommandData == "test")
//                           {
//                             Serial.println("Got test");
//                             myPublish(statusTopic, "Got test over UDP");
//                           }
//                           else if (udpCommandData == "beacon")
//                           {
//                             gotRemoteBeacon = true;
//                           }
//                           else
//                           {
//                             packet.println("ak");
//                           } });
//    }
//  }