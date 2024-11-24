# EE450 Socket Programming Project - Git450
**Chih-Cheng Hsieh 6024588048**

## Overview
This project implements a simplified version of a version control platform similar to GitHub using C. After authentication, users can perform various operations, including:

* **Lookup:** View documents within a specific user's repository.
* **Push:** Upload documents to the server.
* **Deploy:** Retrieve all documents of the user and deploy them on the server.
* **Remove:** Delete existing documents.
* **Log:** View the user's history operations.

## System Architecture
* **Client:** A user interface for command input, response display, and filtering invalid commands.
* **Main Server (serverM):** A central server that receives commands from the client, dispatches tasks to the appropriate backend servers, identifies guest users, and records each member's command history.
* **Authentication Server (serverA):** A backend server that verifies user membership and handles password encryption.
* **Repository Server (serverR):** A backend server that maintains the document list of members, allowing users to view, upload, and delete documents.
* **Deployment Server (serverD):** A backend server that maintains a list of deployed documents.

## Changes to On-Screen Messages
* **Client**\
`Wrong command`: Member client enters invalid command\
`Invalid input, please try again.`: Enter a response other than Y/N when prompted for overwrite confirmation\
`The remove request failed.`: The confirmation message received when there are no target files to delete in the system
`Get socket name failed`: Debug code for getting socket name\

* **serverM**\
`Failed to allocate memory`: Debug code for memory allocation\
`Failed to reallocate memory`: Debug code for memory reallocation\
`UDP Socket creation failed`: Debug code for UDP Socket creation\
`UDP Bind failed`: Debug code for UDP Bind\
`TCP Socket creation failed`: Debug code for TCP Socket creation\
`TCP Bind failed`: Debug code for TCP Bind\
`Listen failed`: Debug code for TCP Listen\
`TCP Accept failed`: Debug code for TCP Accept

* **serverA**\
`UDP Socket creation failed`: Debug code for UDP Socket creation\
`UDP Bind failed`: Debug code for UDP Bind\
`Can't open members.txt`: Debug code for open members.txt\

* **serverR**\
`UDP Socket creation failed`: Debug code for UDP Socket creation\
`UDP Bind failed`: Debug code for UDP Bind\
`Can't open filenames.txt`: Debug code for opening filenames.txt\
`Can't open temp.txt`: Debug code for open temp.txt\
`Error deleting original file`: Debug code for deleting filenames.txt\
`Error renaming temporary file`: Debug code for renaming temp.txt\
`Can't open members.txt`: Debug code for open members.txt

* **serverD**\
`UDP Socket creation failed`: Debug code for UDP Socket creation\
`UDP Bind failed`: Debug code for UDP Bind\
`Can't open deployed.txt`: Debug code for opening deployed.txt\

## Reused Code
* **Socket Setup:** https://beej-zhtw.netdpi.net/client-server_basic
* **Structure:** https://hackmd.io/@ndhu-programming-2021/ryQV46uRt
* **File operations:** https://www.freecodecamp.org/chinese/news/file-handling-in-c-how-to-open-close-and-write-to-files/