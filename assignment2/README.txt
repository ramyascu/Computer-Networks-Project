README FILE:

Computer Networks COEN-233 Assignment2 
By Ramya Padmanabhan
-------------------

Steps to compile:

Run the script named "compile". If necessary, modify the permissions by using the command line "chmod +x compile". This will create the following binary files in the bin folder

     a) server
     b) client
     
     

---
Steps to run:

Start the server first by typing "bin/server" and then start the client "bin/client". This will make the client send 4 different requests to the server and the server will send the appropriate response packet:

Case 1: clientDevice is in database and has paid (ACCESS OK)
Case 2: clientDevice is in database and has NOT paid
Case 3: clientDevice is in database, but wrong technology
Case 4: clientDevice is not in the database

After these 4 requests, the client will quit. The server should be stopped manually by pressing "ctrl+c"
