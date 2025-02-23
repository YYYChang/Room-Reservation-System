The room reservation system allows users to log in as either guests or members. Only members are permitted to make bookings, while both guests and members can check the remaining number of rooms. This system operates using TCP/UDP networking.

1. client.cpp: provide an TCP client to connect with TCP server ServerM
   ServerM.cpp: provide TCP server for connection and UDP server and client function
   ServerS.cpp: provide UDP server and client function
   ServerD.cpp: provide UDP server and client function
   ServerU.cpp: provide UDP server and client function
   file_reader.cpp: let user able to read file with providing path or organize data
   file_reader.h: header file of FileReader.cpp
   udp_connect.cpp: udp client and sever operating method
   udp_connect.h: header file of udpConnect.cpp
2. for user authentication:
   client send string "username\npassword\n"
   serverM reply "S" represent "succeed" when sign in success, reply "failU" represent username not found, reply "failP" represent wrong passowrd

   for room availability/reservation:
   client sent string "roomCode\nA\n" for availability request, "roomCode\nR\n" for reservation request,
   serverM will bridge the same client request to serverS/D/U if necessary
   serverM/S/D/U response follow the following status code:
			   PD: permission denied, due to identity
                           IA: invalid roomCode for avialaility
                           IR: invalid roomCode for reservation
                           E: server side error
                           A: room available
                           N: room not available
                           S: reservation succeed
                           F: reservation failed 