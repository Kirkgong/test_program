#!/bin/bash

TARGET_IP="172.20.0.11"
TARGET_PASSWORD="nex2genEVMaker!#22"
GDB_DIR="/home/kirk/project/skywalker-workspace/starcruiser/starcruiser-qnx/apps/qnx_ap/temp/"
#TARGET_PASSWORD="root"

lib_name=(libOSAbstraction.so \
          libais_client.so \
          libais_log.so \
          libfdt_utils.so.1 \
          libgpio_client.so \
          liblibstd.so \
          libmmap_peer.so \
          libpmem_client.so \
          libpmemext.so \
          libc2d30.so \
          libGSLUser.so \
          libplanedef.so \
          libOSUser.so \
          libOmxCore.so \
          libOmxBase.so \
          libcommonUtils.so \
          libvpp.so \
          libvmm.so \
          libavcodec.so.58 \
          libavdevice.so.58 \
          libavformat.so.58 \
          libavutil.so.56 \
          libswresample.so.3 \
          libswscale.so.5 \
          libPowerClient.so \
          c2d30-bltlib.so \
          libESXEGL_Adreno.so \
          libESXGLESv2_Adreno.so \
          libadreno_utils.so \
          libglnext-llvm.so \
          libmmiVenc.so \
          libioctlClient_shim.so \
          libeglSubDriverQnx.so \
          libaudio_record.so \
          libcsdIpcClient.so \
          libioctlClient.so \
          libdll_utils.so \
          libsmmu_client.so \
          libnpa_client.so \
          libais_shdr.so \
          libOpenCL.so \
          libais.so \
          libais_log.so \
          libdalconfig-sa8155_adp_nio_kylo.so \
          libais_config.so \
          libais_nio_max96712_kylo_svc.so \
          libais_nio_max96712_kylo_interior.so \
	  libais_nio_max9296_kylo_dvr.so)

move_program() {
    /usr/bin/expect <<EOF
	set timeout 3600


	spawn ssh root@$TARGET_IP "mount -uw /;cd /nio/bin/luke-dvr;rm -rf dvr_service;"
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

	spawn ssh root@$TARGET_IP "slay dvr_service"
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

	spawn ssh root@$TARGET_IP "rm -rf /multimedia/video/*"
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

	spawn ssh root@$TARGET_IP "rm -rf /blackbox/cdclogs/qnx/slog/*"
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

	spawn ssh root@$TARGET_IP "rm -rf /blackbox/cdclogs/qnx/coredump/*"
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
	spawn scp -r ./build-qnx/install/bin/luke-dvr/ root@$TARGET_IP:/nio/bin/
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

move_video() {

    rm -rf /home/kirk/Downloads/video/*

    /usr/bin/expect <<EOF
	set timeout 3600

	spawn ssh root@$TARGET_IP "mount -uw /"
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

	spawn scp -r root@$TARGET_IP:/multimedia/video/* /home/kirk/Downloads/video/
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

move_log() {

    /usr/bin/expect <<EOF
	set timeout 3600

	spawn ssh root@$TARGET_IP "mount -uw /"
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




	spawn scp -r root@$TARGET_IP:/blackbox/cdclogs/qnx/slog/* /home/kirk/Downloads/log/
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

move_core() {

    /usr/bin/expect <<EOF
	set timeout 3600
	
	spawn scp -r root@$TARGET_IP:/blackbox/cdclogs/qnx/coredump/dvr_service.core.gz .
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

    gunzip dvr_service.core.gz
    mv dvr_service.core $GDB_DIR
    cp /home/kirk/project/luke-dvr/build-qnx/install/bin/luke-dvr/dvr_service $GDB_DIR
}

ssh() {

    /usr/bin/expect <<EOF
	set timeout 3600

	spawn ssh root@$TARGET_IP
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
	interact
EOF

}

move_image() {

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

	spawn ssh root@$TARGET_IP "screenshot -display=3 -size=1400*1600 -file=a.bmp"
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

	spawn scp -r root@$TARGET_IP:/a.bmp ./
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

	spawn ssh root@$TARGET_IP "rm -rf a.bmp"
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

find_move_file() {
    ret=$(find $1 -name $2) 
    if [ "$ret" == "" ]; then
        echo "can not find $2 in $1"
    else
        cp $ret $GDB_DIR 
        echo "move $2"
    fi
}

move_lib() {
    if [ "$1" == "" ]; then
        echo "parameter error!"
    elif [ "$2" = "" ]; then
        for element in ${lib_name[@]}
            #也可以写成for element in ${array[*]}
            do
            find_move_file $1 $element
            find_move_file $1 $element.sym
            done
    else
        find_move_file $1 $2
        find_move_file $1 $2.sym
    fi
}

if [ "$1" = "program" ]; then
    move_program
elif [ "$1" = "video" ]; then
    move_video
elif [ "$1" = "core" ]; then
    move_core
elif [ "$1" = "image" ]; then
    move_image
elif [ "$1" = "lib" ]; then
    move_lib $2 $3
elif [ "$1" = "log" ]; then
    move_log
elif [ "$1" = "ssh" ]; then
    ssh
else
    echo "parameter error!"
fi
