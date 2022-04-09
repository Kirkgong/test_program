#!/bin/bash

TARGET_IP="172.20.0.11"
TARGET_PASSWORD="nex2genEVMaker!#22"
TEST_TIME=1
#TARGET_PASSWORD="root"

auto_test(){
# reset 
echo "send \"\r\n\"
	  send \"sky off\n\"
	  send \"\r\n\"
	  send \"sky off\n\"
	  send \"\r\n\"
	  send \"sky off\n\"
	  sleep 1
	  send \"\r\n\"
      send \"sky on\n\"
	  send \"\r\n\"
      send \"sky on\n\"
	  send \"\r\n\"
      send \"sky on\n\"
    " > run
sudo minicom  -S run  -D /dev/ttyUSB1 &

sleep 40

# get image
move_image
mv test.1.bmp $TEST_TIME.1.bmp
mv test.2.bmp $TEST_TIME.2.bmp
mv test.3.bmp $TEST_TIME.3.bmp
ffmpeg -i $TEST_TIME.1.bmp $TEST_TIME.1.jpg
ffmpeg -i $TEST_TIME.2.bmp $TEST_TIME.2.jpg
ffmpeg -i $TEST_TIME.3.bmp $TEST_TIME.3.jpg
rm -rf *.bmp

# get log
move_log
echo $TEST_TIME.log
mv test.log $TEST_TIME.log
}


move_log(){
	/usr/bin/expect <<EOF
	set timeout 3600

	spawn ssh root@$TARGET_IP "slog2info -t -w -b dvr_service_QC > test.log &"
	sleep 5
	send "$TARGET_PASSWORD\n";

	spawn scp -r root@$TARGET_IP:/test.log ./
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"

	spawn ssh root@$TARGET_IP "rm -rf test.log"
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"

EOF
}

move_image(){
	
	/usr/bin/expect <<EOF
	set timeout 3600

	spawn ssh root@$TARGET_IP "mount -uw /;cd /nio/bin/luke-dvr;./mqpub -n 5;"
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"
	
	sleep 1

	spawn ssh root@$TARGET_IP "screenshot -display=3 -size=1400*1600 -file=test.1.bmp;"
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"

	spawn ssh root@$TARGET_IP "cd /nio/bin/luke-dvr;./mqpub -n 0;"
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"
	
	sleep 1

	spawn ssh root@$TARGET_IP "screenshot -display=3 -size=1400*1600 -file=test.2.bmp;"
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"

	spawn ssh root@$TARGET_IP "cd /nio/bin/luke-dvr;./mqpub -n 1;"
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"
	
	sleep 1

	spawn ssh root@$TARGET_IP "screenshot -display=3 -size=1400*1600 -file=test.3.bmp;"
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"

	spawn scp -r root@$TARGET_IP:/*.bmp ./
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"

	spawn ssh root@$TARGET_IP "rm -rf *.bmp"
	expect {
		"root@$TARGET_IP's password: " 
			{
				send "$TARGET_PASSWORD\n";
			}
		"yes/no" 
			{
				set "yes\n";
				exp_continue;
			}
	}
	expect eof"

EOF
}



test_dir="test_dir"
# creat dir
if [ ! -d "$test_dir" ]; then
        mkdir $test_dir
fi
cd test_dir 
rm -rf *

for i in $(seq 1 5000) 
do  
auto_test
((TEST_TIME++))
echo $TEST_TIME
done


