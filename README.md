#CIS427Project2
• Ryne White wryne@umich.edu
• Yuliya Wickens yuliiaw@umich.edu

Created for Windows using WinSock lib.
Used language c++,


DB created with fields.

![image](https://github.com/ryne2298/CIS427Project1/assets/83892913/decbc8da-eb33-4e93-8b11-43c6bf8d554f)


Server.cpp:

![image](https://github.com/ryne2298/CIS427Project1/assets/83892913/bb2816a9-50f3-42fb-ac7d-f36575f1b859)


 
Client.cpp:
![image](https://github.com/ryne2298/CIS427Project1/assets/83892913/e0b39597-9486-4ff0-a024-c3ba73fd0935)

![image](https://github.com/ryne2298/CIS427Project1/assets/83892913/9a1aa1c8-56ac-4503-adb5-e814d94249b0)

Compilation: 
For Server: 
cl /EHsc /std:c++latest /D_CRT_SECURE_NO_WARNINGS Server.cpp /link Ws2_32.lib sqlite3.lib
For Client:
cl /EHsc /std:c++latest /D_CRT_SECURE_NO_WARNINGS Client.cpp /link Ws2_32.lib
in command line. Or add the path before running the code.
Here is githubLink just in case (it called Project1 however its project 2 extended from Project1):
https://github.com/ryne2298/CIS427Project1.git



Compilation: 
For Server: 
cl /EHsc /std:c++latest /D_CRT_SECURE_NO_WARNINGS Server.cpp /link Ws2_32.lib sqlite3.lib
For Client:
cl /EHsc /std:c++latest /D_CRT_SECURE_NO_WARNINGS Client.cpp /link Ws2_32.lib
in command line. Or add the path before running the code.
Here is githubLink just in case (it called Project1 however its project 2 extended from Project1):

https://github.com/ryne2298/CIS427Project1.git

Creators: 
Ryne White: implemented new functions needed for part 2.
Yuliya Wickens: Improved functions from Project1, debugged it, Implemented multithreading (multiple users), Logout, makefile, debugged project 2 ( log is available in github).









# CIS427Project1
• Ryne White wryne@umich.edu
• Yuliya Wickens yuliiaw@umich.edu

• Introduction:
Used language c++


• Running Instructions:
   gcc server.cpp sqlite3.o-o server  (not working)

• Each Student’s role:
o Student 1: Implemented the following features:worked on c++ code, debugging
o Student 2: Implemented the following features: worked on SQL, Database, debugging

• Bugs in the code:
did not compile no matter what we did

 We created code for Linux and tried to start but it told us that it did not support the std::  format,
which means it does not support standard libraries C++ 11.
We added the error log and code for the Linux version with the commands that were used.




