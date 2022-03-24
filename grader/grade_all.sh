#!/usr/bin/bash
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar -t startup
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t author
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t ip
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t port
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t _list
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t refresh
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t send
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t broadcast
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t block
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t blocked
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t unblock
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t logout
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t buffer
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t exit
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t statistics
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t exception_login
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t exception_send
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t exception_block
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t exception_unblock
./grader_controller -c grader.cfg -s ../ericbish_pa1.tar  -nu -nb -t exception_blocked
