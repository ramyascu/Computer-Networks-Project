README FILE:

Computer Networks COEN-233 Assignment1 
By Ramya Padmanabhan (W1191465)
-------------------

Steps to compile:

Run the script named "compile". If necessary, modify the permissions by using the command line "chmod +x compile". This will create the following binary files in the bin folder

     a) server
     b) bad_server
     c) client
     d) bad_client_case1 to bad_client_case4

---
Steps to run:

To run each scenario, the server must be started first. In a separate window, the client must be started next. The client will exit once it is done. The server must be manually stopped by pressing "ctrl+c" after the scenario finishes.

The following are the scenarios:

1) server + client : This runs the normal scenario where the server and client behave as expected (without any errors)
2) bad_server + client: The server misbehaves by not sending acknowledgement packets correctly all the time. The client resends the packet up to 3 times before it declares the server unresponsive. The timeout for receiving the acknowledgement is 3 seconds. The server sends ACK as follows

		packet 0: ACK is sent immediately
		packet 1: ACK is sent for the first retry
		packet 2: ACK is sent for the second retry
		packet 3: ACK is sent for the third retry
		packet 4: ACK is not sent even after third retry. The client gives up and says server is not responsive.

3) server + bad_client_case1: The client sends packets with segment number out of sequence, namely, 0, 1, 3. The server will send an appropriate rejection packet to the client. The client exits after getting the rejection packet.

4) server + bad_client_case2: The client sends packet 2 with wrong payload length. The server sends a rejection packet. The client quits after getting the rejection packet.

5) server + bad_client_case3: The client sends packets with the end of packet string set to 0xFFF0 instead of 0xFFFF. The server sends the appropriate rejection packet. The client exits after getting the rejection packet.

6) server + bad_client_case4: The client sends a duplicate packet - it sends packet 1 again even after the server acknowledges it the first time. The server sends a duplicate packet rejection code. Once the client gets this rejection packet, it exits.


