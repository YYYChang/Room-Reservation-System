The room reservation system allows users to log in as either guests or members. Only members are permitted to make bookings, while both guests and members can check the remaining number of rooms. This system operates using TCP/UDP networking.

1. Files and Their Functions:

   client.cpp: Provides a TCP client to connect with the TCP server (ServerM).
   ServerM.cpp: Acts as a TCP server for client connections and functions as both a UDP server and client.
   ServerS.cpp: Provides UDP server and client functionality.
   ServerD.cpp: Provides UDP server and client functionality.
   ServerU.cpp: Provides UDP server and client functionality.
   file_reader.cpp: Allows users to read a file by providing a path or to organize data.
   file_reader.h: Header file for file_reader.cpp.
   udp_connect.cpp: Implements UDP client and server operations.
   udp_connect.h: Header file for udp_connect.cpp.

2. User Authentication:

   The client sends a string in the format: "username\npassword\n".
   ServerM responds with:
      "S" (Success) if the login is successful.
      "failU" (Failure - Username) if the username is not found.
      "failP" (Failure - Password) if the password is incorrect.

3. Room Availability and Reservation:

   The client sends:
      "roomCode\nA\n" to request availability.
      "roomCode\nR\n" to request a reservation.
   ServerM may forward the request to ServerS, ServerD, or ServerU if necessary.
   ServerM/S/D/U will respond with the following status codes:
      "PD" (Permission Denied): The user lacks the necessary privileges.
      "IA" (Invalid Availability): The room code is invalid for an availability check.
      "IR" (Invalid Reservation): The room code is invalid for a reservation request.
      "E" (Error): A server-side error occurred.
      "A" (Available): The room is available.
      "N" (Not Available): The room is not available.
      "S" (Success): The reservation was successful.
      "F" (Failure): The reservation failed.